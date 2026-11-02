#!/usr/bin/env python3
"""Run production SDL poll filtering and native configuration finalization."""
from pathlib import Path
import subprocess
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


poll = function((SRC / 'bstone_sys_event_mgr_sdl2.cpp').read_text(), 'bool Sdl2EventMgr::do_poll_event(')
config = function((SRC / '3d_main.cpp').read_text(), 'void ReadConfig()')
code = r'''
#define __3DS__ 1
#include <cassert>
#include <cstdio>
#include <vector>
#define BSTONE_ASSERT(x) assert(x)
enum {SDL_CONTROLLERBUTTONDOWN,SDL_CONTROLLERBUTTONUP,SDL_CONTROLLERAXISMOTION,
 SDL_JOYBUTTONDOWN,SDL_JOYBUTTONUP,SDL_JOYAXISMOTION,SDL_JOYHATMOTION,SDL_KEYDOWN,SDL_QUIT};
enum class EventType {none,keyboard,quit};
struct Common{EventType type=EventType::none;int timestamp=0;};
struct Event{Common common;};
struct SDL_Event{int type;struct{int timestamp=123;}common;};
std::vector<SDL_Event> queue;unsigned at=0,keys=0;int scans=0,quits=0,handled=0;
constexpr unsigned KEY_START=1,KEY_SELECT=2;
void hidScanInput(){++scans;}unsigned hidKeysHeld(){return keys;}void Quit(){++quits;}
bool SDL_PollEvent(SDL_Event*e){if(at==queue.size())return false;*e=queue[at++];return true;}
struct Logger{void log_information(const char*){}};
struct Sdl2EventMgr{bool is_initialized_=true;Logger logger_;
 bool do_poll_event(Event&);
 bool handle_event(SDL_Event&s,Event&e){++handled;e.common.type=s.type==SDL_QUIT?EventType::quit:EventType::keyboard;return true;}
};
''' + poll + r'''
// Simulate a loaded desktop configuration at the IO boundary. The production
// ReadConfig wrapper must remove its action reinterpretation for each edition.
int scheme,profile;bool legacy=false,defaults=false;int stored[2]={1,2};
void read_text_config(){scheme=stored[profile];legacy=true;defaults=false;}
void in_clear_bindings(){legacy=false;}
void in_set_default_bindings(){assert(!legacy);defaults=true;}
int in_get_control_scheme(){return scheme>=0&&scheme<3?scheme:0;}
void in_set_control_scheme(int v){scheme=v;}
''' + config + r'''
int main(){
 Sdl2EventMgr mgr;Event e;
 for(int repeats:{1,20,300}){
  queue.clear();at=0;handled=0;
  for(int i=0;i<repeats;++i)for(int type=0;type<=SDL_JOYHATMOTION;++type)queue.push_back({type});
  assert(!mgr.do_poll_event(e)&&handled==0&&e.common.type==EventType::none);
  queue.push_back({SDL_KEYDOWN});queue.push_back({SDL_QUIT});
  assert(mgr.do_poll_event(e)&&e.common.type==EventType::keyboard&&e.common.timestamp==123);
  assert(mgr.do_poll_event(e)&&e.common.type==EventType::quit);assert(!mgr.do_poll_event(e));
 }
 keys=KEY_START|KEY_SELECT;mgr.do_poll_event(e);assert(quits==1);
 for(profile=0;profile<2;++profile){
  ReadConfig();assert(scheme==stored[profile]&&defaults&&!legacy);
  stored[profile]=99;ReadConfig();assert(scheme==0&&defaults&&!legacy);
 }
 std::puts("PASS: production SDL controller filtering, retained keyboard/quit events, emergency chord and ReadConfig finalization for two profile fixtures");
}
'''
with tempfile.TemporaryDirectory(prefix='bstone-input-pipeline-') as directory:
    p = Path(directory)
    (p / 'test.cpp').write_text(code)
    subprocess.run(['c++', '-std=c++14', '-fsanitize=address,undefined',
                    str(p / 'test.cpp'), '-o', str(p / 'test')], check=True)
    subprocess.run([str(p / 'test')], check=True)
