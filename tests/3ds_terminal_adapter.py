#!/usr/bin/env python3
"""Exercise the actual 3DS terminal adapter against mocked platform boundaries.
Run from any directory with Python 3 and a host C++17 compiler.
"""
import pathlib
import shutil
import subprocess
import tempfile

FIXTURES = {
'3d_menu.h': r'''#pragma once
#include <string>
enum activetypes { AT_DISABLED, AT_ENABLED };
struct CP_iteminfo { int curpos; int amount; };
struct CP_itemtype { activetypes active; std::string string; };
std::string cp_get_terminal_label(const CP_iteminfo&,const CP_itemtype*,int);
''',
'bstone_3ds_intro.h': r'''#pragma once
#include <vector>
#include <cstdint>
using u32 = unsigned;
constexpr u32 C2D_Color32(int r,int g,int b,int a) { return r|(g<<8)|(b<<16)|(unsigned(a)<<24); }
void C2D_DrawRectSolid(float,float,float,float,float,u32);
void C2D_ViewTranslate(float,float);
bool C2D_DrawLine(float,float,u32,float,float,u32,float,float);
struct fontstruct { std::uint8_t height; };
struct AssetsInfo { bool is_ps() const { return false; } };
AssetsInfo& get_assets_info();
constexpr int STARTFONT = 0;
extern std::vector<std::vector<std::uint8_t>> grsegs;
void CA_CacheGrChunk(int);
namespace bstone { namespace n3ds_intro {
struct Fonts { std::vector<std::uint8_t> menu; };
Fonts& fonts();
void cache_fonts();
void draw_menu_title(bool);
void draw_centered(const char*,int,bool,int,u32,bool);
int font_text_width(const std::vector<std::uint8_t>&,const char*);
void draw_font(int,int,const char*,const std::vector<std::uint8_t>&,int,u32);
} }
''',
'id_vl.h': r'''#pragma once
#include <array>
#include <vector>
extern bool screenfaded;
extern unsigned char vgapal[768];
extern std::vector<unsigned char> vid_ui_buffer_;
extern std::array<bool,64000> vid_mask_buffer_;
bool vid_cfg_menu_crt() noexcept;
void VL_SetPalette(int,int,const unsigned char*);
void VL_RefreshScreen();
''',
'3ds.h': r'''#pragma once
#include <cstdint>
std::uint64_t osGetTime();
void svcSleepThread(std::int64_t);
''',
'test.cpp': r'''#include "bstone_3ds_menu.h"
#include "bstone_3ds_intro.h"
#include "3d_menu.h"
#include "id_vl.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <tuple>
struct Text { float x,y; std::string value; };
using Rect=std::tuple<float,float,float,float,u32>;
std::vector<Text> texts;
std::vector<Rect> rects;
bool screenfaded=false, last_edition=false;
bool crt_enabled=true;
bool vid_cfg_menu_crt() noexcept { return crt_enabled; }
unsigned char vgapal[768]{};
int palette_restores=0, bottom_draws=0;
int presents=0, sleeps=0, radar_lines=0;
float view_x=0;
std::vector<u32> radar_colors;
void C2D_ViewTranslate(float x,float y) { assert(y==0); view_x+=x; }
AssetsInfo& get_assets_info() { static AssetsInfo data; return data; }
void VL_RefreshScreen() { ++presents; bstone::n3ds_menu::begin_frame(); }
std::uint64_t now=1000;
std::uint64_t osGetTime() { return now; }
void svcSleepThread(std::int64_t ns) { assert(ns>0 && ns<=4000000); ++sleeps; now+=ns/1000000; }
bool C2D_DrawLine(float x0,float y0,u32 color,float x1,float y1,u32,float width,float) {
    assert(view_x==0 && x0>=80 && x0<=240 && x1>=80 && x1<=240);
    assert(y0>=35 && y0<=190 && y1>=35 && y1<=190 && width==7);
    ++radar_lines; radar_colors.push_back(color); return true;
}
std::vector<std::vector<std::uint8_t>> grsegs(5);
void CA_CacheGrChunk(int) { assert(false); }
void VL_SetPalette(int first,int count,const unsigned char* palette) {
    assert(first==0 && count==256 && palette==vgapal); ++palette_restores;
}
void C2D_DrawRectSolid(float x,float y,float,float w,float h,u32 c) { rects.emplace_back(x+view_x,y,w,h,c); }
namespace bstone { namespace n3ds_intro {
Fonts& fonts() { static Fonts f{{1}}; return f; }
void cache_fonts() {}
void draw_centered(const char* t,int y,bool,int,u32,bool) { texts.push_back({160.0f,float(y),t}); }
void draw_menu_title(bool ps) { ++bottom_draws; last_edition=ps; }
int font_text_width(const std::vector<std::uint8_t>&,const char* p) {
    int n=0; for(;*p;++p) n+=*p=='I'?2:(*p=='W'?8:5); return n;
}
void draw_font(int x,int y,const char* t,const std::vector<std::uint8_t>&,int scale,u32) {
    assert(scale==1 || scale==2); texts.push_back({x+view_x,float(y),t});
}
} }
void frame() { texts.clear(); rects.clear(); bstone::n3ds_menu::draw(); }
bool has(const char* s) { for(auto& t:texts) if(t.value==s) return true; return false; }
std::vector<unsigned char> vid_ui_buffer_(64000);
std::array<bool,64000> vid_mask_buffer_{};
std::string cp_get_terminal_label(const CP_iteminfo&,const CP_itemtype* items,int i) {
    auto label=items[i].string; for(auto& c:label) if(c=='\n') c=' '; return label;
}
int main() {
    using namespace bstone::n3ds_menu;
    CP_iteminfo info{0,5};
    CP_itemtype items[]={{AT_ENABLED,"III"},{AT_DISABLED,""},{AT_ENABLED,"WWWW"},
        {AT_DISABLED,""},{AT_ENABLED,"LOGOFF"}};
    begin(false,false); show(info,items,false,false); publish_destination(); begin_frame(); frame();
    assert(has("MAIN TERMINAL") && !has("BLAKE STONE"));
    int last_y=0, footer_y=0;
    for(auto& t:texts) if(t.value=="III" || t.value=="WWWW" || t.value=="LOGOFF") {
        assert(t.x==(400-bstone::n3ds_intro::font_text_width({},t.value.c_str()))/2);
        if(last_y) assert(t.y-last_y==20); last_y=t.y;
    }
    for(auto& t:texts) if(t.value.find("A SELECT")!=std::string::npos) footer_y=t.y;
    assert(footer_y==last_y+20 && footer_y<225);
    for(auto& r:rects) assert(!(std::get<1>(r)==218 && std::get<2>(r)==348));
    // The background has full-width scanlines/grain, never an inset contour.
    rects.clear(); draw_background();
    for(auto& r:rects) {
        const auto [x,y,w,h,c]=r;
        assert(x>=0 && y>=0 && x+w<=400 && y+h<=240);
        assert(w==400 || (w==1 && h==1));
    }
    auto eye=rects; rects.clear(); ++now;
    draw_background(); assert(rects==eye); now+=350; begin_frame(); rects.clear();
    draw_background(); assert(rects!=eye);
    crt_enabled=false; rects.clear(); draw_background(); assert(rects.size()==1);
    now+=1000; assert(!needs_refresh());
    crt_enabled=true; assert(needs_refresh());
    leave_on_next_update(); set_header("Load Mission"); screenfaded=true;
    frame(); assert(has("MAIN TERMINAL"));
    clear_content(); set_header("Load Mission"); publish_destination(); frame();
    assert(has("LOAD GAME") && !has("III") && !screenfaded && palette_restores==1);
    assert(rects.empty()); // Header-only information view draws no band/divider/frame.
    for(auto& t:texts) if(t.value=="LOAD GAME") assert(t.y==12);
    unsigned palette[256]{}; palette[17]=0x123456;
    vid_ui_buffer_[80*320+10]=17; vid_mask_buffer_[80*320+10]=true;
    text_paint(true); pixel(10,80); text_paint(false);
    assert(sample_ui(10,80,palette)==0x58EB79FFu);
    pixel(10,80); assert(sample_ui(10,80,palette)==0x123456FFu); // Art keeps its palette.
    clear_ink(0,0,320,200); assert(sample_ui(10,20,palette)==0);
    const char* from[]={"Save Mission","GAME OPTIONS","VIDEO SETTINGS","SOUND SETTINGS",
        "CONTROL","CHOOSE A MISSION","Difficulty Level","SOUND VOLUME","GAME SWITCHES",
        "GAME SWITCHES 2","MOUSE SENSITIVITY","CUSTOMIZE CONTROLS","HIGH SCORES"};
    const char* to[]={"SAVE GAME","OPTIONS","VIDEO SETTINGS","SOUND SETTINGS",
        "CONTROL SETTINGS","NEW GAME","DIFFICULTY","SOUND VOLUME","GAME SWITCHES",
        "GAME SWITCHES 2","MOUSE SENSITIVITY","CUSTOMIZE CONTROLS","HIGH SCORES"};
    for(int i=0;i<13;++i) { set_header(from[i]); publish_destination(); frame(); assert(has(to[i])); }
    show_document("HISTORY",false); document_page(0,2); publish_destination(); frame();
    assert(has("HISTORY") && has("PAGE 1 / 2"));
    vid_mask_buffer_[8*320+10]=true; vid_ui_buffer_[8*320+10]=17;
    assert(sample_ui(10,28,palette)==0x123456FFu);
    // The complete 152-line logical viewport is readable without scrolling.
    for(int source_y=8;source_y<=159;++source_y) {
        vid_ui_buffer_[source_y*320+11]=17;
        vid_mask_buffer_[source_y*320+11]=true;
        assert(sample_ui(11,28+source_y-8,palette)==0x123456FFu);
    }
    assert(sample_ui(11,27,palette)==0 && sample_ui(11,180,palette)==0);
    for(int i=0;i<8;++i) { now+=150; scroll_document(1); }
    assert(!scroll_document(1));
    vid_mask_buffer_[175*320+10]=true; vid_ui_buffer_[175*320+10]=17;
    assert(sample_ui(10,179,palette)==0x123456FFu); // Last original line is reachable.
    document_page(1,2); assert(sample_ui(10,28,palette)==0x123456FFu);
    draw_bottom(); assert(bottom_draws==1 && !last_edition);
    show_meters(1,7,4,10); frame(); assert(has("MUSIC") && has("4 / 10"));
    show(info,items,true,false); publish_destination(); frame(); assert(has("PAUSED"));
    // Main (9) and pause (10) rows retain native centering and safe help spacing.
    CP_itemtype main_items[12];
    CP_iteminfo main_info{0,12};
    for(int count : {9,10}) {
        for(int i=0;i<12;++i) main_items[i]={AT_ENABLED,i<count?"OPTION "+std::to_string(i):""};
        main_items[count-1].string="LOGOFF";
        set_footer("A SELECT   B BACK   D-PAD NAVIGATE");
        show(main_info,main_items,count==10,false); publish_destination(); frame();
        int previous=0, logoff_y=0, help=0;
        for(auto& t:texts) {
            if(t.value.find("OPTION ")==0 || t.value=="LOGOFF") {
                assert(t.x==(400-bstone::n3ds_intro::font_text_width({},t.value.c_str()))/2);
                if(previous) assert(t.y-previous==16);
                previous=t.y;
                if(t.value=="LOGOFF") logoff_y=t.y;
            }
            if(t.value.find("A SELECT")==0) help=t.y;
        }
        assert(logoff_y>0 && help==logoff_y+20 && help<=225);
    }
    end(); frame(); assert(texts.empty() && !active() && !needs_refresh());
    begin(true,false); draw_bottom(); assert(last_edition); end();
    begin(false,false); show(info,items,false,false); publish_destination();
    begin_frame(); const int old_presents=presents;
    for(int i=0;i<4;++i) idle();
    assert(sleeps>=4 && presents==old_presents+1);
    begin_eye(0,1); frame(); auto left_text=texts;
    assert(view_x==0 && content_shift()==-0.35f);
    begin_eye(1,1); frame();
    assert(view_x==0 && content_shift()==0.35f);
    for(auto& left:left_text) for(auto& right:texts) if(left.value==right.value) {
        const float disparity=left.x-right.x;
        if(left.value=="MAIN TERMINAL") assert(std::abs(disparity+0.7f)<0.001f);
        if(left.value=="III") assert(std::abs(disparity-1.3f)<0.001f);
        if(left.value=="WWWW") assert(disparity==0);
    }
    begin_eye(0,0.5f); assert(std::abs(content_shift()+0.175f)<0.0001f);
    begin_eye(1,0.5f); assert(std::abs(content_shift()-0.175f)<0.0001f);
    begin_eye(0,0.005f); assert(content_shift()==0);
    begin_eye(0,0); frame(); const auto flat=texts;
    begin_eye(1,0); frame(); assert(content_shift()==0 && view_x==0);
    for(unsigned i=0;i<flat.size();++i) assert(flat[i].x==texts[i].x);
    begin_eye(0,1); rects.clear(); draw_background();
    bool back=false; for(auto& r:rects) if(std::get<2>(r)==406) back=std::get<0>(r)==-4.25f;
    assert(back && view_x==0);
    begin_eye(1,1); rects.clear(); draw_background();
    back=false; for(auto& r:rects) if(std::get<2>(r)==406) back=std::get<0>(r)==-1.75f;
    assert(back && view_x==0);
    begin_eye(0,0);
    // All long-page sections share the same comfortable, stable reading plane.
    for(const char* section : {"MISSION BRIEFING","INSTRUCTIONS","HISTORY","CREDITS"}) {
        show_document(section,false); document_page(0,2); publish_destination();
        for(float slider : {0.0f,0.5f,1.0f}) {
            begin_eye(0,slider); frame(); auto document_left=texts;
            assert(std::abs(content_shift()-slider*0.25f)<0.0001f && view_x==0);
            begin_eye(1,slider); frame();
            assert(std::abs(content_shift()+slider*0.25f)<0.0001f && view_x==0);
            for(unsigned i=0;i<texts.size();++i) assert(texts[i].x==document_left[i].x);
        }
        begin_eye(0,1); const float reading=content_shift();
        now+=150; scroll_document(1); assert(content_shift()==reading);
        document_page(1,2); assert(content_shift()==reading);
        begin_loading(); assert(content_shift()==-0.35f); end_loading();
        assert(content_shift()==reading);
    }
    show(info,items,false,false); publish_destination();
    begin_eye(0,1); assert(content_shift()==-0.35f);
    begin_eye(1,1); assert(content_shift()==0.35f);
    begin_eye(0,0);
    const auto old_revision=revision(); const int first_frame=presents;
    begin_loading(); assert(loading() && active() && presents==first_frame+1 && revision()!=old_revision);
    frame(); assert(has("LOADING...") && !has("III") && !has("MAIN TERMINAL"));
    assert(sample_ui(10,80,palette)==0);
    begin_loading(); assert(presents==first_frame+1);
    radar_colors.clear(); radar_lines=0; draw_bottom(); assert(radar_lines==24);
    auto first_ring=radar_colors;
    now+=100; loading_step(); radar_colors.clear(); draw_bottom(); assert(radar_colors!=first_ring);
    crt_enabled=false; now+=16; assert(needs_refresh());
    const int before_step=presents; loading_step(); assert(presents==before_step+1);
    end_loading(); assert(loading());
    end_loading(); assert(!loading() && active());
    frame(); assert(has("III") && !has("LOADING..."));
    begin_loading(); suspend_loading(); assert(!loading() && active());
    frame(); assert(!has("LOADING..."));
    end_loading(); end();
    const int standalone=presents; begin_loading(); assert(active() && loading() && presents==standalone+1);
    end_loading(); assert(!active() && !loading() && !needs_refresh());
    std::puts("PASS: layout, document bounds, idle deadlines/yield, CRT, stereo layers/2D fallback, nested loading/first frame/radar/errors and restore");
}
''',
}

if __name__ == "__main__":
    source = pathlib.Path(__file__).resolve().parents[1] / "src/bstone/src"
    with tempfile.TemporaryDirectory(prefix="bstone-terminal-test-") as directory:
        build = pathlib.Path(directory)
        for name, value in FIXTURES.items():
            (build / name).write_text(value)
        for name in ("bstone_3ds_menu.cpp", "bstone_3ds_menu.h"):
            shutil.copyfile(source / name, build / name)
        executable = build / "terminal-test"
        subprocess.run(["c++", "-std=c++17", "-I" + str(build),
                        str(build / "bstone_3ds_menu.cpp"), str(build / "test.cpp"),
                        "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
