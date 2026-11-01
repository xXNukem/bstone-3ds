#!/usr/bin/env python3
"""Original pixel-grid BS insignia. Standard library only; not a build dependency.
SPDX-License-Identifier: MIT
"""
import struct
import zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
pixels=[[(3,12,8,255) for _ in range(48)] for _ in range(48)]
def rect(x,y,w,h,color):
 for row in range(y,y+h):
  for col in range(x,x+w):pixels[row][col]=color
# Stepped military terminal insignia with original BS lettering.
rect(7,3,34,42,(12,48,29,255));rect(3,7,42,34,(12,48,29,255));rect(5,9,38,30,(6,24,15,255));rect(9,5,30,38,(6,24,15,255))
for x,y,w,h in [(9,3,30,2),(3,9,2,30),(43,9,2,30),(9,43,30,2)]:rect(x,y,w,h,(47,144,78,255))
letters={'B':[30,17,17,30,17,17,30],'S':[15,16,16,14,1,1,30]}
for ch,start in [('B',7),('S',26)]:
 for y,bits in enumerate(letters[ch]):
  for x in range(5):
   if bits&(1<<(4-x)):
    rect(start+x*3+1,9+y*3+1,3,3,(23,78,44,255))
    rect(start+x*3,8+y*3,3,3,(155,244,164,255) if y<3 else (78,202,113,255))
rect(10,32,28,1,(40,121,67,255));rect(18,34,12,1,(86,220,128,255))
# Tiny three-dot console signature, deliberately readable at 24 pixels too.
for x in (16,22,28):rect(x,38,3,2,(155,244,164,255))
def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data))
def png(path,size):
 data=b''.join(b'\0'+b''.join(bytes(pixels[y*48//size][x*48//size]) for x in range(size)) for y in range(size))
 path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',size,size,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(data,9))+chunk(b'IEND',b''))
for size in (48,24):png(ROOT/'assets'/('bstone-3ds-icon-%d.png'%size),size)
print(ROOT/'assets/bstone-3ds-icon-48.png')
