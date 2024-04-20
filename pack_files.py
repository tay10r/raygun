#!/bin/python3

import sys
from pathlib import Path

class CodeGenerator:
    def __init__(self):
        self.source = ''

    def add_char_array(self, name, data):
        self.source += f'static const char {name}[] = "'
        for c in data:
            c_str = chr(c)
            # Note: 34 is " (double quote)
            if c_str.isprintable() and c_str != '"':
                pass
            elif c_str == '\n':
                c_str = '\\n'
            else:
                c_str = hex(c).replace('0x', '\\x')
            self.source += c_str
        self.source += '";\n'

    def get_source(self):
        return self.source

def main(output_path: str, prefix: str, args: list):
    gen = CodeGenerator()
    for path in args:
        with open(path, 'rb') as f:
            name = str(Path(path).name).replace('.', '_')
            name = f'{prefix}{name}'
            data = f.read()
            gen.add_char_array(name, data)

    with open(output_path, 'wb') as output_file:
        output_file.write(gen.get_source().encode('utf-8'))

main(sys.argv[1], sys.argv[2], sys.argv[3:])
