// SPDX-License-Identifier: GPL-2.0-or-later
#include "bstone_3ds_display.h"
#include <stdexcept>
namespace bstone { namespace n3ds_display {
namespace { bool c3d{},c2d{}; C3D_RenderTarget* left{};C3D_RenderTarget* right{};C3D_RenderTarget* lower{}; }
C3D_RenderTarget* top(bool r) noexcept { return r ? right : left; }
C3D_RenderTarget* bottom() noexcept { return lower; }
void shutdown() noexcept
{
    if (!c3d) { return; }
    C3D_FrameSync();
    if (lower) { C3D_RenderTargetDelete(lower); }
    if (right) { C3D_RenderTargetDelete(right); }
    if (left) { C3D_RenderTargetDelete(left); }
    lower=right=left=nullptr;
    if (c2d) { C2D_Fini();c2d=false; }
    C3D_Fini();c3d=false;
}
void initialize()
{
    if (c3d && c2d && left && right && lower) { C2D_Prepare();return; }
    try {
        if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) { throw std::runtime_error("Citro3D initialization failed"); }
        c3d=true;
        if (!C2D_Init(C2D_DEFAULT_MAX_OBJECTS*4)) { throw std::runtime_error("Citro2D initialization failed"); }
        c2d=true;C2D_Prepare();gfxSet3D(true);
        // SDL initializes RGBA8 framebuffers. Citro2D screen transfers output
        // RGB8: establish matching scanout formats BEFORE either screen's first
        // transfer, including the resource-independent startup selector.
        gfxSetScreenFormat(GFX_TOP,GSP_BGR8_OES);
        gfxSetScreenFormat(GFX_BOTTOM,GSP_BGR8_OES);
        left=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);right=C2D_CreateScreenTarget(GFX_TOP,GFX_RIGHT);
        lower=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
        if (!left || !right || !lower) { throw std::runtime_error("3DS render target creation failed"); }
    } catch (...) { shutdown();throw; }
}
} }
