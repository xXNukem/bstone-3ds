#include "bstone_3ds_intro.h"

#include "gfxv.h"
#include "id_ca.h"
#include "id_vh.h"
#include <citro2d.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bstone {
namespace n3ds_intro {



Stage& current_stage()
{
    static Stage stage = Stage::none;
    return stage;
}

IntroFonts& fonts()
{
    static IntroFonts result;
    return result;
}

void set_stage(Stage stage)
{
    if (stage != Stage::none)
    {
        auto& data = fonts();

        // Keep copies of the genuine BStone fonts.
        // The JAM animation temporarily unloads
        // some of the original game resources.
        if (data.title.empty() || data.menu.empty())
        {
            for (int number = 3; number <= 4; ++number)
            {
                auto& destination =
                    number == 3 ? data.title : data.menu;

                if (!destination.empty())
                {
                    continue;
                }

                const auto index =
                    static_cast<std::int16_t>(
                        STARTFONT + number);

                CA_CacheGrChunk(index);

                const auto& source = grsegs[index];

                if (source.size() >= sizeof(fontstruct))
                {
                    destination.assign(
                        source.begin(),
                        source.end());
                }
            }
        }
    }

    current_stage() = stage;
}

// --------------------------------------------------
// Original BStone proportional font rendering.
// Font 3: menu titles.
// Font 4: menu items.
// --------------------------------------------------

int font_text_width(
    const std::vector<std::uint8_t>& data,
    const char* text)
{
    if (data.size() < sizeof(fontstruct))
    {
        return 0;
    }

    const auto* font =
        reinterpret_cast<const fontstruct*>(
            data.data());

    int width = 0;

    for (const auto* p =
             reinterpret_cast<const unsigned char*>(text);
         *p; ++p)
    {
        const auto ch =
            static_cast<unsigned char>(
                *p >= 'a' && *p <= 'z' ?
                *p - 32 : *p);

        width += static_cast<unsigned char>(
            font->width[ch]);
    }

    return width;
}

void draw_font(
    int x,
    int y,
    const char* text,
    const std::vector<std::uint8_t>& data,
    int scale,
    u32 color)
{
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
             reinterpret_cast<const unsigned char*>(text);
         *p; ++p)
    {
        const auto ch =
            static_cast<unsigned char>(
                *p >= 'a' && *p <= 'z' ?
                *p - 32 : *p);

        const int width =
            static_cast<unsigned char>(
                font->width[ch]);

        const int offset = font->location[ch];

        if (width > 0 &&
            width <= 64 &&
            offset >= 0 &&
            static_cast<std::size_t>(offset) +
                static_cast<std::size_t>(
                    width * height) <= data.size())
        {
            const auto* glyph =
                data.data() + offset;

            for (int row = 0; row < height; ++row)
            {
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

                    // Combine consecutive lit pixels
                    // to reduce Citro2D draw calls.
                    C2D_DrawRectSolid(
                        static_cast<float>(
                            x + begin * scale),
                        static_cast<float>(
                            y + row * scale),
                        0.8f,
                        static_cast<float>(
                            (column - begin) * scale),
                        static_cast<float>(scale),
                        color);
                }
            }
        }

        x += width * scale;
    }
}

void draw_centered(
    const char* text,
    int y,
    bool is_title,
    int scale,
    u32 color)
{
    const auto& data =
        is_title ? fonts().title : fonts().menu;

    const int width =
        font_text_width(data, text);

    if (width <= 0)
    {
        return;
    }

    // Avoid cropping long lines.
    if (width * scale > 286)
    {
        scale = 1;
    }

    const int x =
        (320 - width * scale) / 2;

    // Subtle static phosphor afterglow.
    // No animation or extra frame updates.
    draw_font(
        x + 1,
        y + 1,
        text,
        data,
        scale,
        C2D_Color32(30, 81, 43, 255));

    draw_font(
        x,
        y,
        text,
        data,
        scale,
        color);
}

// --------------------------------------------------
// Green phosphor CRT presentation.
// --------------------------------------------------

void draw()
{
    const auto stage = current_stage();

    if (stage == Stage::none)
    {
        return;
    }

    // Monochrome phosphor palette.
    const u32 background =
        C2D_Color32(2, 9, 5, 255);

    const u32 screen =
        C2D_Color32(5, 20, 10, 255);

    const u32 frame =
        C2D_Color32(25, 67, 36, 255);

    const u32 grid =
        C2D_Color32(8, 29, 16, 255);

    const u32 bright =
        C2D_Color32(168, 255, 179, 255);

    const u32 green =
        C2D_Color32(88, 235, 121, 255);

    const u32 dim =
        C2D_Color32(76, 153, 93, 255);

    // ---------------------------------------------
    // CRT background and bezel.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        0, 0, 0.1f,
        320, 240, background);

    C2D_DrawRectSolid(
        6, 6, 0.2f,
        308, 228, frame);

    C2D_DrawRectSolid(
        8, 8, 0.3f,
        304, 224, screen);

    // Faint static scanlines.
    for (int y = 12; y < 229; y += 6)
    {
        C2D_DrawRectSolid(
            10,
            static_cast<float>(y),
            0.4f,
            300, 1, grid);
    }

    // Terminal corners.
    C2D_DrawRectSolid(
        14, 14, 0.5f, 28, 2, green);

    C2D_DrawRectSolid(
        278, 14, 0.5f, 28, 2, green);

    C2D_DrawRectSolid(
        14, 224, 0.5f, 28, 2, green);

    C2D_DrawRectSolid(
        278, 224, 0.5f, 28, 2, green);

    // Horizontal separators.
    C2D_DrawRectSolid(
        25, 38, 0.5f, 270, 1, frame);

    C2D_DrawRectSolid(
        25, 208, 0.5f, 270, 1, frame);

    // ---------------------------------------------
    // Presentation content.
    // ---------------------------------------------

    switch (stage)
    {
        // -----------------------------------------
        // Anti-piracy screen.
        // -----------------------------------------

        case Stage::piracy:
        {
            draw_centered(
                "CYANIDE SOFTWARE",
                48, true, 2, bright);

            draw_centered(
                "PRESENTS",
                86, false, 1, dim);

            C2D_DrawRectSolid(
                83, 105, 0.5f,
                154, 1, frame);

            draw_centered(
                "A 3DS PORT MADE BY",
                113, false, 1, dim);

            draw_centered(
                "CARLOS \"XXNUKEM\"",
                133, false, 2, bright);

            draw_centered(
                "DE LA BARRERA",
                158, false, 2, green);

            draw_centered(
                "ORIGINAL BSTONE PORT BY",
                182, false, 1, dim);

            draw_centered(
                "BORIS I. BENDOVSKY",
                195, false, 1, green);

            break;
        }

        // -----------------------------------------
        // Apogee.
        // -----------------------------------------

        case Stage::apogee:
        {
            draw_centered(
                "PUBLISHED BY",
                76, false, 2, dim);

            draw_centered(
                "APOGEE",
                111, true, 3, bright);

            draw_centered(
                "SOFTWARE",
                160, false, 2, green);

            break;
        }

        // -----------------------------------------
        // JAM Productions.
        // -----------------------------------------

        case Stage::jam:
        {
            draw_centered(
                "ORIGINALLY DEVELOPED BY",
                84, false, 1, dim);

            draw_centered(
                "JAM",
                113, true, 3, bright);

            draw_centered(
                "PRODUCTIONS",
                168, false, 2, green);

            break;
        }

        // -----------------------------------------
        // PG-13 warning.
        // -----------------------------------------

        case Stage::pg13:
        {
            draw_centered(
                "CONTENT ADVISORY",
                67, false, 1, dim);

            draw_centered(
                "WARNING!",
                93, true, 2, bright);

            C2D_DrawRectSolid(
                78, 132, 0.5f,
                164, 1, frame);

            draw_centered(
                "TONS OF VIOLENCE",
                145, false, 2, green);

            draw_centered(
                "AHEAD!",
                172, false, 2, bright);

            draw_centered(
                "YOU HAVE BEEN WARNED.",
                196, false, 1, dim);

            break;
        }

        // -----------------------------------------
        // Title, credits and high scores.
        // -----------------------------------------

        case Stage::title:
        {
            draw_centered(
                "BLAKE STONE:",
                65, true, 2, bright);

            draw_centered(
                "ALIENS OF GOLD",
                103, true, 2, green);

            draw_centered(
                "3DS",
                141, true, 2, bright);

            C2D_DrawRectSolid(
                70, 183, 0.5f,
                180, 1, frame);

            draw_centered(
                "PRESS ANY KEY TO CONTINUE",
                193, false, 1, green);

            break;
        }

        default:
            break;
    }

    // ---------------------------------------------
    // Terminal footer.
    // ---------------------------------------------

    draw_centered(
        "BSTONE 3DS",
        216, false, 1, dim);
}

} // namespace n3ds_intro
} // namespace bstone
