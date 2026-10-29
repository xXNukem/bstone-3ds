/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Former "D3_DR_A2.ASM"
//
// Define "W3D" to emulate "D3_DR_A.ASM".

#include <cmath>

#include <vector>

#include "3d_def.h"
#include "id_vl.h"

void HitHorizWall();
void HitVertWall();
void HitHorizDoor();
void HitVertDoor();
void HitHorizPWall();
void HitVertPWall();

extern FineTangent finetangent;
extern int viewwidth;
extern TileMap tilemap;
extern SpotVis spotvis;
extern std::vector<int> pixelangle;
extern int midangle;
extern int focaltx;
extern int focalty;
extern int viewtx;
extern int viewty;
extern double viewx;
extern double viewy;
extern double xpartialup;
extern double xpartialdown;
extern double ypartialup;
extern double ypartialdown;
extern int tilehit;
extern int pixx;
extern WallHeight wallheight;
extern int xtile;
extern int ytile;
extern int xtilestep;
extern int ytilestep;
extern double xintercept;
extern double yintercept;
double xstep;
double ystep;
extern DoorPositions doorposition;
extern double pwallpos;

namespace {

constexpr auto DEG90 = std::intptr_t{900};
constexpr auto DEG180 = std::intptr_t{1800};
constexpr auto DEG270 = std::intptr_t{2700};
constexpr auto DEG360 = std::intptr_t{3600};

double get_integral_1(double value) noexcept
{
	return value < 0 ? std::floor(value) : std::trunc(value);
}

double get_fractional_1(double value) noexcept
{
	double integral;
	return std::modf(1 + value, &integral);
}

} // namespace



void AsmRefresh()
{
#if defined(__3DS__) || defined(_3DS)

    // ARM11 optimized coordinate calculations.

    const auto get_integral = [](double value) noexcept -> double
    {
        return static_cast<double>(
            static_cast<int>(value));
    };

    const auto get_fractional = [](double value) noexcept -> double
    {
        return value - static_cast<double>(
            static_cast<int>(value));
    };

    const auto get_integral_1 = [](double value) noexcept -> double
    {
        const int integer = static_cast<int>(value);

        return static_cast<double>(
            integer - (value < static_cast<double>(integer)));
    };

    const auto get_fractional_1 = [](double value) noexcept -> double
    {
        const double shifted = 1.0 + value;

        return shifted - static_cast<double>(
            static_cast<int>(shifted));
    };

#endif

    vid_hw_clear_wall_render_list();
    vid_hw_clear_pushwall_render_list();
    vid_hw_clear_door_render_list();

    for (pixx = 0; pixx < viewwidth; ++pixx)
    {
        auto angle = midangle + pixelangle[pixx];

        while (angle < 0)
        {
            angle += DEG360;
        }

        while (angle >= DEG360)
        {
            angle -= DEG360;
        }

        auto xpartial = 0.0;
        auto ypartial = 0.0;

        if (angle >= DEG270)
        {
            xtilestep = 1;
            ytilestep = 1;

            xstep = finetangent[angle - DEG270];
            ystep = finetangent[DEG360 - 1 - angle];

            xpartial = xpartialup;
            ypartial = ypartialup;
        }
        else if (angle >= DEG180)
        {
            xtilestep = -1;
            ytilestep = 1;

            xstep = -finetangent[DEG270 - 1 - angle];
            ystep = finetangent[angle - DEG180];

            xpartial = xpartialdown;
            ypartial = ypartialup;
        }
        else if (angle >= DEG90)
        {
            xtilestep = -1;
            ytilestep = -1;

            xstep = -finetangent[angle - DEG90];
            ystep = -finetangent[DEG180 - 1 - angle];

            xpartial = xpartialdown;
            ypartial = ypartialdown;
        }
        else
        {
            xtilestep = 1;
            ytilestep = -1;

            xstep = finetangent[DEG90 - 1 - angle];
            ystep = -finetangent[angle];

            xpartial = xpartialup;
            ypartial = ypartialdown;
        }

        // Initialize ray intersections.

        yintercept = viewy + ystep * xpartial;
        auto yint_h = static_cast<int>(yintercept);

        xtile = focaltx + xtilestep;
        auto xs_x = xtile;
        auto xs_y = yint_h;

        const auto xint_1 = viewx + xstep * ypartial;
        auto xint_h = static_cast<int>(get_integral_1(xint_1));

        xintercept = get_fractional_1(xint_1);

        auto yt = focalty + ytilestep;
        auto ys_x = xint_h;
        auto ys_y = yt;

        auto xt = xtile;


        #if defined(__3DS__) || defined(_3DS)

                // New 3DS: 16.16 fixed-point DDA fast path.
                // Handle empty tiles and ordinary walls using
                // integer arithmetic.
                //
                // Doors, pushwalls and numerically sensitive
                // intersections use the original traversal below.

                constexpr std::int32_t fixed_one = 65536;
                constexpr int fixed_shift = 16;

                bool fast_rendered = false;

                if (yintercept >= 0.0 &&
                    xint_1 >= 0.0 &&
                    std::abs(xstep) < 1024.0 &&
                    std::abs(ystep) < 1024.0)
                {
                    auto fast_x = static_cast<std::int32_t>(
                        xint_1 * fixed_one);

                    auto fast_y = static_cast<std::int32_t>(
                        yintercept * fixed_one);

                    const auto fast_xstep =
                        static_cast<std::int32_t>(
                            xstep * fixed_one);

                    const auto fast_ystep =
                        static_cast<std::int32_t>(
                            ystep * fixed_one);

                    auto fast_xt = xt;
                    auto fast_yt = yt;

                    bool fast_previous_vertical = true;

                    int fast_steps = 0;

                    for (;;)
                    {
                        if (fast_x < 0 || fast_y < 0)
                        {
                            break;
                        }

                        const int fast_xint_h =
                            fast_x >> fixed_shift;

                        const int fast_yint_h =
                            fast_y >> fixed_shift;

                        // Quantization guard.
                        // Falling back near integer boundaries
                        // avoids changing corner intersections.

                        const int margin = 2 * (fast_steps + 2);

                        const int xfrac = fast_x & 0xFFFF;
                        const int yfrac = fast_y & 0xFFFF;

                        if (xfrac <= margin ||
                            xfrac >= fixed_one - margin ||
                            yfrac <= margin ||
                            yfrac >= fixed_one - margin)
                        {
                            break;
                        }

                        const bool use_vertical =
                            fast_previous_vertical
                            ? !(
                                (ytilestep < 0 &&
                                    fast_yint_h <= fast_yt) ||
                                (ytilestep > 0 &&
                                    fast_yint_h >= fast_yt))
                            : (
                                (xtilestep > 0 &&
                                    fast_xint_h >= fast_xt) ||
                                (xtilestep < 0 &&
                                    fast_xint_h <= fast_xt));

                        fast_previous_vertical = use_vertical;

                        if (use_vertical)
                        {
                            if (fast_xt < 0 ||
                                fast_xt >= MAPSIZE ||
                                fast_yint_h < 0 ||
                                fast_yint_h >= MAPSIZE)
                            {
                                break;
                            }

                            const int tile =
                                tilemap[fast_xt][fast_yint_h];

                            if (tile != 0)
                            {
                                // Complex geometry uses the
                                // original double-precision path.
                                if ((tile & 0x80) != 0)
                                {
                                    break;
                                }

                                tilehit = tile;

                                xtile = fast_xt;
                                ytile = fast_yint_h;

                                xintercept = fast_xt;

                                yintercept =
                                    static_cast<double>(fast_y) /
                                    fixed_one;

                                HitVertWall();

                                fast_rendered = true;
                                break;
                            }

                            spotvis[fast_xt][fast_yint_h] = 1;

                            fast_xt += xtilestep;
                            fast_y += fast_ystep;
                        }
                        else
                        {
                            if (fast_xint_h < 0 ||
                                fast_xint_h >= MAPSIZE ||
                                fast_yt < 0 ||
                                fast_yt >= MAPSIZE)
                            {
                                break;
                            }

                            const int tile =
                                tilemap[fast_xint_h][fast_yt];

                            if (tile != 0)
                            {
                                if ((tile & 0x80) != 0)
                                {
                                    break;
                                }

                                tilehit = tile;

                                xtile = fast_xint_h;
                                ytile = fast_yt;

                                xintercept =
                                    static_cast<double>(fast_x) /
                                    fixed_one;

                                yintercept = fast_yt;

                                HitHorizWall();

                                fast_rendered = true;
                                break;
                            }

                            spotvis[fast_xint_h][fast_yt] = 1;

                            fast_yt += ytilestep;
                            fast_x += fast_xstep;
                        }

                        ++fast_steps;
                    }
                }

                // An ordinary wall was rendered using the
                // fixed-point path. Continue with the next ray.
                if (fast_rendered)
                {
                    continue;
                }

                // Otherwise, the original DDA below starts
                // from its untouched initial coordinates.

        #endif


        // Direct DDA traversal.
        // Preserve original tie-breaking depending on
        // whether the previous step was vertical or horizontal.

        bool previous_was_vertical = true;

        for (;;)
        {
            const bool use_vertical = previous_was_vertical
                ? !((ytilestep < 0 && yint_h <= yt) ||
                    (ytilestep > 0 && yint_h >= yt))
                : ((xtilestep > 0 && xint_h >= xt) ||
                    (xtilestep < 0 && xint_h <= xt));

            previous_was_vertical = use_vertical;

            if (use_vertical)
            {
                if (tilemap[xs_x][xs_y] != 0)
                {
                    tilehit = tilemap[xs_x][xs_y];

                    if ((tilehit & 0x80) != 0)
                    {
                        xtile = xt;
                        yintercept = yint_h + get_fractional(yintercept);

                        if ((tilehit & 0x40) != 0)
                        {
                            // Vertical pushwall.

                            const auto intercept = ystep * pwallpos;
                            const auto yint =
                                get_fractional(intercept) +
                                get_fractional(yintercept);

                            const auto intercept_l =
                                get_fractional_1(yint);

                            const auto intercept_h =
                                yint_h +
                                static_cast<int>(
                                    intercept + get_integral_1(yint));

                            if (yint_h == intercept_h)
                            {
                                yintercept = intercept_h + intercept_l;
                                xintercept = xt;

                                HitVertPWall();
                                break;
                            }
                        }
                        else
                        {
                            // Vertical door.

                            const auto intercept =
                                yintercept + 0.5 * ystep;

                            const auto intercept_l =
                                get_fractional(intercept);

                            const auto intercept_h =
                                static_cast<int>(intercept);

                            if (static_cast<int>(yintercept) ==
                                intercept_h)
                            {
                                const auto doorpos = tilehit & 0x7F;
                                auto skip_draw = false;

#ifndef W3D
                                if (intercept_l < 0.5)
                                {
                                    if (2 * intercept_l >
                                        1 - doorposition[doorpos])
                                    {
                                        skip_draw = true;
                                    }
                                }
                                else
                                {
                                    if (2 * intercept_l <
                                        1 + doorposition[doorpos])
                                    {
                                        skip_draw = true;
                                    }
                                }
#else
                                if (intercept_l < doorposition[doorpos])
                                {
                                    skip_draw = true;
                                }
#endif

                                if (!skip_draw)
                                {
                                    yintercept =
                                        intercept_l +
                                        get_integral(yintercept);

                                    xintercept = xtile + 0.5;

                                    HitVertDoor();
                                    break;
                                }
                            }
                        }

                        // Continue through an open door.

                        xt = xtile;
                        yint_h = static_cast<int>(yintercept);
                    }
                    else
                    {
                        // Ordinary vertical wall.

                        xintercept = xt;
                        xtile = xt;

                        yintercept =
                            yint_h + get_fractional(yintercept);

                        ytile = yint_h;

                        HitVertWall();
                        break;
                    }
                }

                spotvis[xs_x][xs_y] = 1;

                xt += xtilestep;

                {
                    const auto yint_1 =
                        get_fractional(yintercept) +
                        get_fractional(ystep);

                    yintercept =
                        get_integral(yintercept) +
                        get_fractional_1(yint_1);

                    yint_h += static_cast<int>(
                        ystep + get_integral_1(yint_1));
                }

                xs_x = xt;
                xs_y = yint_h;

                continue;
            }

            // Horizontal intersection.

            if (tilemap[ys_x][ys_y] != 0)
            {
                tilehit = tilemap[ys_x][ys_y];

                if ((tilehit & 0x80) != 0)
                {
                    xtile = xt;
                    yintercept =
                        yint_h + get_fractional(yintercept);

                    if ((tilehit & 0x40) != 0)
                    {
                        // Horizontal pushwall.

                        const auto intercept = xstep * pwallpos;
                        const auto xint =
                            get_fractional(intercept) +
                            get_fractional(xintercept);

                        const auto intercept_l =
                            get_fractional_1(xint);

                        const auto intercept_h =
                            xint_h +
                            static_cast<int>(
                                intercept + get_integral_1(xint));

                        if (xint_h == intercept_h)
                        {
                            xintercept = intercept_h + intercept_l;
                            yintercept = yt;

                            HitHorizPWall();
                            break;
                        }
                    }
                    else
                    {
                        // Horizontal door.

                        const auto intercept = 0.5 * xstep;
                        const auto xint =
                            get_fractional(intercept) +
                            get_fractional(xintercept);

                        const auto intercept_l =
                            get_fractional_1(xint);

                        const auto intercept_h =
                            xint_h +
                            static_cast<int>(
                                intercept + get_integral_1(xint));

                        if (xint_h == intercept_h)
                        {
                            const auto doorpos = tilehit & 0x7F;
                            auto skip_draw = false;

#ifndef W3D
                            if (intercept_l < 0.5)
                            {
                                if (2 * intercept_l >
                                    1 - doorposition[doorpos])
                                {
                                    skip_draw = true;
                                }
                            }
                            else
                            {
                                if (2 * intercept_l <
                                    1 + doorposition[doorpos])
                                {
                                    skip_draw = true;
                                }
                            }
#else
                            if (intercept_l < doorposition[doorpos])
                            {
                                skip_draw = true;
                            }
#endif

                            if (!skip_draw)
                            {
                                xintercept =
                                    xint_h + intercept_l;

                                yintercept = yt + 0.5;

                                HitHorizDoor();
                                break;
                            }
                        }
                    }

                    // Continue through an open door.

                    xt = xtile;
                    yint_h = static_cast<int>(yintercept);
                }
                else
                {
                    // Ordinary horizontal wall.

                    xintercept =
                        xint_h + get_fractional(xintercept);

                    xtile = xint_h;

                    yintercept = yt;
                    ytile = yt;

                    HitHorizWall();
                    break;
                }
            }

            spotvis[ys_x][ys_y] = 1;

            yt += ytilestep;

            {
                const auto xint_2 =
                    get_fractional(xintercept) +
                    get_fractional(xstep);

                xintercept =
                    get_integral(xintercept) +
                    get_fractional_1(xint_2);

                xint_h += static_cast<int>(
                    xstep + get_integral_1(xint_2));
            }

            ys_x = xint_h;
            ys_y = yt;
        }
    }
}

