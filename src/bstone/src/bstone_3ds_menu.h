// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef BSTONE_3DS_MENU_INCLUDED
#define BSTONE_3DS_MENU_INCLUDED
#include <cstdint>

struct CP_iteminfo;
struct CP_itemtype;

namespace bstone { namespace n3ds_menu {
// The panel lifetime also owns the persistent bottom-screen title.
void begin(bool planet_strike, bool paused);
void end();
bool active();
void set_header(const char* title);
void begin_frame();
void begin_eye(int eye, float slider);
float content_shift();
bool needs_refresh();
void idle();
// Nestable main-thread loading presentation; no artificial minimum duration.
void begin_loading();
void end_loading();
bool loading();
void loading_step();
void suspend_loading();
void draw_bottom();
void draw_background();
// Resource-independent startup use of the same CRT presentation.
void draw_ambient_background(int width, int height, std::uint64_t milliseconds);
void clear_content();
void set_footer(const char* text);
void show_list(const CP_iteminfo& info, const CP_itemtype* items);
void show_document(const char* title, bool can_continue);
void document_page(int index, int count);
bool scroll_document(int direction);
bool document();
bool accepts_text();
void text_paint(bool enabled);
void pixel(int x, int y);
void clear_ink(int x, int y, int width, int height);
std::uint32_t revision();
std::uint32_t sample_ui(int x, int y, const std::uint32_t* palette);
void show_meters(int selected, int sfx, int music, int maximum);
// Presentation only: item ownership, navigation and actions stay in 3d_menu.
void show(const CP_iteminfo& info, const CP_itemtype* items, bool paused, bool planet_strike);
void select(int index);
// Hold the native menu through palette-only presentations. Release it only at
// the next explicit content publication (VW_UpdateScreen), not during a fade.
void leave_on_next_update();
void publish_destination();
void hide();
void draw();
} }
#endif
