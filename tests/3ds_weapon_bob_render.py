#!/usr/bin/env python3
"""Render the actual production scaler into two synthetic eye buffers."""
import pathlib
import runpy

ROOT = pathlib.Path(__file__).resolve().parents[1]
harness = runpy.run_path(str(ROOT / 'tests/3ds_native_options.py'))
source = (ROOT / 'src/bstone/src/3d_scale.cpp').read_text()
scaler = harness['function'](source, 'void generic_scale_shape(')
harness['run'](r'''
#define __3DS__ 1
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "bstone_3ds_weapons.h"
namespace bstone { namespace n3ds_weapons {
const Frame* get_frame(int, bool) noexcept { return nullptr; }
bool available() noexcept { return false; }
} }
bool vid_cfg_ai_weapons() { return false; }
void vid_cfg_ai_weapons(bool) {}
namespace bstone { struct Sprite { static constexpr int dimension=64; }; }
enum class ShapeDrawMode { simple, shaded, player_weapon };
bool useBounceOffset=false, n3ds_stereo_active=true, cloaked_shape=false;
double bounceOffset=6;
int vga_width=400,vga_height=240,vga_ref_width=320,vga_ref_height=200;
int viewwidth=400,vga_area=400*240,bufferofs=0;
int vga_3d_view_top_y=0,vga_3d_view_bottom_y=240,centery=120;
int maxscaleshl2=512,shade_max=0,normalshade=1;
std::array<double,400> wallheight{};
std::uint8_t light_data[8192]{};
const std::uint8_t* lightsource=light_data;
std::vector<std::uint8_t> pixels(2*400*240);
std::uint8_t* vga_memory=pixels.data();
int vl_get_offset(int base,int x,int y) { return base+y*vga_width+x; }
struct Sprite {
    int left=16,top=16,width=32,height=48;
    std::vector<std::int16_t> data;
    Sprite() { reset(false); }
    void reset(bool holes) {
        data.assign(width*height,7);
        if(holes) for(int x=0;x<width;++x) for(int y=0;y<height;++y)
            if((x+y)%7==0) data[x*height+y]=-1;
    }
    int get_left() const { return left; }
    int get_top() const { return top; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    const std::int16_t* get_column(int index) const {
        assert(index>=0 && index<width); return data.data()+index*height;
    }
} sprite;
struct Cache { Sprite* cache(int) { return &sprite; } } vid_sprite_cache;
''' + scaler + r'''
void render(int ref_height, bool weapon=true) {
    pixels.assign(2*vga_area,2); std::fill_n(pixels.data(),vga_area,1);
    vga_memory=pixels.data();
    generic_scale_shape(viewwidth/2,1,ref_height,0,
        weapon?ShapeDrawMode::player_weapon:ShapeDrawMode::simple);
}
void parity() {
    for(int i=0;i<vga_area;++i) {
        if(pixels[i]==1) assert(pixels[i+vga_area]==2);
        else assert(pixels[i]==7 && pixels[i+vga_area]==pixels[i]);
    }
}
int leftmost() {
    for(int x=0;x<viewwidth;++x) for(int y=0;y<vga_height;++y)
        if(pixels[y*vga_width+x]!=1) return x;
    assert(false); return 0;
}
int main() {
    for(int width : {320,400}) for(int size : {88,128}) {
        vga_width=viewwidth=width; vga_area=width*240;
        sprite.reset(false); useBounceOffset=false; render(size);
        const auto stable=pixels; const int rest_x=leftmost();
        for(bool holes : {false,true}) {
            sprite.reset(holes);
            useBounceOffset=false; render(size); const auto disabled=pixels;
            for(int step=0;step<=96;++step) {
                bounceOffset=step/8.0; useBounceOffset=false; render(size);
                assert(pixels==disabled);
                useBounceOffset=true; render(size); parity();
                assert(std::abs(leftmost()-rest_x)<=2);
                // Full-bottom source artwork must stay clipped at the bottom,
                // even with top/height rounding, throughout every bob phase.
                bool reaches_bottom=false;
                for(int x=0;x<viewwidth;++x)
                    reaches_bottom |= pixels[(vga_height-1)*vga_width+x]!=1;
                assert(reaches_bottom);
            }
            bounceOffset=6; useBounceOffset=true; render(size); assert(pixels==disabled);
        }
        useBounceOffset=false; render(200,false); const auto world=pixels;
        useBounceOffset=true; bounceOffset=0; render(200,false); assert(pixels==world);
    }
    // Trimming above the source bottom is an existing resource limitation:
    // movement must never expose MORE of that edge than its resting anchor.
    sprite.height=44; sprite.reset(false);
    for(int size : {88,128}) {
        useBounceOffset=false; render(size);
        int rest_bottom=0;
        for(int y=0;y<240;++y) for(int x=0;x<viewwidth;++x)
            if(pixels[y*vga_width+x]!=1) rest_bottom=std::max(rest_bottom,y);
        for(int step=0;step<=96;++step) {
            bounceOffset=step/8.0; useBounceOffset=true; render(size);
            int moving_bottom=0;
            for(int y=0;y<240;++y) for(int x=0;x<viewwidth;++x)
                if(pixels[y*vga_width+x]!=1) moving_bottom=std::max(moving_bottom,y);
            assert(moving_bottom>=rest_bottom); parity();
        }
    }
    std::puts("PASS: actual scaler, both weapon sizes/aspects, down-only bob, bottom anchoring, horizontal bounds, disabled/rest stability, transparency, eye parity and unchanged world sprites");
}
''', extra_flags=['-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                 '-I' + str(ROOT / 'src/bstone/src')])
