#!/usr/bin/env python3
"""Run extracted production Classic input and weapon-bounce code on the host.
Mocks replace platform/game state, not the algorithms under test.
"""
import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1] / 'src/bstone/src'


def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def run(code, extra_flags=()):
    with tempfile.TemporaryDirectory(prefix='bstone-native-options-') as directory:
        path = pathlib.Path(directory)
        (path / 'test.cpp').write_text(code)
        executable = path / 'test'
        subprocess.run(['c++', '-std=c++14', *extra_flags, str(path / 'test.cpp'), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)


def test_classic():
    source = (ROOT / 'id_in.cpp').read_text()
    header = (ROOT / 'id_in.h').read_text()
    scheme = header[header.index('enum class InControlScheme'):]
    scheme = scheme[:scheme.index(';') + 1]
    layout = source[source.index('struct N3dsButtonBinding'):source.index('} // namespace', source.index('struct N3dsButtonBinding'))]
    start = source.index('        // Menus always retain')
    mapping = source[start:source.index('\n}\n', start)]
    run(r'''
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
using u32=std::uint32_t;
enum class ScanCode { sc_none, sc_w, sc_s, sc_left_arrow, sc_right_arrow,
    sc_control, sc_space, sc_a, sc_d, sc_e, sc_q, sc_tab, sc_escape };
constexpr u32 KEY_UP=1u<<0, KEY_DOWN=1u<<1, KEY_LEFT=1u<<2, KEY_RIGHT=1u<<3,
    KEY_A=1u<<4, KEY_B=1u<<5, KEY_L=1u<<6, KEY_R=1u<<7, KEY_ZL=1u<<8,
    KEY_ZR=1u<<9, KEY_X=1u<<10, KEY_START=1u<<11, KEY_SELECT=1u<<12,
    KEY_Y=1u<<13,KEY_DRIGHT=KEY_RIGHT,KEY_DLEFT=KEY_LEFT;
bool vid_is_hud=true;
namespace bstone { namespace n3ds_menu { bool active(){return false;} } }
struct Keys { std::array<bool,13> values{};
    bool& operator[](ScanCode key) { return values[static_cast<int>(key)]; }
} Keyboard;
ScanCode LastScan=ScanCode::sc_none;
''' + scheme + r'''
InControlScheme in_get_control_scheme() { return InControlScheme::classic; }
''' + layout + r'''
u32 previous=0;
void map(u32 keys) { const auto pressed=keys & ~previous; previous=keys;
''' + mapping + r'''
}
int main() {
    const ScanCode expected[]={ScanCode::sc_w,ScanCode::sc_s,ScanCode::sc_left_arrow,
        ScanCode::sc_right_arrow,ScanCode::sc_control,ScanCode::sc_space,ScanCode::sc_a,
        ScanCode::sc_d,ScanCode::sc_e,ScanCode::sc_q,ScanCode::sc_tab};
    for(u32 mask=0;mask<2048;++mask) {
        previous=0; Keyboard.values.fill(false); LastScan=ScanCode::sc_none;
        map(mask);
        for(int i=0;i<11;++i) assert(Keyboard[expected[i]]==bool(mask & (1u<<i)));
    }
    previous=0; Keyboard.values.fill(false);
    map(KEY_B); assert(LastScan==ScanCode::sc_space);
    LastScan=ScanCode::sc_none; map(KEY_B); assert(LastScan==ScanCode::sc_none);
    map(0); assert(!Keyboard[ScanCode::sc_space]);
    map(KEY_ZL); assert(Keyboard[ScanCode::sc_e]);
    Keyboard[ScanCode::sc_e]=false; map(KEY_ZL); assert(!Keyboard[ScanCode::sc_e]);
    map(0); map(KEY_ZL); assert(Keyboard[ScanCode::sc_e]);
    map(0); map(KEY_ZR); assert(Keyboard[ScanCode::sc_q]);
    Keyboard[ScanCode::sc_q]=false; map(KEY_ZR); assert(!Keyboard[ScanCode::sc_q]);
    map(0); map(KEY_X); assert(Keyboard[ScanCode::sc_tab] && LastScan==ScanCode::sc_tab);
    map(KEY_X); assert(!Keyboard[ScanCode::sc_tab]);
    map(0); map(KEY_START); assert(Keyboard[ScanCode::sc_escape]);
    map(KEY_START); assert(Keyboard[ScanCode::sc_escape]);
    map(0); assert(!Keyboard[ScanCode::sc_escape]);
    map(KEY_START|KEY_SELECT); assert(!Keyboard[ScanCode::sc_escape]);
    std::puts("PASS: Classic mapping for 2048 button combinations, held/edge/consumed keys, START+SELECT");
}
''')


def test_weapon():
    source = (ROOT / '3d_agent.cpp').read_text()
    draw = (ROOT / '3d_draw.cpp').read_text()
    scale = (ROOT / '3d_scale.cpp').read_text()
    constants = source[source.index('constexpr auto wb_MaxPoint'):source.index('namespace', source.index('void HandleWeaponBounce()\n{'))]
    equation = scale[scale.index('    constexpr auto mid_bob'):scale.index('\tconst auto sprite_ptr', scale.index('    constexpr auto mid_bob'))]
    run(r'''
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
double bounceOffset=0;
double sintable[360]{};
int viewsize=20;
''' + constants + r'''
bool useBounceOffset=false;
bool no_bob=false, aog=true;
int playstate=0;
constexpr int ex_victorious=1;
struct Game { int weapon=0,weaponframe=0; } gamestate;
int weaponscale[]={10};
struct Assets { bool is_aog() const { return aog; } } assets;
const Assets& get_assets_info() { return assets; }
bool gp_no_weapon_bobbing() { return no_bob; }
int offset(bool is_player_weapon) {
    const int vga_height=240, vga_ref_height=200, vga_width=400, vga_ref_width=320;
''' + equation + r'''
    return bob_offset;
}
int rendered=0, displacement=0;
void scale_player_weapon(int shape,int height) {
    assert(shape==10 && height==(aog?128:88));
    ++rendered; displacement=offset(true);
}
''' + function(draw, 'void DrawPlayerWeapon()\n{') + r'''
int main() {
    constexpr double pi=3.141592653589793;
    for(int i=0;i<360;++i) sintable[i]=std::sin(i*pi/180);
    for(bool edition : {true,false}) {
        aog=edition; no_bob=false; InitWeaponBounce();
        DrawPlayerWeapon(); assert(displacement==0 && !useBounceOffset);
        bool moved=false; int minimum=0, maximum=0;
        for(int i=0;i<200;++i) {
            bounceOk=8; HandleWeaponBounce(); DrawPlayerWeapon();
            assert(!useBounceOffset && bounceOffset>=0 && bounceOffset<=12);
            assert(std::abs(displacement)<=8);
            minimum=std::min(minimum,displacement); maximum=std::max(maximum,displacement);
            moved |= displacement!=0;
            no_bob=true; DrawPlayerWeapon(); assert(displacement==0);
            no_bob=false;
        }
        assert(moved && minimum==0 && maximum>0 && maximum<=4);
        bounceOk=0;
        for(int i=0;i<20;++i) HandleWeaponBounce();
        DrawPlayerWeapon(); assert(displacement==0);
        useBounceOffset=true; assert(offset(false)==0); useBounceOffset=false;
    }
    std::puts("PASS: production bounce simulation, both editions, switch ON/OFF, neutral rest and non-weapon exclusion");
}
''')


def test_aspect():
    source = (ROOT / 'bstone_sw_video.cpp').read_text()
    start = source.index('const float display_width =')
    end = source.index(');', source.index('C2D_DrawImageAt(', start)) + 2
    presentation = source[start:end]
    run(r'''
#include <cassert>
#include <cstdio>
#include <initializer_list>
struct Image { int eye; } eye_image{0};
bool wide=true;
bool vid_cfg_is_widescreen() { return wide; }
int vga_width=400, vga_height=240;
float image_x=0, image_width=0;
void C2D_DrawImageAt(const Image&,float x,float y,float depth,void*,float sx,float sy) {
    image_x=x; image_width=vga_width*sx;
    assert(y==0 && depth==0.5f && sy==1.0f);
}
void present() {
''' + presentation + r'''
}
int main() {
    for(bool aspect : {true,false}) {
        wide=aspect; vga_width=wide?400:320;
        for(int eye=0;eye<2;++eye) {
            eye_image.eye=eye; present();
            assert(image_x==(wide?0:40) && image_width==(wide?400:320));
            assert(image_x+image_width==(wide?400:360));
        }
    }
    std::puts("PASS: production presentation uses native 5:3 or centered 4:3 identically for both eyes");
}
''')


if __name__ == '__main__':
    test_classic()
    test_weapon()
    test_aspect()
