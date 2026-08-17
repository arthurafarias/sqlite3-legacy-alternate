#!/usr/bin/env python3
import argparse
import sys
import os
import clang.cindex

def find_struct_node(node, struct_name):
    """Recursively search the AST for a struct declaration with struct_name."""
    if node.kind == clang.cindex.CursorKind.STRUCT_DECL:
        if node.spelling == struct_name and node.is_definition():
            return node
    
    for child in node.get_children():
        result = find_struct_node(child, struct_name)
        if result:
            return result
    return None

def extract_struct(source_file, struct_name, output_file, compiler_args=None):
    if compiler_args is None:
        compiler_args = []

    if not os.path.exists(source_file):
        print(f"Error: Source file '{source_file}' not found.", file=sys.stderr)
        sys.exit(1)

    # 1. Parse the source file into an AST
    index = clang.cindex.Index.create()
    tu = index.parse(source_file, args=compiler_args)

    # Check for severe diagnostics/errors
    errors = [diag for diag in tu.diagnostics if diag.severity >= clang.cindex.Diagnostic.Error]
    if errors:
        print("Warnings/Errors encountered during parsing:", file=sys.stderr)
        for diag in errors:
            print(f"  {diag}", file=sys.stderr)

    # 2. Find the requested struct declaration
    struct_node = find_struct_node(tu.cursor, struct_name)
    if not struct_node:
        print(f"Error: Struct '{struct_name}' definition not found in '{source_file}'.", file=sys.stderr)
        sys.exit(1)

    # 3. Read the original file content
    with open(source_file, 'r', encoding='utf-8') as f:
        source_code = f.read()

    # 4. Extract byte/char offsets
    extent = struct_node.extent
    start_offset = extent.start.offset
    end_offset = extent.end.offset

    # Include trailing semicolon if present
    while end_offset < len(source_code) and source_code[end_offset] in (' ', '\t', '\n', ';'):
        if source_code[end_offset] == ';':
            end_offset += 1
            break
        end_offset += 1

    extracted_code = source_code[start_offset:end_offset].strip()

    # 5. Write extracted struct to output header/file
    header_guard = f"_{struct_name.upper()}_H_"
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(f"#pragma once\n\n")
        f.write(f"#ifdef __cplusplus\n\n")
        f.write(f"extern \"C\" {{\n")
        f.write(f"#endif\n\n")
        f.write(f"typedef struct {struct_name} {struct_name};\n\n")
        f.write(f"{extracted_code}\n\n")
        f.write(f"#ifdef __cplusplus\n\n")
        f.write(f"}}\n")
        f.write(f"#endif\n\n")

    print(f"Successfully extracted 'struct {struct_name}' -> '{output_file}'")

    # 6. Update the original C file (Replace struct with #include)
    include_line = f'#include "sqlite/{os.path.basename(output_file)}"'
    new_source_code = source_code[:start_offset] + include_line + source_code[end_offset:]

    with open(source_file, 'w', encoding='utf-8') as f:
        f.write(new_source_code)

    print(f"Updated '{source_file}' to include '{output_file}'.")

def main():
    parser = argparse.ArgumentParser(description="Extract C struct definition using LibClang AST.")
    parser.add_argument("source", help="Source C file")
    parser.add_argument("struct_name", help="Name of the struct to extract")
    parser.add_argument("-o", "--output", required=True, help="Output header file (.h)")
    parser.add_argument("-I", "--include", action="append", help="Include directories for Clang parser", default=[])

    args = parser.parse_args()

    # Format compiler flags (e.g. -I /path/to/includes)
    clang_args = [f"-I{inc}" for inc in args.include]

    extract_struct(args.source, args.struct_name, args.output, clang_args)

if __name__ == "__main__":
    main()