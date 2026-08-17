#!/usr/bin/env python3
import clang.cindex, os, re, sys, json
from collections import defaultdict

SRC = "/home/arthur/Downloads/sqlite-cpp/libraries/libsqlite3-legacy/pkg/sqlite-src-3530400/sqlite3.E.c"
CONTAINER_DIR = "/home/arthur/Downloads/sqlite-cpp/libraries/libsqlite3-legacy-amalgamation/src/sqlite"
UNCAT = "_Uncategorized"

ARGS = ["-std=c23","-D_Float32=float","-D_Float64=double","-D_Float128=__float128",
        "-D_Float32x=float","-D_Float64x=double"]

containers = sorted(f[:-2] for f in os.listdir(CONTAINER_DIR) if f.endswith(".h"))
container_set = set(containers)
containers_by_len = sorted(containers, key=len, reverse=True)

def base_type_name(t):
    while t.kind == clang.cindex.TypeKind.POINTER:
        t = t.get_pointee()
    spelling = t.spelling
    spelling = re.sub(r'\b(const|volatile|struct|union|enum)\b', '', spelling)
    return spelling.strip()

def classify_by_first_param(cursor):
    args = list(cursor.get_arguments())
    if not args:
        return None
    name = base_type_name(args[0].type)
    return name if name in container_set else None

def classify_by_name(fname):
    if fname.startswith("sqlite3"):
        rest = fname[len("sqlite3"):]
        for c in containers_by_len:
            if rest.startswith(c):
                nxt = rest[len(c):len(c)+1]
                if nxt == "" or nxt.isupper() or not nxt.isalnum():
                    return c
    for c in containers_by_len:
        if len(c) < 3:
            continue
        cl = c[0].lower() + c[1:]
        if fname.startswith(cl):
            nxt = fname[len(cl):len(cl)+1]
            if nxt == "" or nxt.isupper():
                return c
    for c in containers_by_len:
        if len(c) < 4:
            continue
        idx = fname.find(c)
        if idx == -1:
            continue
        before_ok = idx == 0 or not fname[idx-1].isalnum()
        after = fname[idx+len(c):idx+len(c)+1]
        after_ok = after == "" or after.isupper() or not after.isalnum()
        if before_ok and after_ok:
            return c
    return None

def extent_text(source, cursor):
    s, e = cursor.extent.start.offset, cursor.extent.end.offset
    return source[s:e]

def main():
    idx = clang.cindex.Index.create()
    tu = idx.parse(SRC, args=ARGS)
    errs = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    if errs:
        print(f"WARNING: {len(errs)} parse errors", file=sys.stderr)

    with open(SRC, "r", encoding="utf-8", errors="surrogateescape") as f:
        source = f.read()

    main_file = os.path.realpath(SRC)

    defs = {}
    decls = defaultdict(list)

    for c in tu.cursor.get_children():
        if c.location.file is None:
            continue
        if os.path.realpath(c.location.file.name) != main_file:
            continue
        if c.kind != clang.cindex.CursorKind.FUNCTION_DECL:
            continue
        if c.is_definition():
            if c.spelling not in defs:
                defs[c.spelling] = c
        else:
            decls[c.spelling].append(c)

    per_container_defs = defaultdict(list)   # container -> [(offset, name, text)]
    per_container_decls = defaultdict(list)  # container -> [(offset, name, text)]

    skipped_libc = []
    for name, cursor in defs.items():
        dtext = extent_text(source, cursor).strip()
        cont = classify_by_first_param(cursor) or classify_by_name(name)
        if cont is None and ("__gnu_inline__" in dtext or re.match(r'^\s*(static|extern)\s+__inline\b', dtext)):
            # glibc header extern-inline leaked into the preprocessed dump; not sqlite's code.
            skipped_libc.append(name)
            continue
        c = cont or UNCAT
        per_container_defs[c].append((cursor.extent.start.offset, name, dtext))

        if name in decls:
            dc = decls[name][0]
            dtext2 = extent_text(source, dc).strip()
            if not dtext2.endswith(";"):
                dtext2 += ";"
            per_container_decls[c].append((dc.extent.start.offset, name, dtext2))

    stats = {}
    for c in sorted(set(list(per_container_defs.keys()) + list(per_container_decls.keys()))):
        defs_list = sorted(per_container_defs[c], key=lambda x: x[0])
        decls_list = sorted(per_container_decls[c], key=lambda x: x[0])

        if c == UNCAT:
            hpath = os.path.join(CONTAINER_DIR, f"{UNCAT}.h")
            cpath = os.path.join(CONTAINER_DIR, f"{UNCAT}.c")
            if not os.path.exists(hpath):
                with open(hpath, "w") as f:
                    f.write("#pragma once\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
                            "/* Functions with no clear owning container (free helpers, glibc-shadowed\n"
                            "   symbols, etc.) collected here for manual triage. */\n\n"
                            "#ifdef __cplusplus\n}\n#endif\n")
            header_content = open(hpath).read()
        else:
            hpath = os.path.join(CONTAINER_DIR, f"{c}.h")
            header_content = open(hpath).read()

        existing_names = set(re.findall(r'\b([A-Za-z_]\w*)\s*\(', header_content))
        new_decls = [t for (_, n, t) in decls_list if n not in existing_names]

        marker = "#ifdef __cplusplus\n}\n#endif"
        idxm = header_content.rfind(marker)
        if new_decls:
            if idxm == -1:
                if not header_content.endswith("\n"):
                    header_content += "\n"
                header_content += "\n" + "\n".join(new_decls) + "\n"
            else:
                block = "\n  " + "\n  ".join(new_decls) + "\n\n"
                header_content = header_content[:idxm] + block + header_content[idxm:]
            with open(hpath, "w") as f:
                f.write(header_content)

        cpath = os.path.join(CONTAINER_DIR, f"{c}.c")
        body = f'#include "sqlite/{c}.h"\n\n' + "\n\n".join(t for (_, n, t) in defs_list) + "\n"
        with open(cpath, "w") as f:
            f.write(body)

        stats[c] = {"defs": len(defs_list), "new_decls_added": len(new_decls), "decls_skipped_dup": len(decls_list)-len(new_decls)}

    with open("/tmp/claude-1000/-home-arthur-Downloads-sqlite-cpp/3661589c-59d0-445d-8a7c-5534d9ab0b57/scratchpad/apply_stats.json", "w") as f:
        json.dump(stats, f, indent=2)

    total_defs = sum(v["defs"] for v in stats.values())
    print(f"containers touched: {len(stats)}")
    print(f"total definitions written: {total_defs}")
    print(f"uncategorized defs: {stats.get(UNCAT,{}).get('defs',0)}")
    print(f"skipped (glibc extern-inline leaks, not written anywhere): {len(skipped_libc)}")
    print(sorted(skipped_libc))

if __name__ == "__main__":
    main()
