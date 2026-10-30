#ifndef BSTONE_3DS_INTRO_INCLUDED
#define BSTONE_3DS_INTRO_INCLUDED

#if defined(__3DS__) || defined(_3DS)

#include "gfxv.h"
#include "id_ca.h"
#include "id_vh.h"
#include <citro2d.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bstone {
namespace n3ds_intro {

enum class Stage
{
    none,
    piracy,
    apogee,
    jam,
    pg13,
    title
};

struct IntroFonts
{
    std::vector<std::uint8_t> title;
    std::vector<std::uint8_t> menu;
};

Stage& current_stage();

IntroFonts& fonts();

void set_stage(Stage stage);

int font_text_width(
    const std::vector<std::uint8_t>& data,
    const char* text);

void draw_font(
    int x,
    int y,
    const char* text,
    const std::vector<std::uint8_t>& data,
    int scale,
    u32 color);

void draw_centered(
    const char* text,
    int y,
    bool is_title,
    int scale,
    u32 color);

void draw();

} // namespace n3ds_intro
} // namespace bstone

#endif // 3DS
#endif // BSTONE_3DS_INTRO_INCLUDED
