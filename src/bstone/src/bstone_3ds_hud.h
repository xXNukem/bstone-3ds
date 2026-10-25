#include "bstone_3ds_automap.h"
#ifndef BSTONE_3DS_HUD_INCLUDED
#define BSTONE_3DS_HUD_INCLUDED

#if defined(__3DS__) || defined(_3DS)

#include "3d_def.h"
#include <citro2d.h>
#include <cstdio>

namespace bstone {
namespace n3ds_hud {

// --------------------------------------------------
// Pixel font.
// --------------------------------------------------

inline void draw_text(int x, int y, const char* value, u32 color, int scale)
{
    static constexpr unsigned char font[38][5] =
    {
        {7,5,5,5,7}, // 0
        {2,6,2,2,7}, // 1
        {7,1,7,4,7}, // 2
        {7,1,7,1,7}, // 3
        {5,5,7,1,1}, // 4
        {7,4,7,1,7}, // 5
        {7,4,7,5,7}, // 6
        {7,1,1,1,1}, // 7
        {7,5,7,5,7}, // 8
        {7,5,7,1,7}, // 9

        {2,5,7,5,5}, // A
        {6,5,6,5,6}, // B
        {3,4,4,4,3}, // C
        {6,5,5,5,6}, // D
        {7,4,6,4,7}, // E
        {7,4,6,4,4}, // F
        {3,4,5,5,3}, // G
        {5,5,7,5,5}, // H
        {7,2,2,2,7}, // I
        {1,1,1,5,2}, // J
        {5,5,6,5,5}, // K
        {4,4,4,4,7}, // L
        {5,7,7,5,5}, // M
        {5,7,7,7,5}, // N
        {7,5,5,5,7}, // O
        {6,5,6,4,4}, // P
        {7,5,5,7,1}, // Q
        {6,5,6,5,5}, // R
        {3,4,7,1,6}, // S
        {7,2,2,2,2}, // T
        {5,5,5,5,7}, // U
        {5,5,5,5,2}, // V
        {5,5,7,7,5}, // W
        {5,5,2,5,5}, // X
        {5,5,2,2,2}, // Y
        {7,1,2,4,7}, // Z

        {5,1,2,4,5}, // %
        {0,2,0,2,0}  // :
    };

    for (const char* p = value; *p; ++p)
    {
        const int glyph =
            (*p >= '0' && *p <= '9') ? *p - '0' :
            (*p >= 'A' && *p <= 'Z') ? *p - 'A' + 10 :
            *p == '%' ? 36 :
            *p == ':' ? 37 : -1;

        if (glyph >= 0)
        {
            for (int row = 0; row < 5; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    if (font[glyph][row] & (1 << (2 - col)))
                    {
                        C2D_DrawRectSolid(
                            static_cast<float>(x + col * scale),
                            static_cast<float>(y + row * scale),
                            0.8f,
                            static_cast<float>(scale),
                            static_cast<float>(scale),
                            color);
                    }
                }
            }
        }

        x += scale * 4;
    }
}

// --------------------------------------------------
// Vital signs monitor.
// --------------------------------------------------

inline void draw_ecg()
{
    const int health =
        gamestate.health < 0 ? 0 :
        gamestate.health > 100 ? 100 :
        gamestate.health;

    const u32 signal_color =
        health > 60 ? C2D_Color32(35, 240, 75, 255) :
        health > 25 ? C2D_Color32(255, 205, 45, 255) :
                      C2D_Color32(255, 55, 55, 255);

    const u32 grid_color = C2D_Color32(20, 45, 85, 255);

    // Monitor frame.
    C2D_DrawRectSolid(
        4, 4, 0.3f, 312, 68,
        C2D_Color32(45, 70, 95, 255));

    C2D_DrawRectSolid(
        7, 7, 0.4f, 306, 62,
        C2D_Color32(5, 12, 32, 255));

    // EKG grid.
    for (int x = 12; x < 224; x += 16)
    {
        C2D_DrawRectSolid(
            static_cast<float>(x),
            12, 0.5f, 1, 48, grid_color);
    }

    for (int y = 12; y <= 60; y += 12)
    {
        C2D_DrawRectSolid(
            12, static_cast<float>(y),
            0.5f, 212, 1, grid_color);
    }

    // ---------------------------------------------
    // Animated PQRST waveform.
    // ---------------------------------------------

    const int bpm = 70 + ((100 - health) * 17 / 10);
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
        -3, 22, -12, 0, 0,
        2, 8, 3, 0, 0
    };

    const auto elapsed = osGetTime();

    int previous_y = 39;

    for (int x = 14; x < 222; ++x)
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

            // Subtle changes between heartbeats.
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

        // Keep the signal inside the monitor.
        const int raw_y = 39 - wave;

        const int current_y =
            raw_y < 18 ? 18 :
            raw_y > 56 ? 56 :
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
            1.0f,
            static_cast<float>(bottom - top + 1),
            signal_color);

        previous_y = current_y;
    }

    // Illuminated signal point.
    C2D_DrawRectSolid(
        218.0f,
        static_cast<float>(previous_y - 1),
        0.8f,
        3.0f,
        3.0f,
        signal_color);

    // Health percentage.
    char health_text[5]{};

    std::snprintf(
        health_text,
        sizeof(health_text),
        "%d%%",
        health);

    int length = 0;

    while (health_text[length] != '\0')
    {
        ++length;
    }

    draw_text(
        244, 17,
        "HEALTH",
        C2D_Color32(107, 158, 191, 255),
        1);

    draw_text(
        294 - length * 12,
        33,
        health_text,
        signal_color,
        3);
}

// --------------------------------------------------
// Game status and automap area.
// --------------------------------------------------

inline void draw_status()
{
    const u32 border = C2D_Color32(49, 80, 112, 255);
    const u32 panel = C2D_Color32(13, 27, 43, 255);
    const u32 white = C2D_Color32(221, 237, 249, 255);
    const u32 muted = C2D_Color32(107, 158, 191, 255);
    const u32 cyan = C2D_Color32(55, 207, 237, 255);

    // ---------------------------------------------
    // Reserved automap area.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        4, 76, 0.2f, 312, 100, border);

    C2D_DrawRectSolid(
        6, 78, 0.3f, 308, 96,
        C2D_Color32(5, 17, 28, 255));

    const u32 grid_color =
        C2D_Color32(14, 36, 53, 255);

    for (int x = 14; x < 310; x += 20)
    {
        C2D_DrawRectSolid(
            static_cast<float>(x),
            79, 0.4f, 1, 94, grid_color);
    }

    for (int y = 86; y < 173; y += 20)
    {
        C2D_DrawRectSolid(
            7, static_cast<float>(y),
            0.4f, 306, 1, grid_color);
    }


    // ---------------------------------------------
    // Three information panels.
    // ---------------------------------------------

    const int panel_x[] = {4, 108, 216};
    const int panel_width[] = {100, 104, 100};

    for (int i = 0; i < 3; ++i)
    {
        C2D_DrawRectSolid(
            static_cast<float>(panel_x[i]),
            180, 0.2f,
            static_cast<float>(panel_width[i]),
            56, border);

        C2D_DrawRectSolid(
            static_cast<float>(panel_x[i] + 2),
            182, 0.3f,
            static_cast<float>(panel_width[i] - 4),
            52, panel);
    }

    char number[32]{};

    // ---------------------------------------------
    // Score, tokens, floor and lives.
    // ---------------------------------------------

    draw_text(10, 185, "SCORE", muted, 1);

    std::snprintf(
        number, sizeof(number), "TOK %u",
        static_cast<unsigned>(gamestate.tokens));

    draw_text(62, 185, number, muted, 1);

    std::snprintf(
        number, sizeof(number), "%d",
        static_cast<int>(gamestate.score));

    draw_text(10, 194, number, white, 2);

    draw_text(10, 211, "FLOOR", muted, 1);
    draw_text(62, 211, "LIVES", muted, 1);

    std::snprintf(
        number, sizeof(number), "%d",
        static_cast<int>(gamestate.mapon) + 1);

    draw_text(10, 220, number, white, 2);

    std::snprintf(
        number, sizeof(number), "%d",
        gamestate.lives > 0 ?
            static_cast<int>(gamestate.lives) : 0);

    draw_text(62, 220, number, white, 2);

    // ---------------------------------------------
    // Current weapon.
    // ---------------------------------------------

    const char* weapon = "NONE";

    switch (gamestate.weapon)
    {
        case wp_autocharge: weapon = "AUTOCHARGE"; break;
        case wp_pistol: weapon = "PISTOL"; break;
        case wp_burst_rifle: weapon = "BURST RIFLE"; break;
        case wp_ion_cannon: weapon = "ION CANNON"; break;
        case wp_grenade: weapon = "GRENADE"; break;
        case wp_bfg_cannon: weapon = "BFG CANNON"; break;
        default: break;
    }

    draw_text(114, 185, "WEAPON", muted, 1);
    draw_text(114, 195, weapon, white, 2);

    // ---------------------------------------------
    // Ammunition and weapon charge.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        114, 225, 0.5f, 88, 6,
        C2D_Color32(30, 45, 59, 255));

    if (gamestate.weapon == wp_autocharge)
    {
        const bool ready = gamestate.weapon_wait == 0;

        draw_text(
            114, 212,
            ready ? "READY" : "WAIT",
            ready ? cyan : muted,
            2);

        if (ready)
        {
            C2D_DrawRectSolid(
                114, 225, 0.6f, 88, 6, cyan);
        }
    }
    else if (gamestate.weapon >= 0)
    {
        const int ammo =
            gamestate.ammo < 0 ? 0 :
            gamestate.ammo > 100 ? 100 :
            gamestate.ammo;

        draw_text(114, 213, "AMMO", muted, 1);

        std::snprintf(
            number, sizeof(number), "%d%%", ammo);

        draw_text(154, 210, number, white, 2);

        if (ammo > 0)
        {
            C2D_DrawRectSolid(
                114, 225, 0.6f,
                static_cast<float>(88 * ammo) / 100.0f,
                6, cyan);
        }
    }

    // ---------------------------------------------
    // Access cards.
    // ---------------------------------------------

    draw_text(222, 185, "ACCESS", muted, 1);

    const int keys[] =
    {
        kt_red, kt_yellow, kt_green, kt_blue, kt_gold
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
            x, 195, 0.5f, 15, 11, key_colors[i]);

        C2D_DrawRectSolid(
            x + 2, 197, 0.6f, 11, 7,
            gamestate.numkeys[keys[i]] > 0 ?
                key_colors[i] :
                C2D_Color32(15, 25, 37, 255));
    }

    // ---------------------------------------------
    // Radar energy.
    // ---------------------------------------------

    const int radar =
        gamestate.rpower < 0 ? 0 :
        gamestate.rpower > MAX_RADAR_ENERGY ? 100 :
        gamestate.rpower * 100 / MAX_RADAR_ENERGY;

    draw_text(222, 213, "RADAR", muted, 1);

    std::snprintf(
        number, sizeof(number), "%d%%", radar);

    draw_text(286, 213, number, white, 1);

    C2D_DrawRectSolid(
        222, 225, 0.5f, 86, 6,
        C2D_Color32(30, 45, 59, 255));

    if (radar > 0)
    {
        C2D_DrawRectSolid(
            222, 225, 0.6f,
            static_cast<float>(86 * radar) / 100.0f,
            6, cyan);
    }
}

// --------------------------------------------------
// Draw the complete bottom-screen HUD.
// --------------------------------------------------

inline void draw()
{
    draw_ecg();
    draw_status();
    draw_automap();
}

} // namespace n3ds_hud
} // namespace bstone

#endif // 3DS
#endif // BSTONE_3DS_HUD_INCLUDED
