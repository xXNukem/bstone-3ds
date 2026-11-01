#!/usr/bin/env python3
"""Exercise the actual shared Citro owner; no SDL/gfx/HID reinitialization."""
from pathlib import Path
import tempfile,subprocess
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'src/bstone/src'
STUB=r'''
#pragma once
#include <cstddef>
struct C3D_RenderTarget{int id;};
extern int init3,init2,fini3,fini2,created,deleted,fail_target,top_format,bottom_format;
extern bool fail3,fail2;
constexpr int C3D_DEFAULT_CMDBUF_SIZE=1,C2D_DEFAULT_MAX_OBJECTS=1,GFX_BOTTOM=0,GFX_TOP=1,GFX_LEFT=0,GFX_RIGHT=1,GSP_BGR8_OES=0;
inline bool C3D_Init(int){++init3;return !fail3;}inline bool C2D_Init(int){++init2;return !fail2;}
inline void C3D_Fini(){++fini3;}inline void C2D_Fini(){++fini2;}
inline void C2D_Prepare(){}inline void C3D_FrameSync(){}inline void gfxSet3D(bool){}
inline void gfxSetScreenFormat(int screen,int format){(screen==GFX_TOP?top_format:bottom_format)=format;}
inline C3D_RenderTarget*C2D_CreateScreenTarget(int,int){if(top_format!=GSP_BGR8_OES||bottom_format!=GSP_BGR8_OES)throw 1;++created;return created==fail_target?nullptr:new C3D_RenderTarget{created};}
inline void C3D_RenderTargetDelete(C3D_RenderTarget*p){++deleted;delete p;}
'''
TEST=r'''
#include "bstone_3ds_display.h"
#include <cassert>
#include <cstdio>
int init3=0,init2=0,fini3=0,fini2=0,created=0,deleted=0,fail_target=0,top_format=-1,bottom_format=-1;bool fail3=false,fail2=false;
using namespace bstone::n3ds_display;
void reset(){init3=init2=fini3=fini2=created=deleted=fail_target=0;top_format=bottom_format=-1;fail3=fail2=false;}
int main(){
 initialize();const auto*original=top();assert(original&&top(true)&&bottom());initialize();assert(top()==original&&init3==1&&init2==1&&created==3);
 shutdown();shutdown();assert(fini3==1&&fini2==1&&deleted==3&&!top()&&!bottom());
 for(int i=1;i<=3;++i){reset();fail_target=i;try{initialize();assert(false);}catch(...){}assert(fini3==1&&fini2==1&&created==3&&deleted==2);shutdown();assert(fini3==1);}
 reset();fail3=true;try{initialize();assert(false);}catch(...){}assert(init3==1&&init2==0&&fini3==0);
 reset();fail2=true;try{initialize();assert(false);}catch(...){}assert(fini3==1&&fini2==0&&created==0);
 reset();initialize();shutdown();assert(deleted==3&&fini3==1&&fini2==1);
 std::puts("PASS: production shared display lifecycle; selector→game reuse, idempotent close, all partial-failure cleanup paths, no gfx/HID ownership changes");
}
'''
with tempfile.TemporaryDirectory(prefix='bstone-display-') as d:
 p=Path(d);(p/'citro2d.h').write_text(STUB);(p/'test.cpp').write_text(TEST)
 subprocess.run(['c++','-std=c++14','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+d,'-I'+str(SRC),str(p/'test.cpp'),str(SRC/'bstone_3ds_display.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
code=(SRC/'bstone_3ds_display.cpp').read_text();assert not any(x in code for x in ['gfxInit(', 'gfxExit(', 'hidInit(', 'hidExit('])
main=(SRC/'3d_main.cpp').read_text();body=main[main.index('int main(int argc, char** argv)'):main.index('void InitDestPath()')]
assert body.index('make_system_mgr')<body.index('n3ds_launcher::run()')<body.index('freed_main()')
assert body.index('n3ds_display::shutdown()')<body.index('sys_system_mgr = nullptr')
assert 'args_3ds' not in body and '"--aog"' not in body
