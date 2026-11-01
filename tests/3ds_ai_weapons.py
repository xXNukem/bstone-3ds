#!/usr/bin/env python3
"""Actual loader + CRC + production scaler; ASan/UBSan on the host.
Run converter first. No graphics/animation implementation is replaced by mocks.
"""
import json
from pathlib import Path
import re
import runpy
import shutil
import struct
import subprocess
import tempfile
import zlib

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src/bstone/src'
helper = runpy.run_path(str(ROOT / 'tests/3ds_native_options.py'))
scale = helper['function']((SRC / '3d_scale.cpp').read_text(), 'void generic_scale_shape(')
vl = (SRC / 'id_vl.cpp').read_text()
getter = helper['function'](vl, 'bool vid_cfg_ai_weapons() noexcept')
setter = helper['function'](vl, 'void vid_cfg_ai_weapons(bool enabled)')
assert re.search(r'vid_ai_enhanced_weapons"\}.*,?\s*bstone::CVarFlags::archive, false', vl)
assert 'cvar_mgr.add(vid_ai_weapons_cvar);' in vl

prefix = r'''
#define __3DS__ 1
#define BSTONE_ENABLE_AI_WEAPONS 1
#include "bstone_3ds_weapons.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
struct Pref { bool value=false; bool get_bool() const {return value;}
    void set_bool(bool b){value=b;} } vid_ai_weapons_cvar;
'''
prefix += getter + '\n' + setter + r'''
namespace bstone { struct Sprite { static constexpr int dimension=64; }; }
enum class ShapeDrawMode { simple,shaded,player_weapon };
bool useBounceOffset=false,n3ds_stereo_active=true,cloaked_shape=false;
double bounceOffset=6;
int vga_width=400,vga_height=240,vga_ref_width=320,vga_ref_height=200;
int viewwidth=400,vga_area=96000,bufferofs=0;
int vga_3d_view_top_y=0,vga_3d_view_bottom_y=240,centery=120;
int maxscaleshl2=512,shade_max=0,normalshade=1;
std::array<double,400> wallheight{};
std::uint8_t light_data[8192]{};
const std::uint8_t* lightsource=light_data;
std::vector<std::uint8_t> pixels;
std::uint8_t* vga_memory=nullptr;
int vl_get_offset(int base,int x,int y){return base+y*vga_width+x;}
struct Sprite {
 int get_width()const{return 32;} int get_height()const{return 48;}
 int get_left()const{return 16;} int get_top()const{return 16;}
 std::vector<std::int16_t> data=std::vector<std::int16_t>(32*48,7);
 const std::int16_t* get_column(int i)const{assert(i>=0&&i<32);return data.data()+i*48;}
} sprite;
struct Cache { Sprite* cache(int){return &sprite;} } vid_sprite_cache;
'''
suffix = r'''
using namespace bstone::n3ds_weapons;
const std::array<int,6> sw{{484,489,494,499,504,0}},full{{668,673,678,683,688,0}},ps{{722,727,732,737,742,982}};
void init(const std::string& dir,Game game=Game::aog,const std::array<int,6>& ids=sw){initialize(dir,game,ids);}
void render(int id,bool world=false){pixels.assign(vga_area*2,2);std::fill_n(pixels.data(),vga_area,1);vga_memory=pixels.data();generic_scale_shape(viewwidth/2,id,world?200:128,0,world?ShapeDrawMode::simple:ShapeDrawMode::player_weapon);}
void parity(){for(int i=0;i<vga_area;++i){if(n3ds_stereo_active){assert((pixels[i]==1&&pixels[i+vga_area]==2)||pixels[i]==pixels[i+vga_area]);}else assert(pixels[i+vga_area]==2);}}
int main(int argc,char**argv){
 assert(argc==2);const std::string dir=argv[1];
 init(dir+"/good");assert(available()&&!vid_cfg_ai_weapons());
 for(auto bases:{sw,full}){
  init(dir+"/good",Game::aog,bases);vid_cfg_ai_weapons(true);assert(vid_cfg_ai_weapons());
  for(int slot=0;slot<5;++slot)for(int phase=0;phase<5;++phase){
   const auto*f=get_frame(bases[slot]+phase,true);assert(f&&f->dimension==128&&f->anchor_y==110);
   assert(!get_frame(bases[slot]+phase,false));
   assert(f->pixels.size()==std::size_t(f->width*f->height));
  }
 }
 // Actual production pixels, all 25 approved phases, both aspects and slider states.
 init(dir+"/good");vid_cfg_ai_weapons(true);
 for(int width:{320,400})for(bool stereo:{false,true}){
  vga_width=viewwidth=width;vga_area=width*240;n3ds_stereo_active=stereo;
  for(int base:{484,489,494,499,504})for(int phase=0;phase<5;++phase){
   useBounceOffset=false;bounceOffset=6;render(base+phase);const auto stable=pixels;
   for(double position:{0.,2.,4.,6.,8.,10.,12.}){
    bounceOffset=position;useBounceOffset=false;render(base+phase);assert(pixels==stable);
    useBounceOffset=true;render(base+phase);parity();bool bottom=false;
    for(int x=0;x<viewwidth;++x)bottom|=pixels[239*vga_width+x]!=1;
    assert(bottom); // bottom outline cannot float above the view
   }
   useBounceOffset=true;bounceOffset=6;render(base+phase);assert(pixels==stable);
  }
 }
 // OFF is immediate and world drawing cannot use an Enhanced frame.
 vid_cfg_ai_weapons(false);assert(!vid_cfg_ai_weapons());render(484);const auto original=pixels;
 vid_cfg_ai_weapons(true);render(484);assert(pixels!=original);
 vid_cfg_ai_weapons(false);render(484);assert(pixels==original);
 render(484,true);const auto world=pixels;vid_cfg_ai_weapons(true);render(484,true);assert(pixels==world);
 // Persisted ON without a pack is normalized OFF. Edition IDs never cross.
 for(auto scenario:{"absent","wronggame","badcrc","truncated","badphase","badpixel","huge"}){
  init(dir+"/"+scenario);vid_ai_weapons_cvar.value=true;assert(!vid_cfg_ai_weapons());
  for(int p=0;p<5;++p)assert(!get_frame(484+p,true));
 }
 init(dir+"/good",Game::planet_strike,ps);vid_ai_weapons_cvar.value=true;assert(!vid_cfg_ai_weapons());
 init(dir+"/ps-wrong",Game::planet_strike,ps);assert(!available());
 init(dir+"/partial");assert(available());vid_cfg_ai_weapons(true);
 for(int p=0;p<5;++p)assert(!get_frame(489+p,true));
 assert(get_frame(484,true));
 // Once cached, removal cannot produce mixed phases. On switching away, a
 // failed reload pins the entire missing weapon to Original until refresh.
 init(dir+"/volatile");vid_cfg_ai_weapons(true);assert(get_frame(484,true));
 std::remove((dir+"/volatile/ai-weapons/aog/autocharge.bsw").c_str());
 for(int p=0;p<5;++p)assert(get_frame(484+p,true));
 assert(get_frame(489,true));for(int p=0;p<5;++p)assert(!get_frame(484+p,true));
 // Cache release / re-entry (same contract as save/load and level transition)
 // does not reset the game's original phase; caller supplies its current ID.
 init(dir+"/good");vid_cfg_ai_weapons(true);release();assert(get_frame(501,true));
 assert(!get_frame(982,true)); // no BFG substitution
 std::puts("PASS: production loader/CRC, 25 frames, AOG semantic edition IDs, original fallback per weapon, malformed/missing packs, immutable five-phase cache, actual scaler/bob/alpha/eye parity, OFF restoration and archived preference gating");
}
'''


def run():
    pack = ROOT / 'build/ai-weapons-local-pack/ai-weapons'
    if not pack.exists():
        raise RuntimeError('Prepare the local pack first (tools/prepare_ai_weapons.py)')
    with tempfile.TemporaryDirectory(prefix='bstone-ai-weapons-') as d:
        t = Path(d)
        for name in ('good','volatile','partial'):
            shutil.copytree(pack,t/name/'ai-weapons')
        (t/'partial/ai-weapons/aog/protector.bsw').unlink()
        raw = (pack/'aog/autocharge.bsw').read_bytes()
        for name in ('wronggame','badcrc','truncated','badphase','badpixel','huge'):
            b = bytearray(raw)
            if name=='wronggame':struct.pack_into('<H',b,10,2)
            elif name=='badcrc':b[-1]^=1
            elif name=='truncated':b=b[:-20]
            elif name=='badphase':struct.pack_into('<H',b,14,4)
            elif name=='badpixel':struct.pack_into('<H',b,40,256)
            else:b=bytearray(200000)
            if name not in ('badcrc','huge'):
                struct.pack_into('<I',b,28,zlib.crc32(b[:28]+b[32:]))
            target=t/name/'ai-weapons/aog/autocharge.bsw';target.parent.mkdir(parents=True);target.write_bytes(b)
        target=t/'ps-wrong/ai-weapons/ps';target.mkdir(parents=True);(target/'autocharge.bsw').write_bytes(raw)
        test=t/'test.cpp';test.write_text(prefix+scale+suffix)
        exe=t/'test'
        subprocess.run(['c++','-std=c++14','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(SRC),str(test),str(SRC/'bstone_3ds_weapons.cpp'),str(SRC/'bstone_crc32.cpp'),'-o',str(exe)],check=True)
        subprocess.run([str(exe),str(t)],check=True)


if __name__=='__main__':run()
