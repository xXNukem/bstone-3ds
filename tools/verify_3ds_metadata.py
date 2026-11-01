#!/usr/bin/env python3
"""Verify the actual embedded Homebrew metadata, not just its CMake inputs."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('directory', type=Path, help='directory containing bstone.3dsx/.smdh/.elf')
parser.add_argument('--stable', action='store_true', help='also reject the experimental weapon loader')
args = parser.parse_args()
d = args.directory
smdh = (d / 'bstone.smdh').read_bytes()
app = (d / 'bstone.3dsx').read_bytes()
assert len(smdh) == 0x36C0 and smdh[:4] == b'SMDH', 'Invalid SMDH'
expected = ('Blake Stone 3DS',
            'Blake Stone: Aliens of Gold & Planet Strike for New Nintendo 3DS',
            'Carlos de la Barrera Perez')
for language in range(16):
    start = 8 + language * 0x200
    actual = tuple(smdh[start + offset:start + offset + length].decode('utf-16le').split('\0')[0]
                   for offset, length in ((0, 0x80), (0x80, 0x100), (0x180, 0x80)))
    assert actual == expected, (language, actual)
assert app[:4] == b'3DSX' and struct.unpack_from('<H', app, 4)[0] >= 44, 'No extended 3DSX header'
offset, size = struct.unpack_from('<II', app, 32)
assert size == len(smdh) and app[offset:offset + size] == smdh, 'SMDH is not embedded correctly'
for start, pixels in ((0x2040, 24 * 24), (0x24C0, 48 * 48)):
    assert len(set(struct.unpack_from('<' + 'H' * pixels, smdh, start))) > 4, 'Empty icon'
if args.stable:
    nm = '/opt/devkitpro/devkitARM/bin/arm-none-eabi-nm'
    symbols = subprocess.check_output([nm, '-C', str(d / 'bstone.elf')], text=True)
    assert 'n3ds_weapons::get_frame(' not in symbols, 'Experimental loader present'
    assert b'AI ENHANCED WEAPON DESIGNS' not in app, 'Experimental menu present'
report = dict(title=expected[0], description=expected[1], author=expected[2],
              smdh_embedded=True, icon_sizes=[24, 48], stable=args.stable,
              sha256={name: hashlib.sha256((d / name).read_bytes()).hexdigest()
                      for name in ('bstone.3dsx', 'bstone.smdh', 'bstone.elf')})
print(json.dumps(report, indent=2))
