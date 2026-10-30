#include "bstone_3ds_hud.h"

#include "3d_def.h"
#include "bstone_3ds_automap.h"
#include "bstone_3ds_messages.h"
#include "bstone_3ds_intro.h"
#include <citro2d.h>
#include <cstdio>
#include <string>
#include <vector>

namespace bstone {
namespace n3ds_hud {

// ---------------------------------------------
// Green phosphor CRT palette.
// ---------------------------------------------

void draw_frame_screw(int x, int y)
{
    const u32 outer =
        C2D_Color32(140, 146, 152, 255);

    const u32 inner =
        C2D_Color32(92, 98, 104, 255);

    const u32 slot =
        C2D_Color32(42, 45, 49, 255);

    C2D_DrawRectSolid(
        static_cast<float>(x),
        static_cast<float>(y),
        0.24f,
        4, 4, outer);

    C2D_DrawRectSolid(
        static_cast<float>(x + 1),
        static_cast<float>(y + 1),
        0.25f,
        2, 2, inner);

    // Ranura del tornillo.
    C2D_DrawRectSolid(
        static_cast<float>(x + 1),
        static_cast<float>(y + 2),
        0.26f,
        2, 1, slot);
}

void draw_watch_frame()
{
    const u32 metal_shadow =
        C2D_Color32(24, 26, 29, 255);

    const u32 metal_dark =
        C2D_Color32(42, 46, 50, 255);

    const u32 metal_mid =
        C2D_Color32(76, 82, 88, 255);

    const u32 metal_light =
        C2D_Color32(122, 130, 137, 255);

    const u32 wear_light =
        C2D_Color32(162, 170, 176, 170);

    const u32 wear_dark =
        C2D_Color32(34, 36, 40, 190);

    // -----------------------------------------
    // Bisel exterior.
    // -----------------------------------------

    C2D_DrawRectSolid(
        0, 0, 0.15f,
        320, 240, metal_shadow);

    C2D_DrawRectSolid(
        0, 0, 0.16f,
        320, 2, metal_light);

    C2D_DrawRectSolid(
        0, 0, 0.16f,
        2, 240, metal_light);

    C2D_DrawRectSolid(
        318, 0, 0.16f,
        2, 240, metal_dark);

    C2D_DrawRectSolid(
        0, 238, 0.16f,
        320, 2, metal_dark);

    C2D_DrawRectSolid(
        2, 2, 0.17f,
        316, 236, metal_mid);

    C2D_DrawRectSolid(
        3, 3, 0.18f,
        314, 234, metal_dark);

    // -----------------------------------------
    // Labio interior del "monitor".
    // -----------------------------------------

    C2D_DrawRectSolid(
        4, 4, 0.19f,
        312, 1, metal_light);

    C2D_DrawRectSolid(
        4, 4, 0.19f,
        1, 232, metal_light);

    C2D_DrawRectSolid(
        315, 4, 0.19f,
        1, 232, metal_shadow);

    C2D_DrawRectSolid(
        4, 235, 0.19f,
        312, 1, metal_shadow);

    // -----------------------------------------
    // Refuerzos laterales tipo carcasa.
    // Dan sensación de "dispositivo".
    // -----------------------------------------

    C2D_DrawRectSolid(
        0, 102, 0.20f,
        4, 36, metal_mid);

    C2D_DrawRectSolid(
        316, 102, 0.20f,
        4, 36, metal_mid);

    C2D_DrawRectSolid(
        1, 108, 0.21f,
        2, 24, metal_light);

    C2D_DrawRectSolid(
        317, 108, 0.21f,
        2, 24, metal_shadow);

    // Pequeñas placas superior e inferior.
    C2D_DrawRectSolid(
        144, 0, 0.20f,
        32, 4, metal_mid);

    C2D_DrawRectSolid(
        146, 1, 0.21f,
        28, 1, metal_light);

    C2D_DrawRectSolid(
        144, 236, 0.20f,
        32, 4, metal_mid);

    C2D_DrawRectSolid(
        146, 238, 0.21f,
        28, 1, metal_shadow);

    // -----------------------------------------
    // Tornillos / anclajes.
    // -----------------------------------------

    draw_frame_screw(6, 6);
    draw_frame_screw(310, 6);
    draw_frame_screw(6, 230);
    draw_frame_screw(310, 230);

    // -----------------------------------------
    // Desgaste / arañazos sutiles.
    // Todo muy leve para no ensuciar demasiado.
    // -----------------------------------------

    // Arañazos superiores.
    C2D_DrawRectSolid(
        34, 1, 0.22f,
        6, 1, wear_light);

    C2D_DrawRectSolid(
        35, 2, 0.22f,
        3, 1, wear_light);

    C2D_DrawRectSolid(
        96, 2, 0.22f,
        5, 1, wear_dark);

    C2D_DrawRectSolid(
        250, 1, 0.22f,
        7, 1, wear_light);

    // Lado izquierdo.
    C2D_DrawRectSolid(
        1, 52, 0.22f,
        1, 5, wear_dark);

    C2D_DrawRectSolid(
        2, 53, 0.22f,
        1, 2, wear_dark);

    C2D_DrawRectSolid(
        1, 184, 0.22f,
        1, 7, wear_light);

    // Lado derecho.
    C2D_DrawRectSolid(
        318, 74, 0.22f,
        1, 6, wear_light);

    C2D_DrawRectSolid(
        317, 76, 0.22f,
        1, 3, wear_light);

    C2D_DrawRectSolid(
        318, 194, 0.22f,
        1, 5, wear_dark);

    // Parte inferior.
    C2D_DrawRectSolid(
        48, 238, 0.22f,
        8, 1, wear_dark);

    C2D_DrawRectSolid(
        49, 237, 0.22f,
        4, 1, wear_dark);

    C2D_DrawRectSolid(
        266, 238, 0.22f,
        6, 1, wear_light);

    // Pequeños píxeles "machacados" en esquinas.
    C2D_DrawRectSolid(
        0, 0, 0.23f,
        2, 2, metal_shadow);

    C2D_DrawRectSolid(
        318, 0, 0.23f,
        2, 2, metal_dark);

    C2D_DrawRectSolid(
        0, 238, 0.23f,
        2, 2, metal_dark);

    C2D_DrawRectSolid(
        318, 238, 0.23f,
        2, 2, metal_shadow);
}

void draw_screen_background()
{
    // Superficie única del monitor de fósforo verde.
    C2D_DrawRectSolid(
        4, 4, 0.28f,
        312, 232,
        C2D_Color32(2, 9, 5, 255));

    // Cuadrícula global, presente en toda la pantalla.
    const u32 grid =
        C2D_Color32(10, 33, 18, 255);

    for (int x = 12; x < 312; x += 20)
    {
        C2D_DrawRectSolid(
            static_cast<float>(x),
            6, 0.29f,
            1, 228, grid);
    }

    for (int y = 12; y < 236; y += 20)
    {
        C2D_DrawRectSolid(
            6, static_cast<float>(y),
            0.29f,
            308, 1, grid);
    }
}

u32 crt_background()
{
    return C2D_Color32(2, 9, 5, 255);
}

u32 crt_panel()
{
    return C2D_Color32(5, 20, 10, 255);
}

u32 crt_border()
{
    return C2D_Color32(25, 67, 36, 255);
}

u32 crt_dim()
{
    return C2D_Color32(76, 153, 93, 255);
}

u32 crt_green()
{
    return C2D_Color32(88, 235, 121, 255);
}

u32 crt_bright()
{
    return C2D_Color32(168, 255, 179, 255);
}

// ---------------------------------------------
// Genuine BStone menu font.
// ---------------------------------------------

const std::vector<std::uint8_t>& hud_font()
{
    auto& data = n3ds_intro::fonts().menu;

    // Normally copied during the presentations.
    // Also support starting without the intro.
    if (data.empty())
    {
        const int index = STARTFONT + 4;

        if (index >= 0 &&
            index < static_cast<int>(grsegs.size()))
        {
            const auto& source = grsegs[index];

            if (source.size() >= sizeof(fontstruct))
            {
                data.assign(
                    source.begin(),
                    source.end());
            }
        }
    }

    return data;
}

int text_width(const char* value)
{
    return n3ds_intro::font_text_width(
        hud_font(), value);
}

void draw_text(
    int x,
    int y,
    const char* value,
    u32 color,
    int scale)
{
    const auto& data = hud_font();

    if (data.empty())
    {
        return;
    }

    n3ds_intro::draw_font(
        x + 1,
        y + 1,
        value,
        data,
        scale,
        C2D_Color32(22, 71, 34, 255));

    n3ds_intro::draw_font(
        x,
        y,
        value,
        data,
        scale,
        color);
}

// Draw original font with strict clipping.
// Used by the scrolling R.E.B.A. terminal.
void draw_clipped_text(
    int x,
    int y,
    const char* value,
    u32 color,
    int top,
    int bottom,
    int right)
{
    const auto& data = hud_font();

    if (data.size() < sizeof(fontstruct))
    {
        return;
    }

    const auto* font =
        reinterpret_cast<const fontstruct*>(
            data.data());

    const int height = font->height;

    if (height <= 0 || height > 64)
    {
        return;
    }

    for (const auto* p =
             reinterpret_cast<const unsigned char*>(value);
         *p; ++p)
    {
        unsigned char ch = *p;

        if (ch >= 'a' && ch <= 'z')
        {
            ch = static_cast<unsigned char>(ch - 32);
        }

        const int width =
            static_cast<unsigned char>(font->width[ch]);

        const int offset = font->location[ch];

        if (width > 0 && width <= 64 &&
            offset >= 0 &&
            static_cast<std::size_t>(offset) +
                static_cast<std::size_t>(width * height)
                <= data.size())
        {
            const auto* glyph =
                data.data() + offset;

            for (int row = 0; row < height; ++row)
            {
                const int py = y + row;

                if (py < top || py >= bottom)
                {
                    continue;
                }

                int column = 0;

                while (column < width)
                {
                    if (glyph[row * width + column] == 0)
                    {
                        ++column;
                        continue;
                    }

                    const int begin = column;

                    while (column < width &&
                           glyph[row * width + column] != 0)
                    {
                        ++column;
                    }

                    const int px = x + begin;

                    if (px >= right)
                    {
                        break;
                    }

                    const int visible_width =
                        px + column - begin > right ?
                        right - px :
                        column - begin;

                    if (visible_width > 0)
                    {
                        C2D_DrawRectSolid(
                            static_cast<float>(px),
                            static_cast<float>(py),
                            0.8f,
                            static_cast<float>(visible_width),
                            1,
                            color);
                    }
                }
            }
        }

        x += width;

        if (x >= right)
        {
            break;
        }
    }
}

// ---------------------------------------------
// ECG / health monitor.
// ---------------------------------------------


// --------------------------------------------------
// Thin phosphor-green outlines on a single CRT surface.
// No metallic borders or separate panel backgrounds.
// --------------------------------------------------

void draw_monitor_outline(
    int x, int y, int width, int height)
{
    const u32 edge =
        C2D_Color32(42, 110, 57, 255);

    const u32 accent =
        C2D_Color32(76, 174, 96, 255);

    C2D_DrawRectSolid(
        x, y, 0.35f, width, 1, edge);

    C2D_DrawRectSolid(
        x, y + height - 1, 0.35f,
        width, 1, edge);

    C2D_DrawRectSolid(
        x, y, 0.35f, 1, height, edge);

    C2D_DrawRectSolid(
        x + width - 1, y, 0.35f,
        1, height, edge);

    // Small green phosphor accent.
    C2D_DrawRectSolid(
        x + 3, y, 0.36f,
        7, 1, accent);
}

void draw_ecg()
{
    const int health =
        gamestate.health < 0 ? 0 :
        gamestate.health > 100 ? 100 :
        gamestate.health;

    const u32 signal =
        health > 60 ? crt_green() :
        health > 25 ? C2D_Color32(125, 218, 115, 255) :
                      crt_bright();

    // Pequeño acento superior.
    C2D_DrawRectSolid(
        8, 8, 0.6f,
        28, 1, signal);

    // Separador entre gráfica y lectura.
    C2D_DrawRectSolid(
        140, 12, 0.6f,
        1, 34, crt_border());

    // Onda PQRST animada.
    const int bpm =
        70 + ((100 - health) * 17 / 10);

    const int beat_period_ms = 60000 / bpm;

    static constexpr int phases[] =
    {
        0, 100, 140, 170, 200, 224,
        245, 260, 277, 300, 355,
        400, 480, 560, 640, 1000
    };

    static constexpr int amplitudes[] =
    {
        0, 0, 2, 4, 2, 0,
        -3, 18, -10, 0, 0,
        2, 7, 3, 0, 0
    };

    const auto elapsed = osGetTime();

    int previous_y = 29;

    for (int x = 14; x < 134; ++x)
    {
        const auto sample_time =
            elapsed + static_cast<u64>((x - 14) * 8);

        const int phase = static_cast<int>(
            ((sample_time % beat_period_ms) * 1000) /
            beat_period_ms);

        const int beat = static_cast<int>(
            (sample_time / beat_period_ms) % 3);

        int wave = 0;

        if (health > 0)
        {
            for (int i = 0; i < 15; ++i)
            {
                if (phase < phases[i + 1])
                {
                    const int duration =
                        phases[i + 1] - phases[i];

                    const int position =
                        phase - phases[i];

                    wave = amplitudes[i] +
                        ((amplitudes[i + 1] - amplitudes[i]) *
                        position / duration);

                    break;
                }
            }

            if (health > 25)
            {
                if (beat == 0)
                    wave = wave * 95 / 100;
                else if (beat == 2)
                    wave = wave * 105 / 100;
            }
            else
            {
                if (beat == 0)
                    wave = wave * 110 / 100;
                else if (beat == 1)
                    wave = wave * 85 / 100;
            }
        }

        const int raw_y = 29 - wave;

        const int current_y =
            raw_y < 16 ? 16 :
            raw_y > 42 ? 42 :
            raw_y;

        if (x == 14)
        {
            previous_y = current_y;
        }

        const int top =
            previous_y < current_y ?
            previous_y : current_y;

        const int bottom =
            previous_y > current_y ?
            previous_y : current_y;

        C2D_DrawRectSolid(
            static_cast<float>(x),
            static_cast<float>(top),
            0.7f,
            1,
            static_cast<float>(bottom - top + 1),
            signal);

        previous_y = current_y;
    }

    C2D_DrawRectSolid(
        130,
        static_cast<float>(previous_y - 1),
        0.8f,
        3, 3, signal);

    draw_text(
        146, 13,
        "HEALTH",
        crt_dim());

    char number[16]{};

    std::snprintf(
        number, sizeof(number),
        "%d%%", health);

    const int width = text_width(number);

    draw_text(
        146 + (46 - width) / 2,
        32,
        number,
        signal);
}

// ---------------------------------------------
// R.E.B.A. information terminal.
// Original BStone font + vertical scrolling.
// ---------------------------------------------

void draw_info()
{
    const auto& state = message_state();

    C2D_DrawRectSolid(
        208, 8, 0.6f,
        24, 1, crt_green());

    draw_text(
        210, 11,
        "R.E.B.A.",
        crt_bright());

    C2D_DrawRectSolid(
        210, 24, 0.6f,
        96, 1, crt_border());

    C2D_DrawRectSolid(
        303, 12, 0.7f,
        4, 4, crt_green());

    constexpr int text_top = 28;
    constexpr int text_bottom = 52;

    if (state.lines.empty())
    {
        draw_clipped_text(
            210, 30,
            "STANDBY",
            crt_dim(),
            text_top,
            text_bottom,
            305);

        return;
    }

    // Recalcular líneas según el ancho real.
    static u64 cached_time = static_cast<u64>(-1);
    static std::vector<std::string> wrapped_lines;

    if (cached_time != state.started_ms)
    {
        cached_time = state.started_ms;
        wrapped_lines.clear();

        for (const auto& source : state.lines)
        {
            std::string line;

            for (const char ch : source)
            {
                std::string candidate = line;
                candidate += ch;

                if (!line.empty() &&
                    text_width(candidate.c_str()) > 94)
                {
                    wrapped_lines.push_back(line);
                    line.clear();
                }

                line += ch;
            }

            if (!line.empty())
            {
                wrapped_lines.push_back(line);
            }
        }
    }

    const auto& data = hud_font();

    if (data.size() < sizeof(fontstruct))
    {
        return;
    }

    const auto* font =
        reinterpret_cast<const fontstruct*>(
            data.data());

    const int font_height = font->height;
    const int line_height = font_height + 2;

    if (font_height <= 0 ||
        font_height > 32 ||
        wrapped_lines.empty())
    {
        return;
    }

    const int visible_height =
        text_bottom - text_top;

    const int content_height =
        static_cast<int>(wrapped_lines.size() - 1) *
        line_height + font_height;

    const int max_scroll =
        content_height > visible_height ?
        content_height - visible_height : 0;

    int scroll = 0;

    if (max_scroll > 0)
    {
        constexpr u64 pause_top = 1000;
        constexpr u64 pause_bottom = 1200;
        constexpr u64 ms_per_pixel = 50;

        const u64 scrolling =
            static_cast<u64>(max_scroll) * ms_per_pixel;

        const u64 cycle =
            pause_top + scrolling + pause_bottom;

        const u64 elapsed =
            osGetTime() - state.started_ms;

        const u64 position = elapsed % cycle;

        if (position > pause_top)
        {
            const u64 moved =
                (position - pause_top) / ms_per_pixel;

            scroll =
                moved >= static_cast<u64>(max_scroll) ?
                max_scroll :
                static_cast<int>(moved);
        }

        // Scrollbar muy discreta.
        C2D_DrawRectSolid(
            308, text_top, 0.6f,
            1, visible_height,
            crt_border());

        int thumb_height =
            visible_height * visible_height /
            content_height;

        if (thumb_height < 4)
        {
            thumb_height = 4;
        }

        const int thumb_y =
            text_top +
            (visible_height - thumb_height) *
            scroll / max_scroll;

        C2D_DrawRectSolid(
            308,
            static_cast<float>(thumb_y),
            0.7f,
            1,
            static_cast<float>(thumb_height),
            crt_bright());
    }

    for (std::size_t i = 0;
         i < wrapped_lines.size();
         ++i)
    {
        const int y =
            text_top +
            static_cast<int>(i) * line_height -
            scroll;

        if (y + font_height <= text_top)
            continue;

        if (y >= text_bottom)
            break;

        draw_clipped_text(
            210,
            y,
            wrapped_lines[i].c_str(),
            crt_green(),
            text_top,
            text_bottom,
            305);
    }
}

// ---------------------------------------------
// Automap frame and bottom status panels.
// ---------------------------------------------

void draw_status()
{
    const u32 bright = crt_bright();
    const u32 dim = crt_dim();
    const u32 green = crt_green();

    char number[32]{};

    // -----------------------------------------
    // Score and tokens.
    // -----------------------------------------

    draw_text(10, 189, "SCORE", dim);
    draw_text(66, 189, "TOK", dim);

    std::snprintf(
        number, sizeof(number),
        "%d",
        static_cast<int>(gamestate.score));

    draw_text(10, 199, number, bright);

    std::snprintf(
        number, sizeof(number),
        "%u",
        static_cast<unsigned>(gamestate.tokens));

    draw_text(66, 199, number, bright);

    // -----------------------------------------
    // Floor and lives.
    // -----------------------------------------

    draw_text(10, 212, "FLOOR", dim);
    draw_text(62, 212, "LIVES", dim);

    std::snprintf(
        number, sizeof(number),
        "%d",
        static_cast<int>(gamestate.mapon) + 1);

    draw_text(10, 223, number, bright);

    std::snprintf(
        number, sizeof(number),
        "%d",
        gamestate.lives > 0 ?
            static_cast<int>(gamestate.lives) : 0);

    draw_text(62, 223, number, bright);

    // -----------------------------------------
    // Current weapon.
    // -----------------------------------------

    const char* weapon = "NONE";

    switch (gamestate.weapon)
    {
        case wp_autocharge:
            weapon = "AUTOCHARGE";
            break;

        case wp_pistol:
            weapon = "PISTOL";
            break;

        case wp_burst_rifle:
            weapon = "BURST RIFLE";
            break;

        case wp_ion_cannon:
            weapon = "ION CANNON";
            break;

        case wp_grenade:
            weapon = "GRENADE";
            break;

        case wp_bfg_cannon:
            weapon = "BFG CANNON";
            break;

        default:
            break;
    }

    draw_text(114, 189, "WEAPON", dim);
    draw_text(114, 199, weapon, bright);

    // -----------------------------------------
    // Ammunition.
    // -----------------------------------------

    C2D_DrawRectSolid(
        114, 225, 0.5f,
        88, 5,
        C2D_Color32(25, 67, 36, 255));

    if (gamestate.weapon == wp_autocharge)
    {
        const bool ready =
            gamestate.weapon_wait == 0;

        draw_text(
            114, 212,
            ready ? "READY" : "WAIT",
            ready ? green : dim);

        if (ready)
        {
            C2D_DrawRectSolid(
                114, 225, 0.6f,
                88, 5, green);
        }
    }
    else if (gamestate.weapon >= 0)
    {
        const int ammo =
            gamestate.ammo < 0 ? 0 :
            gamestate.ammo > 100 ? 100 :
            gamestate.ammo;

        draw_text(114, 212, "AMMO", dim);

        std::snprintf(
            number, sizeof(number),
            "%d%%", ammo);

        draw_text(165, 212, number, bright);

        if (ammo > 0)
        {
            C2D_DrawRectSolid(
                114, 225, 0.6f,
                static_cast<float>(88 * ammo) / 100,
                5, green);
        }
    }

    // -----------------------------------------
    // Access cards.
    // -----------------------------------------

    draw_text(222, 189, "ACCESS", dim);

    const int keys[] =
    {
        kt_red,
        kt_yellow,
        kt_green,
        kt_blue,
        kt_gold
    };

    const u32 key_colors[] =
    {
        C2D_Color32(228, 65, 65, 255),
        C2D_Color32(245, 214, 61, 255),
        C2D_Color32(65, 209, 104, 255),
        C2D_Color32(62, 145, 242, 255),
        C2D_Color32(235, 166, 65, 255)
    };

    for (int i = 0; i < 5; ++i)
    {
        const float x =
            static_cast<float>(222 + i * 18);

        C2D_DrawRectSolid(
            x, 199, 0.5f,
            15, 11, key_colors[i]);

        C2D_DrawRectSolid(
            x + 2, 201, 0.6f,
            11, 7,
            gamestate.numkeys[keys[i]] > 0 ?
                key_colors[i] :
                crt_background());
    }

    // -----------------------------------------
    // Radar.
    // -----------------------------------------

    const int radar =
        gamestate.rpower < 0 ? 0 :
        gamestate.rpower > MAX_RADAR_ENERGY ? 100 :
        gamestate.rpower * 100 / MAX_RADAR_ENERGY;

    draw_text(222, 212, "RADAR", dim);

    std::snprintf(
        number, sizeof(number),
        "%d%%", radar);

    draw_text(284, 212, number, bright);

    C2D_DrawRectSolid(
        222, 225, 0.5f,
        86, 5,
        C2D_Color32(25, 67, 36, 255));

    if (radar > 0)
    {
        C2D_DrawRectSolid(
            222, 225, 0.6f,
            static_cast<float>(86 * radar) / 100,
            5, green);
    }
}

// ---------------------------------------------
// Moving CRT interference.
// Only drawn during gameplay.
// ---------------------------------------------

void draw_interference()
{
    // One pixel every 18 ms. Smooth movement
    // without needing a separate animation loop.
    const int y =
        static_cast<int>((osGetTime() / 18) % 240);

    // Faint static scanlines.
    for (int line = 8; line < 235; line += 8)
    {
        C2D_DrawRectSolid(
            6,
            static_cast<float>(line),
            0.90f,
            308, 1,
            C2D_Color32(63, 147, 76, 15));
    }

    // Soft moving interference line.
    C2D_DrawRectSolid(
        6,
        static_cast<float>(y),
        0.95f,
        308, 2,
        C2D_Color32(130, 255, 151, 48));

    // Very faint trail immediately behind it.
    if (y >= 3)
    {
        C2D_DrawRectSolid(
            6,
            static_cast<float>(y - 3),
            0.94f,
            308, 1,
            C2D_Color32(88, 220, 112, 20));
    }
}

// ---------------------------------------------
// Complete bottom screen.
// ---------------------------------------------


void draw()
{
    C2D_DrawRectSolid(
        0, 0, 0.1f,
        320, 240,
        crt_background());

    // Carcasa exterior física.
    draw_watch_frame();

    // Superficie única del monitor.
    draw_screen_background();

    // Contenido HUD.
    draw_ecg();
    draw_status();
    draw_automap();
    draw_info();

    // Interferencia decorativa.
    draw_interference();

    // ---------------------------------------------
    // Temporary FPS counter for performance testing.
    // ---------------------------------------------

    static u64 fps_sample_start = osGetTime();
    static int fps_frame_count = 0;
    static int current_fps = 0;

    ++fps_frame_count;

    const u64 now = osGetTime();
    const u64 elapsed = now - fps_sample_start;

    if (elapsed >= 1000)
    {
        current_fps = static_cast<int>(
            (static_cast<u64>(fps_frame_count) * 1000) /
            elapsed);

        fps_frame_count = 0;
        fps_sample_start = now;
    }

    // Small overlay in the upper-right HUD area.
    C2D_DrawRectSolid(
        246, 8, 0.79f,
        61, 16,
        C2D_Color32(2, 9, 5, 255));

    char fps_text[20];

    std::snprintf(
        fps_text,
        sizeof(fps_text),
        "FPS %d",
        current_fps);

    draw_text(
        250, 12,
        fps_text,
        crt_bright());
}


} // namespace n3ds_hud
} // namespace bstone
