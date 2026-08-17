#!/usr/bin/env python3

import argparse
import os
import sys
from dataclasses import dataclass

import clang.cindex


@dataclass
class Block:
    start: int
    end: int
    text: str


def real_path(path):
    """Return a normalized absolute path."""
    return os.path.realpath(os.path.abspath(path))


def cursor_is_in_main_file(cursor, main_file):
    """Return True if the cursor belongs to the source file being reordered."""
    if cursor.location.file is None:
        return False

    return real_path(cursor.location.file.name) == main_file


def find_line_end(source_code, offset):
    """Return the offset immediately after the current line."""
    newline = source_code.find("\n", offset)

    if newline == -1:
        return len(source_code)

    return newline


def extract_preprocessor_directives(source_code):
    """
    Extract #include and #define directives directly from the source.

    This is deliberately done from the source text rather than relying solely
    on Clang's preprocessing tokens, because tokenization of preprocessing
    directives can vary depending on libclang/version/options.
    """
    includes = []
    defines = []
    other_directives = []

    lines = source_code.splitlines(keepends=True)

    offset = 0
    in_directive = False
    directive_start = None
    directive_text = ""

    for line in lines:
        stripped = line.lstrip()

        # Handle a continued preprocessor directive.
        if in_directive:
            directive_text += line

            if not line.rstrip().endswith("\\"):
                text = directive_text.strip()
                classify_directive(text)

                in_directive = False
                directive_start = None
                directive_text = ""

        elif stripped.startswith("#"):
            directive_start = offset
            directive_text = line

            if line.rstrip().endswith("\\"):
                in_directive = True
            else:
                text = directive_text.strip()
                classify_directive(text)

        offset += len(line)

    return includes, defines, other_directives


def classify_directive(text):
    """
    Helper used by extract_preprocessor_directives().

    This function is replaced by the closure in the implementation below.
    """
    # Kept only to make the structure obvious.
    return text


def extract_directives(source_code):
    """
    Extract preprocessor directives while preserving their exact text.

    Returns:
        (includes, defines, others)
    """
    includes = []
    defines = []
    others = []

    lines = source_code.splitlines(keepends=True)

    current = ""
    start = None
    offset = 0

    def finish_directive(text):
        stripped = text.lstrip()

        if not stripped.startswith("#"):
            return

        body = stripped[1:].lstrip()

        keyword = body.split(None, 1)[0] if body else ""

        if keyword == "include":
            includes.append(text.strip())
        elif keyword == "define":
            defines.append(text.strip())
        else:
            others.append(text.strip())

    for line in lines:
        stripped = line.lstrip()

        if current:
            current += line

            if not line.rstrip().endswith("\\"):
                finish_directive(current)
                current = ""
                start = None

        elif stripped.startswith("#"):
            current = line
            start = offset

            if not line.rstrip().endswith("\\"):
                finish_directive(current)
                current = ""
                start = None

        offset += len(line)

    if current:
        finish_directive(current)

    return includes, defines, others


def get_cursor_block(cursor, source_code):
    """
    Extract the exact source range represented by a cursor.

    Clang's extent normally covers the complete declaration/definition.
    """
    start = cursor.extent.start.offset
    end = cursor.extent.end.offset

    if start < 0 or end < 0:
        return None

    if start >= len(source_code):
        return None

    end = min(end, len(source_code))

    # Include a trailing semicolon when Clang's extent doesn't include it.
    if cursor.kind in (
        clang.cindex.CursorKind.VAR_DECL,
        clang.cindex.CursorKind.TYPEDEF_DECL,
        clang.cindex.CursorKind.STRUCT_DECL,
        clang.cindex.CursorKind.UNION_DECL,
        clang.cindex.CursorKind.ENUM_DECL,
    ):
        while end < len(source_code) and source_code[end] in " \t\r":
            end += 1

        if end < len(source_code) and source_code[end] == ";":
            end += 1

    return start, end


def classify_cursor(cursor):
    """
    Classify a top-level cursor into one of the output sections.

    Returns:
        "types", "declarations", "definitions", or None
    """
    kind = cursor.kind

    type_kinds = {
        clang.cindex.CursorKind.STRUCT_DECL,
        clang.cindex.CursorKind.UNION_DECL,
        clang.cindex.CursorKind.ENUM_DECL,
        clang.cindex.CursorKind.TYPEDEF_DECL,
    }

    if kind in type_kinds:
        return "types"

    if kind == clang.cindex.CursorKind.FUNCTION_DECL:
        return "definitions" if cursor.is_definition() else "declarations"

    if kind == clang.cindex.CursorKind.VAR_DECL:
        return "definitions" if cursor.is_definition() else "declarations"

    return None


def categorize_ast(source_code, tu, filename):
    """
    Categorize top-level declarations and definitions.

    Returns:
        includes, defines, other_directives, types,
        declarations, definitions
    """
    main_file = real_path(filename)

    includes, defines, other_directives = extract_directives(source_code)

    sections = {
        "types": [],
        "declarations": [],
        "definitions": [],
    }

    seen_spans = set()

    # Only inspect top-level declarations.
    for cursor in tu.cursor.get_children():
        if not cursor_is_in_main_file(cursor, main_file):
            continue

        classification = classify_cursor(cursor)

        if classification is None:
            continue

        block = get_cursor_block(cursor, source_code)

        if block is None:
            continue

        start, end = block

        span = (start, end)

        if span in seen_spans:
            continue

        seen_spans.add(span)

        text = source_code[start:end].strip()

        if not text:
            continue

        sections[classification].append(
            Block(start=start, end=end, text=text)
        )

    # Sort by original source location before constructing the sections.
    for blocks in sections.values():
        blocks.sort(key=lambda block: block.start)

    return (
        includes,
        defines,
        other_directives,
        sections["types"],
        sections["declarations"],
        sections["definitions"],
    )


def build_section(title, items):
    """Build a formatted output section."""
    if not items:
        return None

    return f"/* --- {title} --- */\n" + "\n\n".join(items)


def reorder_c_file(filename, clang_args=None, output=None):
    """
    Parse and reorder a C source file.

    The original file is only overwritten after successful parsing.
    """
    if clang_args is None:
        clang_args = [
            "-std=c11",
            "-Wno-error=parentheses-equality",
            "-Wno-error=sign-compare",
        ]

    filename = real_path(filename)

    if not os.path.isfile(filename):
        raise FileNotFoundError(f"File '{filename}' not found.")

    with open(filename, "r", encoding="utf-8") as f:
        source_code = f.read()

    index = clang.cindex.Index.create()

    try:
        tu = index.parse(
            filename,
            args=clang_args,
            options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )
    except clang.cindex.TranslationUnitLoadError as exc:
        raise RuntimeError(f"Unable to parse '{filename}': {exc}") from exc

    # Display Clang diagnostics.
    errors = []
    for diagnostic in tu.diagnostics:
        print(diagnostic, file=sys.stderr)

        if diagnostic.severity >= clang.cindex.Diagnostic.Error:
            errors.append(diagnostic)

    if errors:
        raise RuntimeError(
            f"Clang reported {len(errors)} error(s); "
            "the source file was not modified."
        )

    (
        includes,
        defines,
        other_directives,
        types,
        declarations,
        definitions,
    ) = categorize_ast(source_code, tu, filename)

    sections = []

    section = build_section("INCLUDES", includes)
    if section:
        sections.append(section)

    section = build_section("DEFINES", defines)
    if section:
        sections.append(section)

    # Keep unknown preprocessor directives rather than silently deleting them.
    section = build_section("OTHER PREPROCESSOR DIRECTIVES", other_directives)
    if section:
        sections.append(section)

    section = build_section(
        "TYPES",
        [block.text for block in types],
    )
    if section:
        sections.append(section)

    section = build_section(
        "DECLARATIONS",
        [block.text for block in declarations],
    )
    if section:
        sections.append(section)

    section = build_section(
        "DEFINITIONS",
        [block.text for block in definitions],
    )
    if section:
        sections.append(section)

    if not sections:
        raise RuntimeError(
            "No reorderable C constructs were found; "
            "the source file was not modified."
        )

    reordered_code = "\n\n".join(sections) + "\n"

    # Write to a temporary file first, then replace the original.
    destination = real_path(output) if output else filename
    temporary = destination + ".tmp"

    try:
        with open(temporary, "w", encoding="utf-8", newline="") as f:
            f.write(reordered_code)

        os.replace(temporary, destination)
    except Exception:
        try:
            if os.path.exists(temporary):
                os.remove(temporary)
        except OSError:
            pass
        raise

    print(f"Successfully reordered '{destination}'.")


def main():
    parser = argparse.ArgumentParser(
        description="Reorder a C source file using libclang."
    )

    parser.add_argument(
        "source_file",
        help="C source file to reorder",
    )

    parser.add_argument(
        "-o",
        "--output",
        help="Write the reordered source to this file instead of modifying the input",
    )

    parser.add_argument(
        "--clang-arg",
        action="append",
        default=[],
        help="Argument passed to Clang. Can be specified multiple times.",
    )

    args = parser.parse_args()

    try:
        reorder_c_file(
            args.source_file,
            clang_args=args.clang_arg,
            output=args.output,
        )
    except (OSError, RuntimeError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())