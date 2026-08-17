#!/usr/bin/env python3
"""
For a given struct's header/source pair (src/sqlite/{Struct}.h / .c, the
project's 1:1 container convention), forward-declare any member field whose
type is a struct pointer instead of #include-ing that struct's header, and
push the #include (needed for the full definition) down into the .c file
that actually dereferences it.

Integral typedef fields (u8, u16, u32, i64, Pgno, ...) are left untouched --
they can't be forward-declared and don't own a struct definition anyway,
since a struct member is only a forward-declaration candidate when its
*canonical* type is a pointer to a RECORD (struct) type.

A field is only detached from its #include when nothing else in the header
requires the complete type (no by-value field/param/return of that type
anywhere else in the same header) -- otherwise the #include is left in place
and a warning is printed.
"""
import argparse
import os
import re
import subprocess
import sys

import clang.cindex

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
SRC_DIR = os.path.join(REPO_ROOT, "src")
DEFAULT_SQLITE_DIR = os.path.join(SRC_DIR, "sqlite")

TYPE_STRIP_RE = re.compile(r"\b(const|volatile|struct|union|enum)\b")

VALUE_LIKE_KINDS = {
    clang.cindex.CursorKind.FIELD_DECL,
    clang.cindex.CursorKind.PARM_DECL,
    clang.cindex.CursorKind.VAR_DECL,
}


def clang_system_includes():
    out = subprocess.run(
        ["clang", "-E", "-x", "c", "-v", "/dev/null"],
        capture_output=True, text=True,
    )
    lines = out.stderr.splitlines()
    paths = []
    capture = False
    for line in lines:
        if "#include <...> search starts here:" in line:
            capture = True
            continue
        if line.startswith("End of search list"):
            capture = False
            continue
        if capture:
            paths.append(line.strip())
    return paths


def base_type_name(spelling):
    """'const Foo *' / 'Foo *' -> 'Foo'; 'unsigned int' -> None (multi-word)."""
    s = TYPE_STRIP_RE.sub("", spelling)
    s = s.replace("*", " ").strip()
    if not s:
        return None
    parts = s.split()
    return parts[0] if len(parts) == 1 else None


def find_struct_node(node, struct_name):
    if node.kind == clang.cindex.CursorKind.STRUCT_DECL:
        if node.spelling == struct_name and node.is_definition():
            return node
    for child in node.get_children():
        result = find_struct_node(child, struct_name)
        if result:
            return result
    return None


def pointee_is_record(t):
    """True if canonical type t is a pointer (any depth) to a struct/union."""
    canonical = t.get_canonical()
    if canonical.kind != clang.cindex.TypeKind.POINTER:
        return False
    pointee = canonical.get_pointee()
    while pointee.kind == clang.cindex.TypeKind.POINTER:
        pointee = pointee.get_pointee()
    return pointee.kind == clang.cindex.TypeKind.RECORD


def struct_pointer_field_names(struct_node):
    """Names of fields in struct_node that are pointer(s)-to-struct."""
    names = set()
    for field in struct_node.get_children():
        if field.kind != clang.cindex.CursorKind.FIELD_DECL:
            continue
        if not pointee_is_record(field.type):
            continue
        name = base_type_name(field.type.spelling)
        if name:
            names.add(name)
    return names


def type_requires_complete(t, name):
    """True if type t, as spelled, is exactly `name` used by value (not
    through a pointer) -- i.e. needs a full definition, not a forward decl."""
    spelling = t.spelling
    canonical = t.get_canonical()

    if canonical.kind in (
        clang.cindex.TypeKind.CONSTANTARRAY,
        clang.cindex.TypeKind.INCOMPLETEARRAY,
    ):
        elem = canonical.get_array_element_type()
        if "*" in elem.spelling:
            return False
        return (
            elem.get_canonical().kind == clang.cindex.TypeKind.RECORD
            and base_type_name(elem.spelling) == name
        )

    if "*" in spelling:
        return False
    return canonical.kind == clang.cindex.TypeKind.RECORD and base_type_name(spelling) == name


def header_needs_complete_type(tu, header_realpath, name):
    """Scan the whole header (not just the target struct) for a by-value use
    of `name` that would break if only a forward declaration were visible."""
    found = False

    def walk(node):
        nonlocal found
        if found:
            return
        if node.location.file is not None and os.path.realpath(node.location.file.name) == header_realpath:
            if node.kind in VALUE_LIKE_KINDS:
                if type_requires_complete(node.type, name):
                    found = True
                    return
            elif node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
                if type_requires_complete(node.result_type, name):
                    found = True
                    return
        for c in node.get_children():
            walk(c)
            if found:
                return

    walk(tu.cursor)
    return found


def find_include_span(text, name):
    m = re.search(rf'[ \t]*#include\s+"sqlite/{re.escape(name)}\.h"[ \t]*\n', text)
    return m.span() if m else None


def has_typedef(text, name):
    return re.search(rf'typedef\s+struct\s+{re.escape(name)}\s+{re.escape(name)}\s*;', text) is not None


def add_includes_to_source(source_path, names, own_header_name, dry_run=False):
    if not names:
        return
    with open(source_path, "r", encoding="utf-8") as f:
        text = f.read()

    existing = set(re.findall(r'#include\s+"sqlite/([^"]+)\.h"', text))
    to_add = sorted(n for n in names if n not in existing)
    if not to_add:
        return

    all_h_match = re.search(r'#include\s+"sqlite/_All\.h"[ \t]*\n', text)
    if all_h_match:
        insert_at = all_h_match.end()
        block = "".join(f'#include "sqlite/{n}.h"\n' for n in to_add)
        new_text = text[:insert_at] + block + text[insert_at:]
    else:
        own_match = re.search(rf'#include\s+"sqlite/{re.escape(own_header_name)}\.h"[ \t]*\n', text)
        if own_match:
            tail = text[own_match.end():]
            block_match = re.match(r'\n(?:#include\s+"sqlite/[^"]+\.h"\s*\n)+', tail)
            if block_match:
                block_names = re.findall(r'#include\s+"sqlite/([^"]+)\.h"', block_match.group(0))
                merged = sorted(set(block_names) | set(to_add))
                new_block = "\n" + "".join(f'#include "sqlite/{n}.h"\n' for n in merged)
                new_text = text[:own_match.end()] + new_block + tail[block_match.end():]
            else:
                new_block = "\n" + "".join(f'#include "sqlite/{n}.h"\n' for n in to_add)
                new_text = text[:own_match.end()] + new_block + tail
        else:
            block = "".join(f'#include "sqlite/{n}.h"\n' for n in to_add)
            new_text = block + text

    if dry_run:
        print(f"[dry-run] would update '{source_path}': add {', '.join('sqlite/' + n + '.h' for n in to_add)}")
        return

    with open(source_path, "w", encoding="utf-8") as f:
        f.write(new_text)
    print(f"Updated '{source_path}': added {', '.join('sqlite/' + n + '.h' for n in to_add)}")


def process(struct_name, sqlite_dir, clang_args, dry_run=False):
    header_path = os.path.join(sqlite_dir, f"{struct_name}.h")
    source_path = os.path.join(sqlite_dir, f"{struct_name}.c")

    if not os.path.exists(header_path):
        print(f"Error: header '{header_path}' not found.", file=sys.stderr)
        sys.exit(1)

    index = clang.cindex.Index.create()
    tu = index.parse(header_path, args=clang_args)
    errors = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    if errors:
        print(f"Parse errors in '{header_path}':", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)

    struct_node = find_struct_node(tu.cursor, struct_name)
    if not struct_node:
        print(f"Error: 'struct {struct_name}' definition not found in '{header_path}'.", file=sys.stderr)
        sys.exit(1)

    candidates = struct_pointer_field_names(struct_node)
    candidates.discard(struct_name)  # self-referential pointers need nothing

    header_realpath = os.path.realpath(header_path)
    with open(header_path, "r", encoding="utf-8") as f:
        header_text = f.read()

    to_remove_spans = []
    to_forward_declare = set()
    c_includes_needed = set()
    skipped_unsafe = []
    skipped_no_container = []

    for name in sorted(candidates):
        container_header = os.path.join(sqlite_dir, f"{name}.h")
        if not os.path.exists(container_header):
            skipped_no_container.append(name)
            continue

        if not has_typedef(header_text, name):
            to_forward_declare.add(name)

        span = find_include_span(header_text, name)
        if span is not None:
            if header_needs_complete_type(tu, header_realpath, name):
                skipped_unsafe.append(name)
                continue
            to_remove_spans.append(span)

        # Either we just stripped the #include, or the field was already
        # forward-declare-only -- either way the .c file needs the full
        # definition to dereference/access the type.
        c_includes_needed.add(name)

    if not to_remove_spans and not to_forward_declare:
        print(f"'{struct_name}': nothing to change.")
    else:
        edits = [(s, e, "") for (s, e) in to_remove_spans]
        if to_forward_declare:
            line_start = header_text.rfind("\n", 0, struct_node.extent.start.offset) + 1
            indent = header_text[line_start:struct_node.extent.start.offset]
            insertion_text = "".join(
                f"{indent}typedef struct {n} {n};\n" for n in sorted(to_forward_declare)
            )
            edits.append((line_start, line_start, insertion_text))
        edits.sort(key=lambda e: e[0], reverse=True)

        new_text = header_text
        for start, end, repl in edits:
            new_text = new_text[:start] + repl + new_text[end:]

        if dry_run:
            print(f"[dry-run] would update '{header_path}':")
            if to_remove_spans:
                print(f"  remove includes: {', '.join(sorted(c_includes_needed))}")
            if to_forward_declare:
                print(f"  add forward decls: {', '.join(sorted(to_forward_declare))}")
        else:
            with open(header_path, "w", encoding="utf-8") as f:
                f.write(new_text)
            print(f"Updated '{header_path}'.")
            if to_remove_spans:
                print(f"  removed includes: {', '.join(sorted(c_includes_needed))}")
            if to_forward_declare:
                print(f"  added forward decls: {', '.join(sorted(to_forward_declare))}")

    if skipped_unsafe:
        print(
            f"Warning: kept #include for {', '.join(sorted(skipped_unsafe))} in '{header_path}' "
            "-- used by value elsewhere in the header, forward declaration is not sufficient.",
            file=sys.stderr,
        )
    if skipped_no_container:
        print(
            f"Note: no container header for {', '.join(sorted(skipped_no_container))}; left untouched.",
            file=sys.stderr,
        )

    if c_includes_needed:
        if os.path.exists(source_path):
            add_includes_to_source(source_path, c_includes_needed, struct_name, dry_run=dry_run)
        else:
            print(
                f"Warning: no source file '{source_path}' to receive includes for "
                f"{', '.join(sorted(c_includes_needed))}.",
                file=sys.stderr,
            )


def main():
    parser = argparse.ArgumentParser(
        description="Forward-declare struct-pointer members of a container's own struct, "
        "pushing the #include for the full definition into its .c file."
    )
    parser.add_argument("struct_name", nargs="+", help="Name(s) of the struct (container) to process")
    parser.add_argument(
        "--sqlite-dir",
        default=DEFAULT_SQLITE_DIR,
        help=f"Directory holding the {{Struct}}.h/.c pairs (default: {DEFAULT_SQLITE_DIR})",
    )
    parser.add_argument("-I", "--include", action="append", default=[], help="Extra -I directories for Clang")
    parser.add_argument("--dry-run", action="store_true", help="Report what would change without writing files")
    args = parser.parse_args()

    clang_args = [f"-I{SRC_DIR}", "-std=c23"] + [f"-I{inc}" for inc in args.include]
    for p in clang_system_includes():
        clang_args += ["-isystem", p]

    for struct_name in args.struct_name:
        process(struct_name, args.sqlite_dir, clang_args, dry_run=args.dry_run)


if __name__ == "__main__":
    main()
