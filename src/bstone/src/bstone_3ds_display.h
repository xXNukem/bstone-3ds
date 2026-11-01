// Shared owner: SDL owns gfx/HID; this owns Citro resources only.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef BSTONE_3DS_DISPLAY_INCLUDED
#define BSTONE_3DS_DISPLAY_INCLUDED
#include <citro2d.h>
namespace bstone { namespace n3ds_display {
void initialize();
void shutdown() noexcept;
C3D_RenderTarget* top(bool right = false) noexcept;
C3D_RenderTarget* bottom() noexcept;
} }
#endif
