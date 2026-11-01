#!/usr/bin/env python3
"""Execute production layouts, analog sampling and PollKeyboardMove on host.

HID and cvar storage are mocked; the actual mapping/movement algorithms run.
"""
from pathlib import Path
import re
import subprocess
import tempfile

SRC = Path(__file__).resolve().parents[1] / 'src/bstone/src'
source = (SRC / 'id_in.cpp').read_text()
play = (SRC / '3d_play.cpp').read_text()
header = (SRC / 'id_in.h').read_text()

def function(s, signature):
    start = s.index(signature)
    brace = s.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (s[end] == '{') - (s[end] == '}')
        end += 1
    return s[start:end]

scheme = header[header.index('enum class InControlScheme'):]
scheme = scheme[:scheme.index(';') + 1]
layout = source[source.index('struct N3dsButtonBinding'):source.index('} // namespace', source.index('struct N3dsButtonBinding'))]
start = source.index('        // Menus always retain')
mapping = source[start:source.index('\n}\n', start)]
functions = '\n'.join(function(source, signature) for signature in (
    'InControlScheme in_get_control_scheme()', 'void in_set_control_scheme(',
    'const char* in_control_scheme_name(', 'void in_get_3ds_motion('))
code = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
using u32=std::uint32_t;
enum class ScanCode {sc_none,sc_w,sc_s,sc_left_arrow,sc_right_arrow,sc_control,sc_space,sc_a,sc_d,sc_e,sc_q,sc_tab,sc_escape};
constexpr u32 KEY_DUP=1u<<0,KEY_DDOWN=1u<<1,KEY_DLEFT=1u<<2,KEY_DRIGHT=1u<<3,
 KEY_A=1u<<4,KEY_B=1u<<5,KEY_L=1u<<6,KEY_R=1u<<7,KEY_ZL=1u<<8,KEY_ZR=1u<<9,
 KEY_X=1u<<10,KEY_START=1u<<11,KEY_SELECT=1u<<12,KEY_Y=1u<<13,
 KEY_CPAD_UP=1u<<28,KEY_CPAD_DOWN=1u<<29,KEY_CPAD_LEFT=1u<<30,KEY_CPAD_RIGHT=1u<<31,
 KEY_UP=KEY_DUP|KEY_CPAD_UP,KEY_DOWN=KEY_DDOWN|KEY_CPAD_DOWN,
 KEY_LEFT=KEY_DLEFT|KEY_CPAD_LEFT,KEY_RIGHT=KEY_DRIGHT|KEY_CPAD_RIGHT;
struct Keys{std::array<bool,13> values{};bool&operator[](ScanCode s){return values[static_cast<int>(s)];}}Keyboard;
ScanCode LastScan=ScanCode::sc_none;
void IN_ClearKeysDown(){Keyboard.values.fill(false);LastScan=ScanCode::sc_none;}
struct CVar {int value{};int get_int32(){return value;}void set_int32(int v){value=std::max(0,std::min(2,v));}}in_control_scheme_cvar;
bool vid_is_hud=true,menu_active=false;
namespace bstone {namespace n3ds_menu {bool active(){return menu_active;}}}
struct circlePosition{int dx{},dy{};};circlePosition circle{},cstick{};
void hidCircleRead(circlePosition*p){*p=circle;}void hidCstickRead(circlePosition*p){*p=cstick;}
''' + scheme + '\n' + layout + functions + r'''
u32 previous=0;
void map(u32 keys){const auto pressed=keys&~previous;previous=keys;
''' + mapping + r'''
}
enum BindingId{e_bi_run,e_bi_forward,e_bi_backward,e_bi_left,e_bi_right,e_bi_strafe,e_bi_strafe_left,e_bi_strafe_right};
bool running=false;
bool gp_is_always_run(){return running;}
bool in_is_binding_pressed(BindingId id){
 switch(id){
 case e_bi_forward:return Keyboard[ScanCode::sc_w];case e_bi_backward:return Keyboard[ScanCode::sc_s];
 case e_bi_left:return Keyboard[ScanCode::sc_left_arrow];case e_bi_right:return Keyboard[ScanCode::sc_right_arrow];
 case e_bi_strafe_left:return Keyboard[ScanCode::sc_a];case e_bi_strafe_right:return Keyboard[ScanCode::sc_d];
 default:return false;}}
int tics=1,controlx=0,controly=0,strafe_value=0;
constexpr int RUNMOVE=70,BASEMOVE=35;
''' + function(play, 'void PollKeyboardMove()') + r'''
void move(){controlx=controly=strafe_value=0;PollKeyboardMove();}
int main(){
 for(auto s:{InControlScheme::modern,InControlScheme::modern_cstick}){
  in_set_control_scheme(s);map(0);
  map(KEY_R);assert(Keyboard[ScanCode::sc_control]&&!Keyboard[ScanCode::sc_d]);
  map(0);map(KEY_A);assert(Keyboard[ScanCode::sc_right_arrow]&&!Keyboard[ScanCode::sc_control]);
  map(0);map(KEY_Y);assert(Keyboard[ScanCode::sc_left_arrow]);
  map(0);map(KEY_B);assert(Keyboard[ScanCode::sc_space]);
  map(0);map(KEY_CPAD_RIGHT);assert(!Keyboard[ScanCode::sc_e]);
  map(0);map(KEY_DRIGHT);assert(Keyboard[ScanCode::sc_e]);Keyboard[ScanCode::sc_e]=false;
  for(int i=0;i<300;++i){map(KEY_DRIGHT);assert(!Keyboard[ScanCode::sc_e]);}
  map(0);map(KEY_DRIGHT);assert(Keyboard[ScanCode::sc_e]);Keyboard[ScanCode::sc_e]=false;
  map(0);map(KEY_DLEFT);assert(Keyboard[ScanCode::sc_q]);Keyboard[ScanCode::sc_q]=false;
  map(0);map(KEY_X);assert(Keyboard[ScanCode::sc_tab]);map(KEY_X);assert(!Keyboard[ScanCode::sc_tab]);
  map(0);circle={0,156};cstick={};move();assert(controly==-BASEMOVE&&strafe_value==0&&controlx==0);
  circle={156,0};move();assert(strafe_value==BASEMOVE&&controly==0&&controlx==0);
  circle={-156,-156};move();assert(strafe_value<0&&controly>0&&std::hypot(strafe_value,controly)<=BASEMOVE);
  circle={0,0};cstick={156,0};move();assert(controlx==(s==InControlScheme::modern_cstick?BASEMOVE:0));
  cstick={-156,0};move();assert(controlx==(s==InControlScheme::modern_cstick?-BASEMOVE:0));
  cstick={0,156};move();assert(controlx==0); // No fabricated vertical freelook.
  cstick={156,0};map(KEY_A);move();assert(controlx==BASEMOVE);map(0);
  // Same maximum run/walk speed and tick scaling as the existing movement path.
  cstick={};circle={0,156};running=true;tics=3;move();assert(controly==-3*RUNMOVE);
  running=false;tics=1;
  // Menus use Classic navigation/confirmation and cannot retain a fire/turn.
  map(KEY_R);menu_active=true;map(KEY_R);assert(!Keyboard[ScanCode::sc_control]);
  map(0);map(KEY_A);assert(Keyboard[ScanCode::sc_control]);
  int f,st,tr;in_get_3ds_motion(f,st,tr);assert(f==0&&st==0&&tr==0);
  menu_active=false;map(0);assert(!Keyboard[ScanCode::sc_control]);
 }
 // Exercise the entire analog input range, drift deadzones and normalized diagonals.
 in_set_control_scheme(InControlScheme::modern_cstick);map(0);
 for(int x=-180;x<=180;++x)for(int y=-180;y<=180;++y){
  circle={x,y};cstick={x,y};int f,st,tr;in_get_3ds_motion(f,st,tr);
  assert(std::hypot(f,st)<=1001&&std::abs(tr)<=1000);
  if(std::abs(x)<=20)assert(st==0);if(std::abs(y)<=20)assert(f==0);
  if(std::abs(x)<=15)assert(tr==0);
 }
 // Returning to Classic ignores analog sampling and keeps existing button roles.
 in_set_control_scheme(InControlScheme::classic);map(0);map(KEY_A|KEY_R);
 assert(Keyboard[ScanCode::sc_control]&&Keyboard[ScanCode::sc_d]);
 int f,st,tr;in_get_3ds_motion(f,st,tr);assert(f==0&&st==0&&tr==0);
 std::puts("PASS: production Modern/C-Stick layouts, 130321 analog positions, actual movement speeds/ticks, drift/diagonal bounds, held weapons, menus, layout changes and Classic restoration; physical feel pending");
}
'''
with tempfile.TemporaryDirectory(prefix='bstone-modern-controls-') as directory:
    p = Path(directory)
    (p / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++14', '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                    str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
assert 'CVarFlags::archive, 0, 0, static_cast<int>(InControlScheme::count) - 1' in source
