// SPDX-License-Identifier: GPL-2.0-or-later
#include "bstone_3ds_launcher.h"
#include "bstone_3ds_installation.h"
#include "bstone_3ds_display.h"
#include "bstone_3ds_intro.h"
#include "bstone_3ds_menu.h"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <string>
namespace bstone { namespace n3ds_launcher {
namespace {
constexpr const char* root = "sdmc:/3ds/bstone";
const u32 bright = C2D_Color32(168,255,179,255), green = C2D_Color32(88,235,121,255);
const u32 dim = C2D_Color32(76,153,93,255), warning = C2D_Color32(220,182,92,255);
// Hand-authored resource-free 5x7 startup glyphs. No game font is loaded.
const std::vector<std::uint8_t>& font()
{
    static const auto data = [] {
        std::vector<std::uint8_t> out(sizeof(fontstruct)+256*42);
        auto* f = reinterpret_cast<fontstruct*>(out.data()); f->height=7;
        for (int i=0;i<256;++i) { f->width[i]=6;f->location[i]=sizeof(fontstruct)+i*42; }
        const char* chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:./-+?[]()%=_";
        const std::array<unsigned char,7> glyphs[]={
        {{14,17,17,31,17,17,17}},{{30,17,17,30,17,17,30}},{{14,17,16,16,16,17,14}},{{30,17,17,17,17,17,30}},
        {{31,16,16,30,16,16,31}},{{31,16,16,30,16,16,16}},{{14,17,16,23,17,17,15}},{{17,17,17,31,17,17,17}},
        {{14,4,4,4,4,4,14}},{{7,2,2,2,18,18,12}},{{17,18,20,24,20,18,17}},{{16,16,16,16,16,16,31}},
        {{17,27,21,21,17,17,17}},{{17,25,21,19,17,17,17}},{{14,17,17,17,17,17,14}},{{30,17,17,30,16,16,16}},
        {{14,17,17,17,21,18,13}},{{30,17,17,30,20,18,17}},{{15,16,16,14,1,1,30}},{{31,4,4,4,4,4,4}},
        {{17,17,17,17,17,17,14}},{{17,17,17,17,17,10,4}},{{17,17,17,21,21,21,10}},{{17,17,10,4,10,17,17}},
        {{17,17,10,4,4,4,4}},{{31,1,2,4,8,16,31}},
        {{14,17,19,21,25,17,14}},{{4,12,4,4,4,4,14}},{{14,17,1,2,4,8,31}},{{30,1,1,14,1,1,30}},
        {{2,6,10,18,31,2,2}},{{31,16,16,30,1,1,30}},{{14,16,16,30,17,17,14}},{{31,1,2,4,8,8,8}},
        {{14,17,17,14,17,17,14}},{{14,17,17,15,1,1,14}},
        {{0,4,4,0,4,4,0}},{{0,0,0,0,0,6,6}},{{1,2,2,4,8,8,16}},{{0,0,0,31,0,0,0}},
        {{0,4,4,31,4,4,0}},{{14,17,1,2,4,0,4}},{{14,8,8,8,8,8,14}},{{14,2,2,2,2,2,14}},
        {{2,4,8,8,8,4,2}},{{8,4,2,2,2,4,8}},{{17,2,4,8,16,17,0}},{{0,31,0,31,0,0,0}},{{0,0,0,0,0,0,31}}};
        for (int i=0;chars[i];++i) for (int y=0;y<7;++y) for (int x=0;x<5;++x)
        { out[f->location[static_cast<unsigned char>(chars[i])]+y*6+x]=(glyphs[i][y]>>(4-x))&1; }
        return out;
    }();
    return data;
}
void text(int x,int y,const std::string& s,u32 color=green,int scale=1)
{ n3ds_intro::draw_font(x,y,s.c_str(),font(),scale,color); }
void centered(int width,int y,const std::string& s,u32 color=green,int scale=1,float shift=0)
{ text(int((width-int(s.size())*6*scale)/2+shift),y,s,color,scale); }
void top_screen(int selected,const n3ds_install::Installation& e,bool exiting,bool pending,const std::string& message,int eye,float slider)
{
    C2D_TargetClear(n3ds_display::top(eye!=0),C2D_Color32(2,9,5,255));C2D_SceneBegin(n3ds_display::top(eye!=0));C2D_ViewReset();
    n3ds_menu::draw_ambient_background(400,240,osGetTime());
    centered(400,26,"BLAKE STONE 3DS",bright,2);
    centered(400,62,exiting?"EXIT TO HOMEBREW LAUNCHER?":"SELECT GAME",dim);
    for (int i=0;i<2;++i)
    {
        const std::string label=i?"PLANET STRIKE":"ALIENS OF GOLD";const int y=105+i*38;
        const float shift=(eye? -1.f:1.f)*slider*(selected==i?.9f:.15f);
        const int x=(400-int(label.size())*12)/2;
        if (i==selected) { C2D_DrawRectSolid(x-18+shift,y-5,.7f,label.size()*12+36,24,C2D_Color32(16,57,28,255));text(x-14+int(shift),y+3,">",bright); }
        centered(400,y,label,selected==i?bright:green,2,shift);
    }
    centered(400,184,std::string(n3ds_install::edition_name(e.edition))+(e.legacy?" / LEGACY LOCATION":" / GAMES DIRECTORY"),dim);
    centered(400,201,exiting?"A EXIT   B BACK":pending?"VERIFYING BEFORE START...":message.empty()?"A START   B EXIT   X RESCAN":message,warning);
    centered(400,223,"D-PAD / CIRCLE PAD SELECT   LEFT/RIGHT EDITION",dim);
}
void bottom_screen(const n3ds_install::Installation& e,const n3ds_install::Scanner& scanner)
{
    using namespace n3ds_install;
    C2D_TargetClear(n3ds_display::bottom(),C2D_Color32(2,9,5,255));C2D_SceneBegin(n3ds_display::bottom());C2D_ViewReset();
    n3ds_menu::draw_ambient_background(320,240,osGetTime());
    centered(320,9,"BLAKE STONE",bright,2);
    centered(320,30,e.edition==Edition::planet_strike?"PLANET STRIKE":"ALIENS OF GOLD",green);
    text(10,47,std::string("EDITION: ")+edition_name(e.edition));
    text(10,58,std::string("VERSION: ")+assets_version_label(e.version),dim);
    text(10,69,"FILES PRESENT: "+std::to_string(e.found)+" / "+std::to_string(e.files.size()));
    text(10,80,state_name(e.state),e.state==State::ready?bright:warning);
    text(10,91,e.state==State::verifying?"HASHED: "+std::to_string(e.checked)+" / "+std::to_string(e.files.size()):e.state==State::ready?"ALL FILES MATCH ONE KNOWN RELEASE":"P=PRESENT  !=EMPTY  -=MISSING",dim);
    int y=106;
    for(const auto& file:e.files)
    { text(12,y,std::string(e.state==State::ready?"V ":!file.present?"- ":file.size==0?"! ":"P ")+file.name,file.present&&file.size?green:warning);y+=8; }
    text(10,209,e.directory,dim);
    if (e.edition!=Edition::planet_strike)
    {
        const int full=std::max(scanner.entries[0].found,scanner.entries[2].found);
        const int sw=std::max(scanner.entries[1].found,scanner.entries[3].found);
        text(10,224,"AOG PRESENT: FULL "+std::to_string(full)+"/12  SW "+std::to_string(sw)+"/10",dim);
    }
}
}
bool run()
{
    using namespace n3ds_install;
    n3ds_display::initialize();Scanner scanner{root};
    int selected=0,aog_current=scanner.preferred(false),current=aog_current,last_nav=0;u32 previous=~u32{};
    bool exiting=false,pending=false;std::string message;
    while (aptMainLoop())
    {
        SDL_PumpEvents();hidScanInput();const u32 keys=hidKeysHeld(),pressed=keys&~previous;previous=keys;
        if ((keys&(KEY_START|KEY_SELECT))==(KEY_START|KEY_SELECT)) { return false; }
        circlePosition circle{};hidCircleRead(&circle);
        const int nav=(keys&KEY_UP)||circle.dy>40?-1:(keys&KEY_DOWN)||circle.dy< -40?1:0;
        if (!exiting && nav && !last_nav) { selected^=1;current=selected?4:aog_current;pending=false;message.clear(); }
        last_nav=nav;
        if (!exiting && (pressed&(KEY_LEFT|KEY_RIGHT)) && !selected)
        {
            int next=current;
            for(int n=0;n<4;++n) { next=(next+((pressed&KEY_LEFT)?3:1))%4;if(scanner.entries[next].found) {current=next;break;} }
            aog_current=current;pending=false;message.clear();
        }
        if (pressed&KEY_B) { exiting=!exiting;pending=false;message.clear(); }
        if (!exiting && (pressed&KEY_X)) { scanner.rescan();aog_current=scanner.preferred(false);current=selected?4:aog_current;pending=false;message.clear(); }
        if ((pressed&KEY_A) && !(pressed&KEY_B))
        {
            if(exiting) { return false; }
            pending=true;message.clear();
        }
        scanner.step(current);const auto& e=scanner.entries[current];
        if (pending && e.state==State::ready)
        {
            auto chosen=make_selection(e,root);
            if (chosen.active && create_directories(chosen.profile_directory)) { selection()=std::move(chosen);C3D_FrameSync();return true; }
            pending=false;
            if (!chosen.active)
            {
                scanner.entries[current].state=State::invalid;
                scanner.entries[current].version=AssetsVersion::none;
                message="FILES CHANGED - X TO RESCAN";
            }
            else { message="PROFILE DIRECTORY IS NOT WRITABLE"; }
        }
        if (pending && (e.state==State::absent||e.state==State::incomplete||e.state==State::invalid))
        { pending=false;message="CANNOT START - CHECK FILES BELOW"; }
        const float slider=osGet3DSliderState();
        if (!C3D_FrameBegin(C3D_FRAME_SYNCDRAW)) { continue; }
        top_screen(selected,e,exiting,pending,message,0,slider);
        top_screen(selected,e,exiting,pending,message,1,slider);
        bottom_screen(e,scanner);C3D_FrameEnd(0);
    }
    return false;
}
} }
