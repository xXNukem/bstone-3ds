#include "bstone_3ds_automap.h"

#include "3d_def.h"
#include <citro2d.h>

namespace bstone {
namespace n3ds_hud {

void draw_automap()
{
    if (!player)
    {
        return;
    }

    // ---------------------------------------------
    // Expanded automap viewport.
    // ---------------------------------------------

    constexpr float left = 8.0f;
    constexpr float top = 64.0f;
    constexpr float right = 312.0f;
    constexpr float bottom = 176.0f;

    constexpr float center_x = 160.0f;
    constexpr float center_y = 120.0f;
    constexpr float tile_size = 8.0f;

    // ---------------------------------------------
    // Green phosphor CRT palette.
    // ---------------------------------------------

    // Explored floor:
    // subtle translucent tint so the global
    // background grid remains visible.
    const u32 floor_color =
        C2D_Color32(36, 95, 53, 145);

    // Walls: medium phosphor green.
    const u32 wall_color =
        C2D_Color32(57, 126, 75, 255);

    // Doors: brighter than walls.
    const u32 door_color =
        C2D_Color32(100, 215, 135, 255);

    // Locked doors: very bright green.
    const u32 locked_color =
        C2D_Color32(174, 255, 186, 255);

    // Player: almost white phosphor.
    const u32 player_color =
        C2D_Color32(226, 255, 230, 255);

    const int player_tile_x = player->tilex;
    const int player_tile_y = player->tiley;

    // ---------------------------------------------
    // Render nearby discovered tiles.
    // ---------------------------------------------

    for (int y = player_tile_y - 8;
         y <= player_tile_y + 8;
         ++y)
    {
        if (y < 0 || y >= MAPSIZE)
        {
            continue;
        }

        for (int x = player_tile_x - 21;
             x <= player_tile_x + 21;
             ++x)
        {
            if (x < 0 || x >= MAPSIZE)
            {
                continue;
            }

            // Only show discovered tiles.
            if ((travel_table_[x][y] & TT_TRAVELED) == 0)
            {
                continue;
            }

            // Smooth scrolling using the player's
            // actual position.
            const float screen_x =
                center_x +
                static_cast<float>(x - player->x) *
                tile_size;

            const float screen_y =
                center_y +
                static_cast<float>(y - player->y) *
                tile_size;

            // -------------------------------------
            // Clip tiles to the viewport.
            // -------------------------------------

            const float x1 =
                screen_x < left ? left : screen_x;

            const float y1 =
                screen_y < top ? top : screen_y;

            const float x2 =
                screen_x + tile_size > right ?
                right : screen_x + tile_size;

            const float y2 =
                screen_y + tile_size > bottom ?
                bottom : screen_y + tile_size;

            if (x2 <= x1 || y2 <= y1)
            {
                continue;
            }

            const int tile = tilemap[x][y];

            u32 color = floor_color;

            // -------------------------------------
            // Walls and doors.
            // -------------------------------------

            if ((tile & tilemap_door_flag) != 0)
            {
                const int door_index =
                    tile & tilemap_wall_mask;

                const auto& door =
                    doorobjlist[door_index];

                if (door.lock != kt_none)
                {
                    color = locked_color;
                }
                else if (door.action == dr_closed)
                {
                    color = door_color;
                }
                else
                {
                    color = floor_color;
                }
            }
            else if (tile != 0)
            {
                color = wall_color;
            }

            C2D_DrawRectSolid(
                x1,
                y1,
                0.6f,
                x2 - x1,
                y2 - y1,
                color);
        }
    }

    // ---------------------------------------------
    // Player position and direction.
    // ---------------------------------------------

    // Blake Stone:
    // 0 degrees = East
    // 90 = North
    // 180 = West
    // 270 = South

    const int direction =
        ((player->angle + 22) / 45) % 8;

    static constexpr int dx[8] =
    {
        1, 1, 0, -1, -1, -1, 0, 1
    };

    static constexpr int dy[8] =
    {
        0, -1, -1, -1, 0, 1, 1, 1
    };

    const int dir_x = dx[direction];
    const int dir_y = dy[direction];

    // ---------------------------------------------
    // Player center.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        center_x - 2.0f,
        center_y - 2.0f,
        0.8f,
        5.0f,
        5.0f,
        player_color);

    // ---------------------------------------------
    // Direction arrow tip.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        center_x + dir_x * 7 - 1,
        center_y + dir_y * 7 - 1,
        0.9f,
        3.0f,
        3.0f,
        player_color);

    // ---------------------------------------------
    // Direction arrow wings.
    // ---------------------------------------------

    C2D_DrawRectSolid(
        center_x + dir_x * 4 + dir_y * 2 - 1,
        center_y + dir_y * 4 - dir_x * 2 - 1,
        0.9f,
        2.0f,
        2.0f,
        player_color);

    C2D_DrawRectSolid(
        center_x + dir_x * 4 - dir_y * 2 - 1,
        center_y + dir_y * 4 + dir_x * 2 - 1,
        0.9f,
        2.0f,
        2.0f,
        player_color);
}

} // namespace n3ds_hud
} // namespace bstone
