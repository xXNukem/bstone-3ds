#!/usr/bin/env python3
"""Production catalog/scanner/SHA-1/profile routing + actual launcher input loop.
Full/PS disk fixtures are deliberately INVALID; only licensed shareware supplied
by --shareware-dir is verified end-to-end. No claim of running either full game.
"""
import argparse
from pathlib import Path
import runpy
import shutil
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'src/bstone/src'
helper=runpy.run_path(str(ROOT/'tests/3ds_native_options.py'))
runbody=helper['function']((SRC/'bstone_3ds_launcher.cpp').read_text(),'bool run()\n{')
CPP=r'''
#include "bstone_3ds_installation.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
using u32=unsigned;
constexpr u32 KEY_UP=1,KEY_DOWN=2,KEY_LEFT=4,KEY_RIGHT=8,KEY_A=16,KEY_B=32,KEY_X=64,KEY_START=128,KEY_SELECT=256;
struct circlePosition{int dx=0,dy=0;};
std::vector<u32> trace;int frame_index=-1,max_frames=512,top_frames=0,bottom_frames=0,ps_frames=0,exit_frames=0;
bool circle_down=false;
bool aptMainLoop(){return ++frame_index<max_frames;}
void SDL_PumpEvents(){}void hidScanInput(){}
u32 hidKeysHeld(){return frame_index<int(trace.size())?trace[frame_index]:0;}
void hidCircleRead(circlePosition*p){p->dy=circle_down&&frame_index==1?-80:0;}
float osGet3DSliderState(){return 0.;}
void C3D_FrameSync(){}constexpr int C3D_FRAME_SYNCDRAW=0;
bool C3D_FrameBegin(int){return true;}void C3D_FrameEnd(int){}
namespace bstone{namespace n3ds_display{void initialize(){}}namespace n3ds_launcher{
const char*root=nullptr;
void top_screen(int selected,const n3ds_install::Installation&e,bool exiting,bool,const std::string&,int,float){++top_frames;if(selected){++ps_frames;assert(e.edition==n3ds_install::Edition::planet_strike);}if(exiting)++exit_frames;}
void bottom_screen(const n3ds_install::Installation&,const n3ds_install::Scanner&){++bottom_frames;}
'''+runbody+r'''
}}
using namespace bstone::n3ds_install;
void finish(Scanner&s,int i){int steps=0;while(s.entries[i].state==State::present||s.entries[i].state==State::verifying){s.step(i);assert(++steps<10000);}}
bool launch(const std::string&dir,std::vector<u32>buttons,int count=512,bool circle=false){selection()={};bstone::n3ds_launcher::root=dir.c_str();trace=buttons;frame_index=-1;top_frames=bottom_frames=ps_frames=exit_frames=0;max_frames=count;circle_down=circle;return bstone::n3ds_launcher::run();}
int main(int argc,char**argv){
 assert(argc==2);const std::string base=argv[1];
 assert(!create_directories(base+"/legacy/bstone_config.txt/child"));
 // All nine known signature sets, never a partial or mixed set.
 for(auto v:{AssetsVersion::aog_sw_v1_0,AssetsVersion::aog_sw_v2_0,AssetsVersion::aog_sw_v2_1,AssetsVersion::aog_sw_v3_0,AssetsVersion::aog_full_v1_0,AssetsVersion::aog_full_v2_0,AssetsVersion::aog_full_v2_1,AssetsVersion::aog_full_v3_0,AssetsVersion::ps}){
  auto e=v==AssetsVersion::ps?Edition::planet_strike:int(v)<int(AssetsVersion::aog_full_v1_0)?Edition::aog_shareware:Edition::aog_full;
  std::vector<File>files;for(auto&r:resources_for_version(v)){File f;f.hash=r.hash_string;files.push_back(f);}
  assert(identify_version(e,files)==v);files.pop_back();assert(identify_version(e,files)==AssetsVersion::none);
 }
 Scanner empty(base+"/empty");assert(empty.entries.size()==5);for(auto&e:empty.entries)assert(e.state==State::absent&&e.found==0);
 Scanner sw(base+"/sw");assert(sw.preferred(false)==1);assert(sw.entries[1].state==State::present);sw.step(1);assert(sw.entries[1].state==State::verifying);finish(sw,1);assert(sw.entries[1].state==State::ready&&sw.entries[1].version==AssetsVersion::aog_sw_v3_0);
 auto chosen=make_selection(sw.entries[1],base+"/sw");assert(chosen.active&&chosen.profile_directory==base+"/sw/profiles/aog_sw/");
 Scanner legacy(base+"/legacy");assert(legacy.preferred(false)==3);finish(legacy,3);auto old=make_selection(legacy.entries[3],base+"/legacy");assert(old.active&&old.legacy_directory==base+"/legacy/");
 const std::string save="bstone_aog_sw_saved_game_0";assert(profile_read_path(old,save)==base+"/legacy/"+save);assert(profile_read_path(old,"bstone_config.txt")==base+"/legacy/bstone_config.txt");
 assert(create_directories(old.profile_directory));std::FILE*f=std::fopen((old.profile_directory+save).c_str(),"wb");assert(f);std::fputs("new",f);std::fclose(f);assert(profile_read_path(old,save)==old.profile_directory+save);
 // PS never reads legacy AOG root config/saves.
 old.legacy_directory.clear();old.profile_directory=base+"/legacy/profiles/ps/";assert(profile_read_path(old,"bstone_config.txt")==old.profile_directory+"bstone_config.txt");
 for(auto name:{"partial","empty-file"}){Scanner s(base+"/"+name);assert(s.entries[1].state==State::incomplete);assert(!make_selection(s.entries[1],base).active);}
 for(auto name:{"full","ps","both"}){Scanner s(base+"/"+name);const int i=std::string(name)=="ps"?4:0;assert(s.entries[i].state==State::present);finish(s,i);assert(s.entries[i].state==State::invalid);assert(!make_selection(s.entries[i],base).active);}
 Scanner lower(base+"/lower");finish(lower,1);assert(lower.entries[1].state==State::ready);
 // Real executable startup loop: only launch verified SW; incomplete/wrong data never leave selector.
 assert(launch(base+"/sw",{0,KEY_A}));assert(selection().active&&selection().version==AssetsVersion::aog_sw_v3_0&&bottom_frames>10);
 assert(launch(base+"/legacy",{0,KEY_A}));assert(selection().legacy_directory==base+"/legacy/");
 assert(!launch(base+"/empty",{0,KEY_A},20));assert(!selection().active&&bottom_frames==20);
 assert(!launch(base+"/full",{0,KEY_A},40));assert(!selection().active);
 assert(!launch(base+"/ps",{0,KEY_DOWN,0,KEY_A},40));assert(ps_frames>0&&!selection().active);
 assert(!launch(base+"/empty",{0,0,0},4,true));assert(ps_frames>0);
 // Both editions present: prefer Full, explicit RIGHT can select verified SW.
 assert(launch(base+"/both",{0,KEY_RIGHT,0,KEY_A}));assert(selection().edition==Edition::aog_shareware);
 // Held buttons cannot repeat a confirmation or cancel. Both screens keep rendering.
 std::vector<u32>buttons{0};buttons.insert(buttons.end(),100,KEY_B);buttons.push_back(0);buttons.push_back(KEY_A);
 assert(!launch(base+"/empty",buttons,120));assert(exit_frames>=190&&!selection().active);
 std::puts("PASS: production scanner/SHA-1, all nine signature vectors, real SW3.0, uppercase/lowercase, Full preference/SW override, missing/incomplete/invalid/legacy layouts, isolated profile read fallback, actual selector D-pad/Circle/A/B/held inputs and both-screen frame pumping. Full/PS gameplay NOT tested.");
}
'''

def run(shareware):
    names=('AUDIOHED','AUDIOT','IANIM','MAPHEAD','MAPTEMP','SANIM','VGADICT','VGAGRAPH','VGAHEAD','VSWAP')
    with tempfile.TemporaryDirectory(prefix='bstone-launcher-') as directory:
        t=Path(directory)
        def copy_sw(dest,lower=False):
            dest.mkdir(parents=True,exist_ok=True)
            for n in names:
                src=shareware/(n+'.BS1')
                if not src.is_file():src=shareware/(n+'.bs1').lower()
                shutil.copyfile(src,dest/((n+'.BS1').lower() if lower else n+'.BS1'))
        (t/'empty').mkdir();copy_sw(t/'sw/games/aog');copy_sw(t/'legacy');copy_sw(t/'lower/games/aog',True)
        (t/'legacy/bstone_config.txt').write_text('old configuration')
        (t/'legacy/bstone_aog_sw_saved_game_0').write_text('old save sentinel (not a real game save)')
        copy_sw(t/'empty-file/games/aog');(t/'empty-file/games/aog/AUDIOT.BS1').write_bytes(b'')
        (t/'partial/games/aog').mkdir(parents=True);(t/'partial/games/aog/VSWAP.BS1').write_bytes(b'wrong')
        full=(*names[:2],'EANIM','GANIM',*names[2:])
        ps=(*names[:2],'EANIM',*names[2:5],*names[6:])
        for scenario,folder,ext,required in [('full','aog','BS6',full),('ps','ps','VSI',ps),('both','aog','BS6',full),('both','ps','VSI',ps)]:
            dest=t/scenario/'games'/folder;dest.mkdir(parents=True,exist_ok=True)
            for n in required:(dest/(n+'.'+ext)).write_bytes(b'invalid retail-data fixture; not commercial artwork')
        copy_sw(t/'both/games/aog')
        source=t/'test.cpp';source.write_text(CPP);exe=t/'test'
        files=['bstone_3ds_installation.cpp','bstone_game_catalog.cpp','bstone_sha1.cpp','bstone_exception.cpp','bstone_source_location.cpp']
        subprocess.run(['c++','-std=c++14','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(SRC),str(source),*[str(SRC/f) for f in files],'-o',str(exe)],check=True)
        subprocess.run([str(exe),str(t)],check=True)
        assert (t/'legacy/bstone_config.txt').read_text()=='old configuration'
        assert (t/'legacy/bstone_aog_sw_saved_game_0').read_text().startswith('old save sentinel')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);group=p.add_mutually_exclusive_group(required=True);group.add_argument('--shareware-dir',type=Path);group.add_argument('--shareware-zip',type=Path);a=p.parse_args()
    if a.shareware_dir:run(a.shareware_dir)
    else:
        import zipfile
        with tempfile.TemporaryDirectory(prefix='bstone-original-sw-') as directory:
            with zipfile.ZipFile(a.shareware_zip) as archive:
                for name in archive.namelist():
                    if name.upper().endswith('.BS1'):(Path(directory)/Path(name).name).write_bytes(archive.read(name))
            run(Path(directory))
