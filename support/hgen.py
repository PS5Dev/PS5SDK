#!/usr/bin/env python3
# encoding: utf-8
# Copyright (C) 2022 John Törnblom

import argparse
import ctypes.util
import logging
import os
import xml.etree.ElementTree as ET

import clang.cindex

from pathlib import Path
from elftools.elf.elffile import ELFFile


logger = logging.getLogger('hgen')
NID_DB = (os.path.dirname(__file__) or '.') + '/nid_db.xml'


# load clang library
for libname in ['clang', 'clang-15']:
    filename = ctypes.util.find_library(libname)
    if filename:
        clang.cindex.Config.set_library_file(filename)
        break
else:
    raise Exception('clang python bindings not found')


# read NIDs from nid_db.xml
nid_map = {entry.get('obf'): entry.get('sym')
           for entry in ET.parse(NID_DB).getroot()}


class CursorWalker:
    '''
    Walk the clang AST and yield function prototypes.
    '''
    
    def accept(self, node, **kwargs):
        if node is None:
            return

        name = 'accept_' + node.__class__.__name__
        fn = getattr(self, name, self.default_accept)
        yield from fn(node, **kwargs)
    
    def default_accept(self, node):
        for child in node.get_children():
            yield from self.accept(child)

    def accept_Cursor(self, node):
        name = 'accept_' + node.kind.name
        fn = getattr(self, name, self.default_accept)
        yield from fn(node)

    def accept_FUNCTION_DECL(self, node):
        ret = node.type.get_result().spelling
        name = node.spelling

        if node.type.kind.name != 'FUNCTIONNOPROTO':
            args = [arg.spelling
                    for arg in node.type.argument_types()]
        else:
            args = []
            
        args = ', '.join(args) if args else 'void'
        
        yield ret, name, args


def prototypes(*inc_dirs, args=None):
    '''
    yield function prototypes from files found in a given include dir.
    '''
    args = args or list()
    args += [f'-nostdinc', '-fno-builtin', '-nostdlib']
    args += [f'-I{inc_dir}' for inc_dir in inc_dirs]
    
    for inc_dir in inc_dirs:
        for path in Path(inc_dir).rglob('*.h'):
            try:
                index = clang.cindex.Index.create()
                tu = index.parse(str(path), args=args)
                w = CursorWalker()
                yield from w.accept(tu.cursor)
            except:
                pass


def symbols(*filenames):
    '''
    yield pairs of (sym_name, sym_type) in PT_DYNAMIC segments using the NID lookup table
    "nid_db.xml".
    '''
    for filename in filenames:
        with open(filename, 'rb') as f:
            elf = ELFFile(f)

            for segment in elf.iter_segments():
                if segment.header.p_type != 'PT_DYNAMIC':
                    continue

                for sym in segment.iter_symbols():
                    sym_nid = sym.name[:11]
                    if not sym_nid in nid_map:
                        logger.warning(f'skipping unknown NID {sym.name}')
                        continue
                
                    sym_type = sym.entry['st_info']['type']
                    sym_name = nid_map[sym_nid]

                    yield sym_name, sym_type


if __name__ == '__main__':
    logging.basicConfig()
    
    parser = argparse.ArgumentParser()
    parser.add_argument('--prx', action='append', required=True)
    parser.add_argument('--inc-dir', action='append', required=True)
    parser.add_argument('--skip-untyped', action='store_true')
    
    cli_args = parser.parse_args()

    protos = {name: (ret, args)
              for ret, name, args in prototypes(*cli_args.inc_dir)}
    for sym_name, sym_type in sorted(set(symbols(*cli_args.prx))):
        if sym_name in protos:
            ret, args = protos[sym_name]
            print(f'_Fn_({ret}, {sym_name}, {args});')
            
        elif sym_type == 'STT_FUNC' and not cli_args.skip_untyped:
            print(f'_Fn_(long, {sym_name});')
        
#        elif sym_type == 'STT_OBJECT':
#            print(f'_Data_(long, {sym_name});')
