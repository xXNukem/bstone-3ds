#!/usr/bin/env python3
"""Prepare a LOCAL optional artwork pack; never embed/distribute artwork in .3dsx.
Python 3 + Pillow, offline. Reads only final paths from the approved gallery.
SPDX-License-Identifier: MIT
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import zlib
from PIL import Image

KEYS = ('autocharge', 'protector', 'pulse', 'ion', 'grenade')


def prepare(lab, output, layout=None):
    lab = lab.resolve()
    # Read the actual gallery's embedded manifest, not discarded concepts.
    html = (lab / 'index.html').read_text()
    match = re.search(r'<script type="application/json" id="manifest">(.*?)</script>', html, re.S)
    if not match:
        raise ValueError('Gallery manifest missing')
    manifest = json.loads(match.group(1))
    adjustments = json.loads(layout.read_text()) if layout else {}
    if manifest['gameKey'] != 'aog-sw-3.0':
        raise ValueError('This converter maps the approved AOG gallery only; no PS substitution')
    palette_path = Path(__file__).resolve().parents[1] / 'src/bstone/src/vgapal.cpp'
    values = [int(x, 16) for x in re.findall(r'0x([0-9a-fA-F]{2})', palette_path.read_text())]
    if len(values) != 768:
        raise ValueError('Engine palette not recognized')
    colors = {}
    for i in range(256):
        colors.setdefault(tuple(255*c//63 for c in values[i*3:i*3+3]), i)
    prepared = []
    for slot, key in enumerate(KEYS):
        weapon = next(w for w in manifest['weapons'] if w['key'] == key)
        frames = weapon['frames']
        if len(frames) != 5 or [f['frame'] for f in frames] != list(range(5)):
            raise ValueError(key + ': incomplete or reordered animation')
        body = bytearray()
        sources = []
        bounds = []
        for frame in frames:
            source = (lab / frame['enhanced']).resolve()
            if lab not in source.parents or 'animations' not in source.relative_to(lab).parts:
                raise ValueError('Only final gallery animation frames may be integrated')
            with Image.open(source) as png:
                if png.mode != 'RGBA' or png.size != (128, 128):
                    raise ValueError(str(source) + ': expected 128x128 RGBA')
                im = png.copy()
            if not set(im.getchannel('A').getdata()) <= {0, 255}:
                raise ValueError(str(source) + ': nonbinary transparency')
            box = im.getbbox()
            if not box or min(box[:2]) < 4 or max(box[2:]) > 124:
                raise ValueError(str(source) + ': empty or clipped silhouette')
            bounds.append(box)
            left, top, right, bottom = box
            body += struct.pack('<4H', left, top, right-left, bottom-top)
            for x in range(left, right):
                for y in range(top, bottom):
                    r, g, b, a = im.getpixel((x, y))
                    if a and (r, g, b) not in colors:
                        raise ValueError(str(source) + ': color outside engine palette')
                    body += struct.pack('<H', colors[(r, g, b)] if a else 65535)
            sources.append({'path': str(source.relative_to(lab)),
                            'sha256': hashlib.sha256(source.read_bytes()).hexdigest()})
        # Calibrate by original visible IDLE area and actual original AOG scaler
        # (128 logical pixels for its 64-pixel canvas), not gallery mockup pixels.
        with Image.open(lab / frames[0]['original']) as png:
            ob = png.getbbox()
        nb = bounds[0]
        old_area = (ob[2]-ob[0]) * (ob[3]-ob[1])
        new_area = (nb[2]-nb[0]) * (nb[3]-nb[1])
        ref_height = round(128 * 2 * 1.4 * math.sqrt(old_area / new_area))
        tune = adjustments.get(key, {})
        ref_height = int(tune.get('refHeight', ref_height))
        anchor_x, anchor_y = tune.get('anchor', [64, 110])
        offset_x, offset_y = tune.get('offset', [0, 0])
        if not 32 <= ref_height <= 180:
            raise ValueError(key + ': unsafe calibrated scale')
        if not (16 <= anchor_x <= 112 and 64 <= anchor_y <= 127 and
                abs(offset_x) <= 32 and abs(offset_y) <= 32):
            raise ValueError(key + ': unsafe anchor/offset')
        # Hide the final 3 native rows below the viewport. Same anchor and scale
        # for all five frames; keep existing down-only bob and stereo unchanged.
        if any(b[3] != 113 for b in bounds):
            raise ValueError(key + ': re-audit lower anchor for updated artwork')
        header = struct.pack('<8s8H2h', b'BS3DAIW\0', 1, 1, slot, 5, 128,
                             anchor_x, anchor_y, ref_height, offset_x, offset_y)
        blob = header + struct.pack('<I', zlib.crc32(header + body)) + body
        prepared.append((key, blob, {'weapon': key, 'slot': slot, 'game': 'AOG',
                          'canvas': 128, 'anchor': [anchor_x, anchor_y], 'refHeight': ref_height,
                          'offset': [offset_x, offset_y], 'bounds': bounds, 'sources': sources,
                          'bytes': len(blob), 'sha256': hashlib.sha256(blob).hexdigest()}))
    destination = output / 'ai-weapons' / 'aog'
    destination.mkdir(parents=True, exist_ok=True)
    for key, blob, _ in prepared:
        path = destination / (key + '.bsw')
        temp = path.with_suffix('.tmp')
        temp.write_bytes(blob)
        temp.replace(path)
    report = {'note': 'LOCAL TEST PACK, not automatically published/embedded. No PS assets.',
              'sourceVswapSha256': manifest['sourceHash'],
              'weapons': [r for _, _, r in prepared]}
    (output / 'ai-weapons' / 'pack-report.json').write_text(json.dumps(report, indent=2))
    for _, _, r in prepared:
        print(r['weapon'], 'scale=', r['refHeight'], 'anchor=', r['anchor'], 'bytes=', r['bytes'])
    print('Install directory:', output / 'ai-weapons')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--lab', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True,
                        help='Local staging root; copy its ai-weapons directory beside VSWAP')
    parser.add_argument('--layout', type=Path, help='Optional per-weapon refHeight/anchor/offset JSON')
    args = parser.parse_args()
    prepare(args.lab, args.output, args.layout)
