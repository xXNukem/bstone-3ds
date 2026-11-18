#!/usr/bin/env python3
"""Production guide construction, parser, font acquisition and page navigation.

GPU, resource IO and HID are boundaries mocked here; this is not an ARM11 test.
--baseline reproduces both unsafe parser alignment and an uncached font using
the release source. The font check exercises the presenter startup followed by
the same font-width access used by its centering handler.
"""
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src/bstone/src'


def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


baseline = '--baseline' in sys.argv
source = (subprocess.check_output(['git', 'show', 'b32b818b:src/bstone/src/jm_tp.cpp'],
                                 cwd=ROOT).decode() if baseline else (SRC / 'jm_tp.cpp').read_text())
menu = (SRC / '3d_menu.cpp').read_text()
header = (SRC / 'jm_tp.h').read_text()
defines = '\n'.join(line for line in header.splitlines() if line.startswith('#define ') and
                    not line.startswith(('#define ch_width', '#define TP_INIT_DISPLAY_STR')))
struct = header[header.index('struct PresenterInfo'):header.index('enum tpCacheType')]
build_guide = menu[menu.index('        const std::string gameplay ='):menu.index('        classic_script.assign')]
init = function(source, 'void TP_InitScript(')
prefix = function(source, 'void TP_Presenter(').split('\tflags = fl_presenting | fl_startofline;')[0] + '\n}\n'
helper = '' if baseline else function(source, 'static std::uint16_t TP_ReadCode(')
page_start = source.index("\n\t\tcase TP_CNVT_CODE('E', 'P'):", source.index('void TP_HandleCodes('))
page_end = source.index('\n\t\t\t// EXIT PRESENTER', page_start)
page = source[page_start:page_end]
code = r'''
#define __3DS__ 1
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#define BSTONE_THROW_STATIC_SOURCE(s) throw std::runtime_error(s)
''' + defines + '\n' + struct + r'''
enum class InControlScheme {classic,modern,modern_cstick};
const char* in_control_scheme_name(InControlScheme s){return s==InControlScheme::classic?"CLASSIC":s==InControlScheme::modern?"MODERN":"MODERN + C-STICK";}
std::string guide(InControlScheme scheme){
''' + build_guide + r'''
return manual;
}
constexpr int STARTFONT=0;
struct fontstruct {int height=10; unsigned char width[256]{};};
std::array<std::vector<unsigned char>,5> grsegs;
fontstruct* font=nullptr;
PresenterInfo* pi;
int bgcolor,ltcolor,dkcolor,shcolor,xl,yl,xh,yh,music_num,flags,cur_x,cur_y,px,py;
int numanims,disp_str_num,old_fontnumber,fontnumber,stemp;
const char* first_ch;const char* first_ch_copy;

void TP_PurgeAllGfx(){}void TP_CachePage(const char*){}
void CA_CacheGrChunk(int index){
 assert(index==4);grsegs[index].resize(sizeof(fontstruct));
 auto*f=reinterpret_cast<fontstruct*>(grsegs[index].data());f->height=10;
 for(auto&w:f->width)w=5;
}
int TP_LineCommented(const char*){return 0;}
''' + helper + init + prefix + r'''
enum Direction{dir_None,dir_North,dir_South,dir_West,dir_East};
struct ControlInfo{Direction dir=dir_None;bool button0=false,button1=false;};
enum class ScanCode{sc_page_up,sc_page_down,sc_space,sc_return,sc_escape};
struct Keys{std::array<bool,5> data{};bool&operator[](ScanCode s){return data[int(s)];}}Keyboard;
bool EscPressed,screenfaded=false,tp_handle_codes_can_peek_prev_2;
ScanCode TPscan;int old_bgcolor=0,anim_bgcolor=-1;
std::vector<ControlInfo> input;std::size_t input_at;int frames;
void ReadAnyControl(ControlInfo*ci){assert(input_at<input.size());*ci=input[input_at++];}
void CycleColors(){}void CalcTics(){}void TP_AnimatePage(int){}
void VW_UpdateScreen(){++frames;}void VW_FadeIn(){}
void VWB_Bar(int,int,int,int,std::uint8_t){}void TP_PrintPageNumber(){}
void TP_HandleCodes(){} // Next page's drawing is tested by the terminal adapter suite.
namespace bstone{namespace n3ds_menu{
void document_page(int p,int n){assert(p>=0&&p<n);}
bool document(){return true;}bool scroll_document(int){return false;}
}}
void end_page(){ControlInfo ci;switch(0){
''' + page.replace("case TP_CNVT_CODE('E', 'P'):", 'case 0:') + r'''
}}
void load(PresenterInfo&info,std::vector<char>&script){
 info={};info.fontnumber=4;info.xl=info.yl=8;info.xh=311;info.yh=175;
 info.script[0]=script.data();info.scriptstart=script.data();
 TP_InitScript(&info);TP_Presenter(&info);
}
int main(int argc,char**argv){
 if(argc>1&&std::string(argv[1])=="font"){
  // Cached copies in n3ds_intro do not populate this resource slot.
  PresenterInfo info{};char script[]="^ST0^CECLASSIC CONTROLS\r^EP^XX";
  info.fontnumber=4;info.script[0]=script;TP_Presenter(&info);
  volatile int width=font->width['C'];(void)width;return 0;
 }
 for(auto scheme:{InControlScheme::classic,InControlScheme::modern,InControlScheme::modern_cstick}){
  const auto text=guide(scheme);
  assert(text.find(in_control_scheme_name(scheme))!=std::string::npos);
  assert((text.find("C-STICK: TURN")!=std::string::npos)); // C-Stick turns in every layout.
  assert(text.find("ZL / ZR: NEXT / PREVIOUS WEAPON")!=std::string::npos);
  for(int repeat=0;repeat<100;++repeat){
   // Exercise both alignments of the byte stream, as allocated guide strings do.
   std::vector<char> storage(text.size()+2);std::memcpy(storage.data()+repeat%2,text.c_str(),text.size()+1);
   PresenterInfo info{};info.script[0]=storage.data()+repeat%2;TP_InitScript(&info);
   assert(info.numpages==2);info.fontnumber=4;info.xl=info.yl=8;info.xh=311;info.yh=175;
   grsegs[4].clear();TP_Presenter(&info);assert(font&&font->width['C']==5);
   pi=&info;flags=fl_presenting|fl_dnreleased|fl_upreleased;
   input={{dir_East}};input_at=0;end_page();assert(info.pagenum==1);
   flags|=fl_upreleased;input={{dir_West}};input_at=0;end_page();assert(info.pagenum==0);
   input={{dir_None,false,true}};input_at=0;end_page();assert(EscPressed&&!(flags&fl_presenting));
  }
 }
 std::puts("PASS: production guide data, both parser alignments, font startup and page next/previous/B exit, 300 reopen cycles; GPU/HID boundaries mocked");
}
'''
with tempfile.TemporaryDirectory(prefix='bstone-button-guide-') as directory:
    p = Path(directory)
    (p / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++14', '-fsanitize=address,undefined',
                    '-fno-sanitize-recover=all', str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    if baseline:
        for args, diagnostic in [([], 'misaligned address'), (['font'], 'null pointer')]:
            result = subprocess.run([str(p / 'test'), *args], capture_output=True, text=True)
            assert result.returncode != 0 and diagnostic in result.stderr, result.stderr
            print('REPRODUCED release fault:', diagnostic)
    else:
        subprocess.run([str(p / 'test')], check=True)
