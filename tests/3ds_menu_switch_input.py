#!/usr/bin/env python3
"""Execute actual HandleMenu/WaitKeyUp/cp_video against held-button traces."""
from pathlib import Path
import runpy
ROOT=Path(__file__).resolve().parents[1]
h=runpy.run_path(str(ROOT/'tests/3ds_native_options.py'))
s=(ROOT/'src/bstone/src/3d_menu.cpp').read_text()
handler=h['function'](s,'std::int16_t HandleMenu(\n\tCP_iteminfo* item_i,\n\tCP_itemtype* items,\n\tvoid(*routine)(std::int16_t w))\n{')
wait=h['function'](s,'void WaitKeyUp()\n{')
delay=h['function'](s,'void TicDelay(\n\tstd::int16_t count)\n{')
video=h['function'](s,'void cp_video(\n\tstd::int16_t)\n{')
h['run'](r'''
#define __3DS__ 1
#define BSTONE_ENABLE_AI_WEAPONS 1
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
enum Direction {dir_None,dir_North,dir_South,dir_West,dir_East};
enum class ScanCode {sc_space,sc_return,sc_escape};
struct KeyboardState{std::array<bool,3> keys{};bool&operator[](ScanCode k){return keys[int(k)];}}Keyboard;
struct ControlInfo{Direction dir=dir_None;bool button0=false,button1=false,button2=false,button3=false;};
struct CP_iteminfo{int x=0,y=0,curpos=0,y_spacing=9,indent=0,amount=3;struct{int on=0;}cursor;};
struct CP_itemtype{int active=1;std::string string="OPTION";void(*routine)(std::int16_t)=nullptr;void(*carousel_func_)(int,bool,bool)=nullptr;};
constexpr unsigned A=1,B=2,UP=4,DOWN=8,LEFT=16,RIGHT=32;
std::vector<unsigned> trace;std::size_t trace_at=0;int polls=0,idles=0,presents=0,tics=1,TimeCount=0,refreshes=0;
char LastASCII=0;bool ingame=false,enhanced=false,aspect=true,crt=true,pack=true;
CP_iteminfo MainItems,video_items;CP_itemtype video_menu[3];
constexpr int video_menu_item_count=3;
constexpr int mvl_widescreen=0,mvl_menu_crt=1,mvl_ai_weapons=2,BACKGROUND_SCREENPIC=0,DISK_SPACE_NEEDED=0;
const char*CANT_PLAY_TXT="";const char*CANT_SAVE_GAME_TXT="";
namespace bstone{namespace n3ds_menu{bool active(){return true;}void idle(){++idles;}void leave_on_next_update(){}}namespace n3ds_weapons{void refresh(){++refreshes;}bool available(){return pack;}const char*status(){return "NO PACK";}}}
void IN_ClearKeysDown(){Keyboard.keys.fill(false);LastASCII=0;}
void ReadAnyControl(ControlInfo*ci){assert(trace_at<trace.size());auto k=trace[trace_at++];++polls;++TimeCount;*ci={};ci->button0=k&A;ci->button1=k&B;Keyboard[ScanCode::sc_space]=k&B;ci->dir=k&UP?dir_North:k&DOWN?dir_South:k&LEFT?dir_West:k&RIGHT?dir_East:dir_None;}
void WaitKeyUp();void TicDelay(std::int16_t);
void CalcTics(){tics=1;++TimeCount;}void CycleColors(){++presents;}void CheckPause(){}
void DrawGun(CP_iteminfo*i,CP_itemtype*,int,std::int16_t*y,int which,int base,void(*)(std::int16_t)){*y=base+which*i->y_spacing;}
void EraseGun(CP_iteminfo*,CP_itemtype*,int,int,int){}void SetTextColor(CP_itemtype*,int){}
void ShadowPrint(const char*,int,int){}void VW_UpdateScreen(){++presents;}
void CP_SaveGame(std::int16_t){}void CP_NewGame(std::int16_t){}void cds_menu_print(const char*){}
bool CheckDiskSpace(int,const char*,void(*)(const char*)){return true;}
struct Assets{bool is_ps()const{return false;}}assets;const Assets&get_assets_info(){return assets;}
bool Confirm(const char*){return true;}const char*CURGAME="";
void MenuFadeOut(){}void MenuFadeIn(){}void ShootSnd(){}void menu_play_esc_pressed_sound(){}
bool vid_cfg_is_widescreen(){return aspect;}void vid_cfg_set_is_widescreen(bool v){aspect=v;}
bool vid_cfg_menu_crt(){return crt;}void vid_cfg_menu_crt(bool v){crt=v;}
bool vid_cfg_ai_weapons(){if(!pack)enhanced=false;return enhanced;}void vid_cfg_ai_weapons(bool v){enhanced=v&&pack;}
void CA_CacheScreen(int){}void video_draw_menu(){++presents;}void video_draw_switch(std::int16_t){}
void vl_update_widescreen(){}void SetupWalls(){}void NewViewSize(){}void SetPlaneViewSize(){}void VL_RefreshScreen(){++presents;}
void Message(const char*){}void IN_Ack(){}
''' + wait + delay + handler + video + r'''
void start(std::vector<unsigned>frames){trace=frames;trace_at=0;polls=idles=presents=refreshes=0;IN_ClearKeysDown();}
void add(std::vector<unsigned>&v,unsigned value,int count){v.insert(v.end(),count,value);}
int main(){
 for(int held:{1,5,60,300}){
  CP_iteminfo info;CP_itemtype entries[3];
  std::vector<unsigned>v{0,A};add(v,A,held);v.push_back(0);v.push_back(A);add(v,A,held);v.push_back(0);v.push_back(B);add(v,B,held);v.push_back(0);v.push_back(A);
  start(v);bool setting=false;int actions=0;
  for(int action=0;action<2;++action){assert(HandleMenu(&info,entries,nullptr)==0);setting=!setting;++actions;}
  assert(!setting&&actions==2);assert(HandleMenu(&info,entries,nullptr)==-1);
  // Holding B cannot skip another parent; a fresh A press is required next.
  assert(HandleMenu(&info,entries,nullptr)==0);assert(idles>=held*3&&presents>0);
  // Exercise actual Enhanced branch and its pack rescan: OFF→ON→OFF.
  v={0,0,DOWN,0,DOWN,0,A};add(v,A,held);v.push_back(0);v.push_back(A);add(v,A,held);v.push_back(0);v.push_back(B);
  start(v);pack=true;enhanced=false;cp_video(0);assert(!enhanced);assert(refreshes==2);
  assert(idles>=held*2); // paced CRT pumping continues during held buttons
 }
 // Initial held A belongs to the previous screen, not a new confirmation.
 start({A,A,A,0,B});CP_iteminfo info;CP_itemtype entries[3];assert(HandleMenu(&info,entries,nullptr)==-1);
 std::puts("PASS: actual menu code, 1/5/60/300 held polls, exactly one action per press, B parent isolation, live CRT idle, D-pad navigation and actual Enhanced OFF-ON-OFF branch");
}
''',extra_flags=['-fsanitize=address,undefined','-fno-omit-frame-pointer'])
