/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cmath>
#include <cstring>

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"
#include "jm_cio.h"
#include "jm_lzh.h"
#include "jm_tp.h"
#include <cstdio>
#include <ctime>

#include "bstone_exception.h"
#include "bstone_saved_game.h"
#include "bstone_scope_exit.h"
#if defined(__3DS__) || defined(_3DS)
#include <3ds.h>
#include "bstone_3ds_weapons.h"
#include "bstone_3ds_menu.h"
#endif

#define GAME_DESCRIPTION_LEN (31)

// Box behind text for cursor
#define HIGHLIGHT_BOX_COLOR (0x52)

#define HIGHLIGHT_DEACTIAVED_COLOR (0x55)

#define DISABLED_TEXT_COLOR (0x53)
#define DEACTIAVED_TEXT_COLOR (0x52)

#define INSTRUCTIONS_TEXT_COLOR (0x53)

#define TERM_SHADOW_COLOR (0x01)

//
// Clearable Menu Terminal Area
//

#define SCREEN_X (32)
#define SCREEN_Y (28)
#define SCREEN_W (244)
#define SCREEN_H (132)

#define BORD2COLOR (0x74)
#define DEACTIVE (0x72)
#define BKGDCOLOR (0x76)


#define READCOLOR (0x4A)
#define READHCOLOR (0x47)
#define TEXTCOLOR (WHITE)


#define SENSITIVE (60)
#define CENTER (SENSITIVE * 2)

#define MENU_X (111)
#define MENU_Y (50)

#define SM_X (121)
#define SM_Y (84)

#define CTL_X (100)
#define CTL_Y (70)

#define LSM_X (85)
#define LSM_Y (55)
#define LSM_W (144)

#define NM_X (71)
#define NM_Y (66)

#define NE_X (58)
#define NE_Y (54)

#define CST_X (77)
#define CST_Y (60)

#define CST_START (77)
#define CST_SPC (41)


// CP_Switch() menu labels
enum sw_labels
{
	SW_LIGHTING,
	SW_REBA_ATTACK_INFO,
	SW_CEILING,
	SW_FLOORS,

	// BBi
	SW_NO_WALL_HIT_SOUND,
	SW_ALWAYS_RUN,
	SW_QUIT_ON_ESCAPE,
	SW_HEART_BEAT_SOUND,
	SW_ROTATED_AUTOMAP,
}; // sw_labels

// BBi
enum sw2_labels
{
	SW2_NO_INTRO_OUTRO,
	SW2_NO_FADE_IN_OR_OUT,
	SW2_NO_WEAPON_BOBBING,
	SW2_VANILLA_FIZZLE_FX,
	SW2_PS_MAP_IN_STATS,
}; // sw2_labels

enum MenuVideoLables
{
	mvl_widescreen,
	mvl_menu_crt,

#ifdef BSTONE_ENABLE_AI_WEAPONS
	mvl_ai_weapons,
#endif
};
#ifdef BSTONE_ENABLE_AI_WEAPONS
constexpr int video_menu_item_count = 3;
#else
constexpr int video_menu_item_count = 2;
#endif
// BBi


void draw_carousel(
	const int item_index,
	CP_iteminfo* item_i,
	CP_itemtype* items,
	const std::string& text);


namespace
{

std::int16_t COAL_FONT()
{
	return STARTFONT + 4;
}

} // namespace


bool is_full_menu_active = false;


//
// VARIABLES
//
extern std::int16_t SaveGamesAvail[10];
extern std::int16_t StartGame;
extern std::int16_t SoundStatus;


void CA_CacheScreen(
	std::int16_t chunk);

void DrawHighScores();
void ClearMemory();

void DrawTopInfo(
	sp_type type);

void PreloadUpdate(
	std::uint16_t current,
	std::uint16_t total);

bool LoadTheGame(
	const std::string& file_name);

bool IN_CheckAck();

void MenuFadeIn();

void MenuFadeOut();

void SetupControlPanel();

void CleanupControlPanel();

void ControlPanelFree();

void ControlPanelAlloc();

void DrawMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items);

std::int16_t HandleMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	void (*routine)(std::int16_t w));

void DrawWindow(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t wcolor);

void DrawOutline(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t color1,
	std::int16_t color2);

void WaitKeyUp();

void TicDelay(
	std::int16_t count);

void CheckPause();

void ShootSnd();

void DrawGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t* y,
	std::int16_t which,
	std::int16_t basey,
	void(*routine)(std::int16_t w));

void EraseGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t y,
	std::int16_t which);

void SetTextColor(
	CP_itemtype* items,
	std::int16_t hlight);

void DrawMenuGun(
	CP_iteminfo* iteminfo);

void DrawMainMenu();
void DrawSoundMenu();

void DrawLoadSaveScreen(
	std::int16_t loadsave);

void DrawNewEpisode();

void DrawNewGame();



void DrawLSAction(
	std::int16_t which);

void PrintLSEntry(
	std::int16_t w,
	std::int16_t color);

void TrackWhichGame(
	std::int16_t w);

void DrawNewGameDiff(
	std::int16_t w);

void CP_BlakeStoneSaga(
	std::int16_t temp1);

void CP_NewGame(
	std::int16_t temp1);

void CP_Sound(
	std::int16_t temp1);

std::int16_t CP_LoadGame(
	std::int16_t quick);

std::int16_t CP_SaveGame(
	std::int16_t quick);

void CP_Control(
	std::int16_t temp1);

void CP_ExitOptions(
	std::int16_t temp1);

void CP_Quit();

std::int16_t CP_EndGame();

bool CP_CheckQuick(
	ScanCode scancode);



//
// End game message
//

char EndGameStr[] = {"    End current game?\n"
" Are you sure (Y or N)?"};


#define ENDGAMESTR (EndGameStr)

char QuitToDosStr[] = {"      Quit to DOS?\n"
" Are you sure (Y or N)?"};

#define FREEFONT(fontnum) { if (fontnum != STARTFONT + 2 && !grsegs[fontnum].empty()) { UNCACHEGRCHUNK(fontnum); } }


static const char* const CURGAME =
"   Continuing past this\n"
"  point will end the game\n"
" you're currently playing.\n"
"\n"
" Start a NEW game? (Y/N)";

static const char* const GAMESVD =
"There's already a game\n"
"saved at this position.\n"
"\n"
"    Overwrite? (Y/N)";


bool EscPressed = false;

std::int16_t lastmenumusic;

std::int16_t MENUSONG = 0;
std::int16_t ROSTER_MUS = 0;
std::int16_t TEXTSONG = 0;


// ===========================================================================
//
// PRIVATE PROTOTYPES
//
// ===========================================================================

void CP_ReadThis(
	std::int16_t temp1);

void CP_OrderingInfo(
	std::int16_t temp1);

void DrawEpisodePic(
	std::int16_t w);

void DrawAllSoundLights(
	std::int16_t which);

void ReadGameNames();
void FreeMusic();

void CP_GameOptions(
	std::int16_t temp1);

void DrawGopMenu();
void ExitGame();

void CP_Switches(
	std::int16_t temp1);

void cp_switches2(
	std::int16_t);

void DrawSwitchMenu();

void DrawAllSwitchLights(
	std::int16_t which);

void DrawSwitchDescription(
	std::int16_t which);

// BBi
void cp_sound_volume(
	std::int16_t);

void cp_video(
	std::int16_t);
// BBi


extern bool refresh_screen;


// ===========================================================================
//
// LOCAL DATA...
//
// ===========================================================================

CP_iteminfo MainItems = {MENU_X, MENU_Y, 12, MM_NEW_MISSION, 0, 9, {77, 1, 154, 9, 1}};
CP_iteminfo GopItems = {MENU_X, MENU_Y + 25, 6, 0, 0, 9, {77, 1, 154, 9, 1}};
CP_iteminfo SndItems = {SM_X, SM_Y, 4, 0, 0, 8, {87, -1, 144, 7, 1}};
CP_iteminfo LSItems = {LSM_X, LSM_Y, 10, 0, 0, 8, {86, -1, 144, 8, 1}};
CP_iteminfo CtlItems = {CTL_X, CTL_Y, 2, 0, 0, 9, {87, 1, 174, 9, 1}};
CP_iteminfo CusItems = {CST_X, CST_Y + 7, 6, -1, 0, 15, {54, -1, 203, 7, 1}};
CP_iteminfo NewEitems = {NE_X, NE_Y, 6, 0, 0, 16, {43, -2, 119, 16, 1}};
CP_iteminfo NewItems = {NM_X, NM_Y, 4, 1, 0, 16, {60, -2, 105, 16, 1}};
CP_iteminfo SwitchItems = {MENU_X, 0, 0, 0, 0, 9, {87, -1, 132, 7, 1}};

// BBi
CP_iteminfo video_items = {MENU_X, MENU_Y + 30, video_menu_item_count, 0, 0, 9, {77, -1, 154, 7, 1}};
CP_iteminfo switches2_items = {MENU_X, MENU_Y + 30, 5, 0, 0, 9, {87, -1, 132, 7, 1}};
// BBi


CP_itemtype MainMenu[] = {
	{AT_ENABLED, "NEW MISSION", CP_NewGame, static_cast<std::uint8_t>(COAL_FONT())},
	{AT_READIT, "ORDERING INFO", CP_OrderingInfo},
	{AT_READIT, "INSTRUCTIONS", CP_ReadThis},
	{AT_ENABLED, "STORY", CP_BlakeStoneSaga},
	{AT_DISABLED, "", nullptr},
	{AT_ENABLED, "GAME OPTIONS", CP_GameOptions},
	{AT_ENABLED, "HIGH SCORES", CP_ViewScores},
	{AT_ENABLED, "LOAD MISSION", [](std::int16_t arg) { CP_LoadGame(arg); }},
	{AT_DISABLED, "SAVE MISSION", [](std::int16_t arg) { CP_SaveGame(arg); }},
	{AT_DISABLED, "", nullptr},
	{AT_ENABLED, "BACK TO DEMO", CP_ExitOptions},
	{AT_ENABLED, "LOGOFF", nullptr}
};

CP_itemtype GopMenu[] = {
	// BBi
	{AT_ENABLED, "VIDEO", cp_video},
	// BBi

	{AT_ENABLED, "SOUND", CP_Sound},

	// BBi
	{AT_ENABLED, "SOUND VOLUME", cp_sound_volume},
	// BBi

	{AT_ENABLED, "CONTROLS", CP_Control},
	{AT_ENABLED, "SWITCHES", CP_Switches},

	// BBi
	{AT_ENABLED, "SWITCHES2", cp_switches2},
	// BBi
};

CP_itemtype SndMenu[] =
{
	{AT_ENABLED, "SOUND EFFECTS", 0},
	{AT_ENABLED, "SFX TYPE", 0},
	{AT_ENABLED, "DIGITIZED SFX", 0},
	{AT_ENABLED, "BACKGROUND MUSIC", 0},
};

CP_itemtype CtlMenu[] = {
    {AT_ENABLED, "CONTROL SCHEME", nullptr},
    {AT_ENABLED, "VIEW CONTROLS", nullptr},
};

CP_itemtype SwitchMenu[] = {
	{AT_ENABLED, "LIGHTING", 0},
	{AT_ENABLED, "REBA ATTACK INFO", 0},
	{AT_ENABLED, "TEXTURED CEILING", 0},
	{AT_ENABLED, "TEXTURED FLOOR", 0},

	// BBi
	{AT_ENABLED, "NO WALL HIT SOUND", 0},
	{AT_ENABLED, "ALWAYS RUN", 0},
	{AT_ENABLED, "QUIT ON ESCAPE", 0},
	{AT_ENABLED, "HEART BEAT SOUND", 0},
	{AT_ENABLED, "STATS MAP", 0},
};

CP_itemtype switch2_menu[] =
{
	{AT_ENABLED, "SKIP TITLE / EXIT PAGES", 0},
	{AT_ENABLED, "SKIP GAME FADES", 0},
	{AT_ENABLED, "NO WEAPON BOBBING", 0},
	{AT_ENABLED, "VANILLA FIZZLE FX", 0},
	{AT_ENABLED, "[PS] MAP IN STATS", 0},
};

CP_itemtype NewEmenu[] = {
	{AT_ENABLED, "MISSION 1:\n"
	"STAR INSTITUTE", 0},

	{AT_NON_SELECTABLE, "MISSION 2:\n"
	"FLOATING FORTRESS", 0},

	{AT_NON_SELECTABLE, "MISSION 3:\n"
	"UNDERGROUND NETWORK", 0},

	{AT_NON_SELECTABLE, "MISSION 4:\n"
	"STAR PORT", 0},

	{AT_NON_SELECTABLE, "MISSION 5:\n"
	"HABITAT II", 0},

	{AT_NON_SELECTABLE, "MISSION 6:\n"
	"SATELLITE DEFENSE", 0},
};

CP_itemtype NewMenu[] = {
	{AT_ENABLED, "LEVEL 1:\n"
	"NOVICE AGENT", 0},
	{AT_ENABLED, "LEVEL 2:\n"
	"SKILLED AGENT", 0},
	{AT_ENABLED, "LEVEL 3:\n"
	"EXPERT AGENT", 0},
	{AT_ENABLED, "LEVEL 4:\n"
	"VETERAN AGENT", 0}
};

CP_itemtype LSMenu[] = {
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0}
};

CP_itemtype CusMenu[] = {
	{AT_ENABLED, "", 0},
	{AT_DISABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_DISABLED, "", 0},
	{AT_ENABLED, "", 0},
	{AT_ENABLED, "", 0}
};

// BBi

CP_itemtype video_menu[] =
{
	{AT_ENABLED, "VIEW ASPECT", nullptr},
	{AT_ENABLED, "MENU CRT", nullptr},

#ifdef BSTONE_ENABLE_AI_WEAPONS
	{AT_ENABLED, "AI ENHANCED WEAPON DESIGNS", nullptr},
#endif
};
// BBi


std::int16_t color_hlite[] = {
	HIGHLIGHT_DISABLED_COLOR,
	HIGHLIGHT_TEXT_COLOR,
	READHCOLOR,
	HIGHLIGHT_DEACTIAVED_COLOR,
};

std::int16_t color_norml[] = {
	DISABLED_TEXT_COLOR,
	ENABLED_TEXT_COLOR,
	READCOLOR,
	DEACTIAVED_TEXT_COLOR,
};

std::int16_t EpisodeSelect[6] = {
	1,
	0,
	0,
	0,
	0,
	0,
};

std::int16_t SaveGamesAvail[10];
std::int16_t StartGame;
std::int16_t SoundStatus = 1;
std::int16_t pickquick;
char SaveGameNames[10][GAME_DESCRIPTION_LEN + 1];

static std::uint8_t menu_background_color = 0x00;


static const std::string& get_saved_game_base_name()
try {
	static auto base_name = std::string();
	static auto is_initialized = false;

	if (!is_initialized)
	{
		is_initialized = true;

		const auto& assets_info = get_assets_info();

		base_name = "bstone_";

		if (assets_info.is_aog_sw())
		{
			base_name += "aog_sw";
		}
		else if (assets_info.is_aog_full())
		{
			base_name += "aog_full";
		}
		else if (assets_info.is_ps())
		{
			base_name += "ps";
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid game type.");
		}

		base_name += "_saved_game_";
	}

	return base_name;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

////////////////////////////////////////////////////////////////////
//
// INPUT MANAGER SCANCODE TABLES
//
////////////////////////////////////////////////////////////////////

#ifndef CACHE_KEY_DATA

using ScanCodes = std::vector<ScanCode>;
using ScanNames = std::vector<std::string>;

// Scan code names with single chars
static ScanNames scan_names = {
	"?", "?", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "?", "?",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "|", "?", "A", "S",
	"D", "F", "G", "H", "J", "K", "L", ";", "\"", "?", "?", "?", "Z", "X", "C", "V",
	"B", "N", "M", ",", ".", "/", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "\xF", "?", "-", "\x15", "5", "\x11", "+", "?",
	"\x13", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
}; // scan_names

// Scan codes with >1 char names
static ScanCodes ext_scan_codes = {
	ScanCode::sc_escape,
	ScanCode::sc_backspace,
	ScanCode::sc_tab,
	ScanCode::sc_control,
	ScanCode::sc_left_shift,
	ScanCode::sc_space,
	ScanCode::sc_caps_lock,
	ScanCode::sc_f1,
	ScanCode::sc_f2,
	ScanCode::sc_f3,
	ScanCode::sc_f4,
	ScanCode::sc_f5,
	ScanCode::sc_f6,
	ScanCode::sc_f7,
	ScanCode::sc_f8,
	ScanCode::sc_f9,
	ScanCode::sc_f10,
	ScanCode::sc_f11,
	ScanCode::sc_f12,
	ScanCode::sc_scroll_lock,
	ScanCode::sc_return,
	ScanCode::sc_right_shift,
	ScanCode::sc_print_screen,
	ScanCode::sc_alt,
	ScanCode::sc_home,
	ScanCode::sc_page_up,
	ScanCode::sc_end,
	ScanCode::sc_page_down,
	ScanCode::sc_insert,
	ScanCode::sc_delete,
	ScanCode::sc_num_lock,
	ScanCode::sc_up_arrow,
	ScanCode::sc_down_arrow,
	ScanCode::sc_left_arrow,
	ScanCode::sc_right_arrow,
	ScanCode::sc_none,
}; // ExtScanCodes

static ScanNames ext_scan_names = {
	"ESC", "BKSP", "TAB", "CTRL", "LSHFT", "SPACE", "CAPSLK", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
	"F10", "F11", "F12", "SCRLK", "ENTER", "RSHFT", "PRTSC", "ALT",
	"HOME", "PGUP", "END", "PGDN", "INS", "DEL", "NUMLK", "UP",
	"DOWN", "LEFT", "RIGHT", "",
}; // ext_scan_names

#else

std::uint8_t* ScanNames, *ExtScanNames;
ScanCode* ExtScanCodes;

#endif

using SpecialKeys = std::vector<ScanCode>;

static SpecialKeys special_keys = {
	ScanCode::sc_back_quote,
	ScanCode::sc_equals,
	ScanCode::sc_minus,
	ScanCode::sc_l,
	ScanCode::sc_p,
	ScanCode::sc_m,
	ScanCode::sc_s,
	ScanCode::sc_i,
	ScanCode::sc_q,
	ScanCode::sc_w,
	ScanCode::sc_e,
	ScanCode::sc_return,
	ScanCode::sc_1,
	ScanCode::sc_2,
	ScanCode::sc_3,
	ScanCode::sc_4,
	ScanCode::sc_5,
	ScanCode::sc_tab,
}; // special_keys


// BBi
namespace
{


void menu_play_sound(
	int sound_index)
{
	sd_play_ui_sound(sound_index);
}

void menu_play_esc_pressed_sound()
{
	menu_play_sound(ESCPRESSEDSND);
}

void menu_play_no_way_sound()
{
	menu_play_sound(NOWAYSND);
}

void menu_play_move_gun_1_sound()
{
	menu_play_sound(MOVEGUN1SND);
}


} // namespace
// BBi

void HelpScreens()
{
	HelpPresenter(nullptr, false, HELPTEXT, true);
}

void HelpPresenter(
	const char* fname,
	bool continue_keys,
	std::uint16_t id_cache,
	bool startmusic,
    const char* section)
{
#if defined(__3DS__) || defined(_3DS)
    const bool owns_terminal = !bstone::n3ds_menu::active();
    if (owns_terminal) { bstone::n3ds_menu::begin(get_assets_info().is_ps(), ingame); }
    const auto terminal_guard = bstone::make_scope_exit([&]()
    {
        if (owns_terminal) { bstone::n3ds_menu::end(); }
    });
    const char* heading = section ? section : id_cache == HELPTEXT ? "INSTRUCTIONS" :
        id_cache == SAGATEXT ? "HISTORY" : id_cache == ORDERTEXT ? "ORDERING INFORMATION" :
        "MISSION BRIEFING";
    bstone::n3ds_menu::show_document(heading, continue_keys);
#endif
	const int FULL_VIEW_WIDTH = 19;

	PresenterInfo pi;
	int oldwidth;

	pi = PresenterInfo{};

	pi.flags = TPF_SHOW_PAGES;
	if (continue_keys)
	{
		pi.flags |= TPF_CONTINUE;
	}

	VW_FadeOut();

	// Change view size to MAX! (scaler clips shapes on smaller views)
	//
	oldwidth = viewwidth / 16;
	if (oldwidth != FULL_VIEW_WIDTH)
	{
		NewViewSize();
	}

#if !defined(__3DS__) && !defined(_3DS)
	// Draw help border
	//
	CacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);
	VWB_DrawPic(0, 0, H_TOPWINDOWPIC);
	VWB_DrawPic(0, 8, H_LEFTWINDOWPIC);
	VWB_DrawPic(312, 8, H_RIGHTWINDOWPIC);
	VWB_DrawPic(8, 176, H_BOTTOMINFOPIC);
	UnCacheLump(H_TOPWINDOWPIC, H_BOTTOMINFOPIC);

#endif

	// Setup for text presenter
	//
	pi.xl = 8;
	pi.yl = 8;
	pi.xh = 311;
	pi.yh = 175;
	pi.ltcolor = 0x7b;
	pi.bgcolor = 0x7d;
	pi.dkcolor = 0x7f;
	pi.shcolor = 0x00;
	pi.fontnumber = 4;

	if (continue_keys)
	{
		pi.infoline = (char*)" UP / DN - PAGES       ENTER - CONTINUES         ESC - EXITS";
	}
	else
	{
		pi.infoline = (char*)"           UP / DN - PAGES            ESC - EXITS";
	}

	if (startmusic)
	{
		StartCPMusic(static_cast<std::int16_t>(TEXTSONG));
	}

	// Only the control guide is replacement content. Narrative scripts
    // remain byte-for-byte the original resources and use the same presenter.
#if defined(__3DS__) || defined(_3DS)
    pi.infoline = nullptr;
    std::vector<char> classic_script;
    if (id_cache == HELPTEXT)
    {
        const auto scheme = in_get_control_scheme();
        const std::string gameplay = scheme == InControlScheme::classic ?
            "CIRCLE PAD / D-PAD:\r"
            "UP / DOWN: FORWARD / BACK\r"
            "LEFT / RIGHT: TURN\r"
            "L / R: STRAFE LEFT / RIGHT\r"
            "A: FIRE\r"
            "B: USE / OPEN DOORS\r"
            "ZL / ZR: NEXT / PREVIOUS WEAPON\r" :
            std::string{
            "CIRCLE PAD UP / DOWN: FORWARD / BACK\r"
            "CIRCLE PAD LEFT / RIGHT: STRAFE\r"
            "Y / A: TURN LEFT / RIGHT\r"
            "B: USE / OPEN DOORS\r"
            "R: FIRE\r"
            "D-PAD RIGHT / LEFT: NEXT / PREV WEAPON\r"} +
            (scheme == InControlScheme::modern_cstick ? "C-STICK: TURN LEFT / RIGHT\r" : "");
        const std::string manual = std::string{"^ST0^CE"} + in_control_scheme_name(scheme) + " CONTROLS\r\r" + gameplay +
            "X: TOGGLE AUTOMAP\r"
            "START: PAUSE / OPEN TERMINAL\r"
            "^EP^ST0^CETERMINAL AND AUTOMAP\r\r"
            "D-PAD: SELECT MENU OPTIONS\r"
            "A: SELECT / CONFIRM\r"
            "B: BACK / CANCEL\r"
            "LEFT / RIGHT: CHANGE VALUES\r"
            "IN DOCUMENTS: UP / DOWN SCROLL\r"
            "LEFT / RIGHT: PREVIOUS / NEXT PAGE\r"
            "THE AUTOMAP, MESSAGES AND EKG APPEAR\r"
            "ON THE LOWER SCREEN DURING PLAY.\r"
            "RUN SPEED USES THE ALWAYS RUN SWITCH.\r"
            "START + SELECT: EMERGENCY EXIT\r"
            "^EP^XX";
        classic_script.assign(manual.begin(), manual.end());
        classic_script.push_back(0);
        pi.scriptstart = classic_script.data();
        pi.script[0] = classic_script.data();
        TP_InitScript(&pi);
    }
    else
#endif
    {
        TP_LoadScript(fname, &pi, id_cache);
    }
    TP_Presenter(&pi);
#if defined(__3DS__) || defined(_3DS)
    if (id_cache != HELPTEXT)
#endif
    { TP_FreeScript(&pi, id_cache); }
#if defined(__3DS__) || defined(_3DS)
    else { TP_FreeScript(&pi, 0); }
#endif

	MenuFadeOut();

	// Reset view size
	//
	if (oldwidth != FULL_VIEW_WIDTH)
	{
		NewViewSize();
	}

	if (startmusic && TPscan == ScanCode::sc_escape)
	{
		StartCPMusic(MENUSONG);
	}

	IN_ClearKeysDown();
}

void US_ControlPanel(
	ScanCode scancode)
{
	is_full_menu_active = (scancode != ScanCode::sc_f7 && scancode != ScanCode::sc_f10);
	sd_pause_scene_sfx(true);

	const auto guard_flag = bstone::make_scope_exit(
		[&]()
		{
#if defined(__3DS__) || defined(_3DS)
            bstone::n3ds_menu::end();
#endif
			is_full_menu_active = false;
			sd_pause_scene_sfx(false);
		});

	// BBi
	const auto& assets_info = get_assets_info();

#if defined(__3DS__) || defined(_3DS)
    bstone::n3ds_menu::begin(assets_info.is_ps(), ingame);
#endif

	menu_background_color = (
		(assets_info.is_aog_sw_v3_0() || assets_info.is_aog_full_v3_0()) ?
		0x04 :
		TERM_BACK_COLOR);


	std::int16_t which;

	if (ingame)
	{
		if (CP_CheckQuick(scancode))
		{
			return;
		}
	}

	SetupControlPanel();

	StartCPMusic(MENUSONG);

	//
	// F-KEYS FROM WITHIN GAME
	//
	auto finish = false;

	switch (scancode)
	{
	case ScanCode::sc_f1:
		CleanupControlPanel();
		HelpScreens();
		return;

	case ScanCode::sc_f2:
		CP_SaveGame(0);
		finish = true;
		break;

	case ScanCode::sc_f3:
		CP_LoadGame(0);
		finish = true;
		break;

	case ScanCode::sc_f4:
		CP_Sound(0);
		finish = true;
		break;

	case ScanCode::sc_f6:
		CP_Control(0);
		finish = true;
		break;

	default:
		break;
	}

	if (finish)
	{
		CleanupControlPanel();
		return;
	}

	DrawMainMenu();
	MenuFadeIn();
	StartGame = 0;

	//
	// MAIN MENU LOOP
	//
	do
	{
		which = HandleMenu(&MainItems, &MainMenu[0], nullptr);

		switch (which)
		{
		case MM_VIEW_SCORES:
			if (!MainMenu[MM_VIEW_SCORES].routine)
			{
				if (CP_EndGame())
				{
					StartGame = 1;
				}
			}

			DrawMainMenu();
			MenuFadeIn();
			break;

		case -1:
			// on hit ESC on main menu
			if (ingame && !gp_quit_on_escape())
			{
				// return to game if quit on escape not enabled
				StartGame = 1;
			}
			else
			{
				CP_Quit();
			}
			break;

		case MM_LOGOFF:
			CP_Quit();
			break;

		default:
			if (!StartGame)
			{
				DrawMainMenu();
				MenuFadeIn();
			}
		}

		//
		// "EXIT OPTIONS" OR "NEW GAME" EXITS
		//
	} while (!StartGame);

	//
	// DEALLOCATE EVERYTHING
	//
	CleanupControlPanel();
	if (!loadedgame)
	{
		StopMusic();
	}


	//
	// CHANGE MAINMENU ITEM
	//
	if (startgame || loadedgame)
	{
		MainMenu[MM_VIEW_SCORES].routine = nullptr;
		MainMenu[MM_VIEW_SCORES].string = "END GAME";
	}

	if (ingame && loadedgame)
	{
		refresh_screen = false;
	}


#ifdef CACHE_KEY_DATA
	FREEFONT(SCANNAMES_DATA);
	FREEFONT(EXTSCANNAMES_DATA);
	FREEFONT(EXTSCANCODES_DATA);
#endif
}

void DrawMainMenu()
{
	ControlPanelFree();
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ControlPanelAlloc();

    // Keep the backing template needed by existing submenus and dialogs, but
    // never write the old main menu title, instructions or options underneath.
	ClearMScreen();
#if !defined(__3DS__) && !defined(_3DS)
	DrawMenuTitle("MAIN OPTIONS");
	DrawInstructions(IT_STANDARD);
#endif

	//
	// CHANGE "MISSION" AND "DEMO"
	//
	auto& back_to_demo_mi = MainMenu[MM_BACK_TO_DEMO];

	if (ingame)
	{
		back_to_demo_mi.string = "BACK TO MISSION";
		back_to_demo_mi.active = AT_READIT;
	}
	else
	{
		back_to_demo_mi.string.clear();
		back_to_demo_mi.active = AT_DISABLED;
	}

	fontnumber = 4; // COAL

	DrawMenu(&MainItems, &MainMenu[0]);
#if defined(__3DS__) || defined(_3DS)
    bstone::n3ds_menu::show(MainItems, MainMenu, ingame, get_assets_info().is_ps());
#endif

	VW_UpdateScreen();
}

void CP_ReadThis(
	std::int16_t)
{
	ControlPanelFree();
	HelpScreens();
	ControlPanelAlloc();
}

void CP_OrderingInfo(
	std::int16_t)
{
	ControlPanelFree();
	HelpPresenter("", false, ORDERTEXT, true);
	ControlPanelAlloc();
}

void CP_BlakeStoneSaga(
	std::int16_t)
{
	ControlPanelFree();
	HelpPresenter("", false, SAGATEXT, true);
	ControlPanelAlloc();
}

// --------------------------------------------------------------------------
// CP_CheckQuick() - CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
// --------------------------------------------------------------------------
bool CP_CheckQuick(
	ScanCode scancode)
{
	auto string = std::string{};
	string.reserve(100);

	switch (scancode)
	{
		// END GAME
		//
	case ScanCode::sc_f7:
		// BBi
#if 0
		VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

		CA_CacheGrChunk(STARTFONT + 1);

		WindowH = 160;
		if (Confirm(ENDGAMESTR))
		{
			playstate = ex_died;
			pickquick = gamestate.lives = 0;
		}

		WindowH = 200;
		fontnumber = 4;
		return true;

		// QUICKSAVE
		//
	case ScanCode::sc_f8:
		if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick)
		{
			string = "Quick Save will overwrite:\n\"";

			CA_CacheGrChunk(STARTFONT + 1);

			string += SaveGameNames[static_cast<int>(LSItems.curpos)];
			string += "\"?";

			// BBi
#if 0
			VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

			if (Confirm(string.c_str()))
			{
				CA_CacheGrChunk(STARTFONT + 1);
				CP_SaveGame(1);
				fontnumber = 4;
			}
			else
			{
				refresh_screen = false;
			}
		}
		else
		{
			CA_CacheGrChunk(STARTFONT + 1);

			vid_is_hud = true;
			VW_FadeOut();
			vid_is_hud = false;

			StartCPMusic(MENUSONG);

			pickquick = CP_SaveGame(0);

			lasttimecount = TimeCount;
			in_clear_mouse_deltas();
		}

		return true;

		// QUICKLOAD
		//
	case ScanCode::sc_f9:
		if (SaveGamesAvail[static_cast<int>(LSItems.curpos)] && pickquick)
		{
			string = "Quick Load:\n\"";

			CA_CacheGrChunk(STARTFONT + 1);

			string += SaveGameNames[static_cast<int>(LSItems.curpos)];
			string += "\"?";

			// BBi
#if 0
			VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

			if (Confirm(string.c_str()))
			{
				CP_LoadGame(1);
			}
			else
			{
				refresh_screen = false;
				return true;
			}

			fontnumber = 4;
		}
		else
		{
			CA_CacheGrChunk(STARTFONT + 1);

			vid_is_hud = true;
			VW_FadeOut();
			vid_is_hud = false;

			StartCPMusic(MENUSONG);

			pickquick = CP_LoadGame(0);

			lasttimecount = TimeCount;
			in_clear_mouse_deltas();
		}

		if (pickquick)
		{
			refresh_screen = false;
		}
		return true;

		// QUIT
		//
	case ScanCode::sc_f10:
		CA_CacheGrChunk(STARTFONT + 1);

		// BBi
#if 0
		VW_ScreenToScreen(PAGE1START, bufferofs, 320, 160);
#endif

		WindowX = WindowY = 0;
		WindowW = 320;
		WindowH = 160;
		if (Confirm(QuitToDosStr))
		{
			ExitGame();
		}

		refresh_screen = false;
		WindowH = 200;
		fontnumber = 4;

		return true;

	default:
		return false;
	}
}

std::int16_t CP_EndGame()
{
	if (!Confirm(ENDGAMESTR))
	{
		return 0;
	}

	pickquick = gamestate.lives = 0;
	playstate = ex_died;
	InstantQuit = 1;

	return 1;
}

void CP_ViewScores(
	std::int16_t)
{
	fontnumber = 4;

	StartCPMusic(static_cast<std::int16_t>(ROSTER_MUS));

	DrawHighScores();
	VW_UpdateScreen();
	MenuFadeIn();
	fontnumber = 1;

	IN_Ack();

	StartCPMusic(MENUSONG);

	MenuFadeOut();
}

void CP_NewGame(
	std::int16_t)
{
	std::int16_t which, episode = 0;

	DrawMenuTitle("Difficulty Level");
	DrawInstructions(IT_STANDARD);

	const auto& assets_info = get_assets_info();

firstpart:

	if (!assets_info.is_ps())
	{
		DrawNewEpisode();
		do
		{
			which = HandleMenu(&NewEitems, &NewEmenu[0], DrawEpisodePic);
			switch (which)
			{
			case -1:
				MenuFadeOut();
				return;

			default:
				if (!EpisodeSelect[which])
				{
					menu_play_no_way_sound();
					CacheMessage(READTHIS_TEXT);
					IN_ClearKeysDown();
					IN_Ack();
					VL_Bar(35, 69, 250, 62, menu_background_color);
					DrawNewEpisode();
					which = 0;
				}
				else
				{
					episode = which;
					which = 1;
				}
				break;
			}

		} while (!which);

		ShootSnd();
	}
	else
	{
		episode = 0;
	}

	//
	// ALREADY IN A GAME?
	//
	if (!assets_info.is_ps() && ingame)
	{
		if (!Confirm(CURGAME))
		{
			MenuFadeOut();
			return;
		}
	}

secondpart:

	MenuFadeOut();
	if (ingame)
	{
		CA_CacheScreen(BACKGROUND_SCREENPIC);
	}
	DrawNewGame();
	which = HandleMenu(&NewItems, &NewMenu[0], DrawNewGameDiff);

	if (which < 0)
	{
		MenuFadeOut();

		if (!assets_info.is_ps())
		{
			goto firstpart;
		}
		else
		{
			return;
		}
	}

	ShootSnd();
	MenuFadeOut();
	ControlPanelFree();

	if (Breifing(BT_INTRO, episode))
	{
		CA_CacheScreen(BACKGROUND_SCREENPIC);
		ControlPanelAlloc();
		goto secondpart;
	}

	StartGame = 1;
	NewGame(which, episode);

	//
	// CHANGE "READ THIS!" TO NORMAL COLOR
	//
	MainMenu[MM_READ_THIS].active = AT_ENABLED;
}

void DrawMenuTitle(
	const char* title)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::set_header(title);
        fontnumber = 4;
        WindowX = PrintX = 32; WindowY = PrintY = 32;
        WindowW = 244; WindowH = 20;
        return;
    }
#endif
	fontnumber = 3;
	CA_CacheGrChunk(STARTFONT + fontnumber);

	PrintX = WindowX = 32;
	PrintY = WindowY = 32;
	WindowW = 244;
	WindowH = 20;

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(title);

	WindowX = 32 - 1;
	WindowY = 32 - 1;

	SETFONTCOLOR(ENABLED_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(title);

	FREEFONT(STARTFONT + fontnumber);
}

const std::int16_t INSTRUCTIONS_Y_POS = 154 + 10;

// ---------------------------------------------------------------------------
// DrawInstructions() - Draws instructions centered at the bottom of
//      the view screen.
//
// NOTES: Original font number or font color is not maintained.
// ---------------------------------------------------------------------------
void DrawInstructions(
	inst_type Type)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::set_footer(Type == IT_SOUND_VOLUME ?
            "UP/DN SELECT   LEFT/RIGHT VOLUME   B BACK" :
            "A SELECT   B BACK   D-PAD NAVIGATE");
        return;
    }
#endif
	const char* instr[MAX_INSTRUCTIONS] = {
		"UP/DN SELECTS - ENTER CHOOSES - ESC EXITS",
		"PRESS ANY KEY TO CONTINUE",
		"ENTER YOUR NAME AND PRESS ENTER",
		"RT/LF ARROW SELECTS - ENTER CHOOSES",

		// BBi
		"UP/DN SELECTS - LF/RT CHANGES - ESC EXITS",
		"ARROWS SELECTS - ENTER CHOOSES - DEL REMOVES",
		"ESC EXITS"
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = INSTRUCTIONS_Y_POS;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[Type]);
}

void DrawNewEpisode()
{
	ClearMScreen();

	DrawMenuTitle("CHOOSE A MISSION");
	DrawInstructions(IT_STANDARD);

	PrintY = 51;
	WindowX = 58;

	fontnumber = 2; // six point font
	DrawMenu(&NewEitems, &NewEmenu[0]);

	DrawEpisodePic(NewEitems.curpos);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();

}

void DrawNewGame()
{
	ClearMScreen();
	DrawMenuTitle("DIFFICULTY LEVEL");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2; // six point font
	DrawMenu(&NewItems, &NewMenu[0]);

	DrawNewGameDiff(NewItems.curpos);

	px = 48;
	py = INSTRUCTIONS_Y_POS - 24;
	ShPrint("        HIGHER DIFFICULTY LEVELS CONTAIN", TERM_SHADOW_COLOR, false);

	px = 48;
	py += 6;
	ShPrint("            MORE, STRONGER ENEMIES", TERM_SHADOW_COLOR, false);


	VW_UpdateScreen();

	MenuFadeIn();
	WaitKeyUp();
}

void DrawNewGameDiff(
	std::int16_t w)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	VWB_DrawPic(192, 77, w + C_BABYMODEPIC);
}

void DrawEpisodePic(
	std::int16_t w)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	VWB_DrawPic(176, 72, w + C_EPISODE1PIC);
}

void CP_GameOptions(
	std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawGopMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&GopItems, &GopMenu[0], nullptr);

		if (which != -1)
		{
			DrawGopMenu();
			MenuFadeIn();
		}

	} while (which >= 0);

	MenuFadeOut();
}

void DrawGopMenu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME OPTIONS");
	DrawInstructions(IT_STANDARD);

	fontnumber = 4; // COAL

	DrawMenu(&GopItems, &GopMenu[0]);

	VW_UpdateScreen();
}

void ChangeSwaps()
{
	WindowX = WindowY = 0;
	WindowW = 320;
	WindowH = 200;
	Message(Computing);

	ClearMemory();
	ControlPanelAlloc();

	IN_UserInput(50);
	IN_ClearKeysDown();

}

void CP_Switches(
	std::int16_t)
{
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawSwitchMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&SwitchItems, &SwitchMenu[0], DrawAllSwitchLights);

		switch (which)
		{
		case SW_LIGHTING:
			gp_no_shading(!gp_no_shading());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_REBA_ATTACK_INFO:
			gp_hide_attacker_info(!gp_hide_attacker_info());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_CEILING:
			gp_is_ceiling_solid(!gp_is_ceiling_solid());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_FLOORS:
			gp_is_flooring_solid(!gp_is_flooring_solid());
			ShootSnd();
			DrawSwitchMenu();
			break;

		// BBi
		case SW_NO_WALL_HIT_SOUND:
			gp_no_wall_hit_sfx(!gp_no_wall_hit_sfx());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_ALWAYS_RUN:
			gp_is_always_run(!gp_is_always_run());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_QUIT_ON_ESCAPE:
			gp_quit_on_escape(!gp_quit_on_escape());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_HEART_BEAT_SOUND:
			gp_use_heart_beat_sfx(!gp_use_heart_beat_sfx());
			ShootSnd();
			DrawSwitchMenu();
			break;

		case SW_ROTATED_AUTOMAP:
			am_rotatable(!am_rotatable());
			ShootSnd();
			DrawSwitchMenu();
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void DrawSwitchMenu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME SWITCHES");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2;

	DrawMenu(&SwitchItems, &SwitchMenu[0]);
	DrawAllSwitchLights(SwitchItems.curpos);

	VW_UpdateScreen();
}

void DrawAllSwitchLights(
	std::int16_t which)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	std::int16_t i;
	std::uint16_t Shape;

	for (i = 0; i < SwitchItems.amount; i++)
	{
		if (SwitchMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (SwitchItems.cursor.on)
			{
				if (i == which)
				{ // Is the cursor sitting on this pic?
					Shape += 2;
				}
			}

			switch (i)
			{
			case SW_LIGHTING:
				if (!gp_no_shading())
				{
					Shape++;
				}
				break;

			case SW_REBA_ATTACK_INFO:
				if (!gp_hide_attacker_info())
				{
					Shape++;
				}
				break;

			case SW_CEILING:
				if (!gp_is_ceiling_solid())
				{
					Shape++;
				}
				break;

			case SW_FLOORS:
				if (!gp_is_flooring_solid())
				{
					Shape++;
				}
				break;

			// BBi
			case SW_NO_WALL_HIT_SOUND:
				if (gp_no_wall_hit_sfx())
				{
					++Shape;
				}
				break;

			case SW_ALWAYS_RUN:
				if (gp_is_always_run())
				{
					++Shape;
				}
				break;

			case SW_QUIT_ON_ESCAPE:
				if (gp_quit_on_escape())
				{
					++Shape;
				}
				break;

			case SW_HEART_BEAT_SOUND:
				if (gp_use_heart_beat_sfx())
				{
					++Shape;
				}
				break;

			case SW_ROTATED_AUTOMAP:
				if (am_rotatable())
				{
					++Shape;
				}
				break;
			}

			VWB_DrawPic(SwitchItems.x - 16, SwitchItems.y + i * SwitchItems.y_spacing - 1, Shape);
		}
	}

	DrawSwitchDescription(which);

}

void DrawSwitchDescription(
	std::int16_t which)
{
	const char* instr[] = {
		"TOGGLES LIGHT SOURCING IN HALLWAYS",
		"TOGGLES DETAILED ATTACKER INFO",
		"TOGGLES CEILING MAPPING",
		"TOGGLES FLOOR MAPPING",

		// BBi
		"TOGGLES WALL HIT SOUND",
		"TOGGLES ALWAYS RUN MODE",
		"ESC QUITS INSTEAD OF RETURNING TO GAME",
		"TOGGLES HEART BEAT SOUND WITH EKG",
		"TOGGLES <TAB>/<SHIFT+TAB> FUNCTIONS",
	};

	const auto& assets_info = get_assets_info();

	fontnumber = 2;

	WindowX = 48;
	WindowY = (assets_info.is_ps() ? 134 : 144);
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);
}

void digitized_sfx_carousel(
	int item_index,
	bool,
	bool)
{
	const auto old_sfx_type = sd_cfg_get_sfx_type();

	auto new_sfx_type = AudioSfxType{};

	switch (old_sfx_type)
	{
		case AudioSfxType::adlib:
			new_sfx_type = AudioSfxType::pc_speaker;
			break;

		case AudioSfxType::pc_speaker:
		default:
			new_sfx_type = AudioSfxType::adlib;
			break;
	}

	sd_cfg_set_sfx_type(new_sfx_type);
	sd_apply_sfx_type();

	DrawSoundMenu();
	DrawAllSoundLights(static_cast<std::int16_t>(item_index));

	TicDelay(20);
}

const char* get_sfx_type_string(
	AudioSfxType sfx_type)
{
	switch (sfx_type)
	{
		case AudioSfxType::pc_speaker:
			return "PC SPEAKER";

		case AudioSfxType::adlib:
		default:
			return "ADLIB";
	}
}

void CP_Sound(
	std::int16_t)
{
    SndMenu[1].carousel_func_ = digitized_sfx_carousel;

	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawSoundMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&SndItems, &SndMenu[0], DrawAllSoundLights);
		//
		// HANDLE MENU CHOICES
		//
		switch (which)
		{
			//
			// SOUND EFFECTS / DIGITIZED SOUND
			//
		case 0:
			sd_wait_sound_done();
			sd_enable_sound(!sd_is_sound_enabled());

			DrawSoundMenu();

			if (sd_is_sound_enabled())
			{
				ShootSnd();
			}

			break;

		case 2:
			sd_wait_sound_done();
			sd_cfg_set_is_sfx_digitized(!sd_cfg_get_is_sfx_digitized());
			apply_digitized_sfx();
			DrawSoundMenu();
			ShootSnd();
			break;

		case 3:
			sd_enable_music(!sd_is_music_enabled());

			if (sd_is_music_enabled())
			{
				StartCPMusic(MENUSONG);
			}

			DrawSoundMenu();
			ShootSnd();

			break;

		default:
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void DrawSoundMenu()
{
	//
	// DRAW SOUND MENU
	//

	ClearMScreen();
	DrawMenuTitle("SOUND SETTINGS");
	DrawInstructions(IT_STANDARD);

	//
	// IF NO ADLIB, NON-CHOOSENESS!
	//

	if (!sd_has_audio())
	{
		SndMenu[0].active = AT_DISABLED;
		SndMenu[3].active = AT_DISABLED;
	}

	fontnumber = 2;
	DrawMenu(&SndItems, &SndMenu[0]);

	DrawAllSoundLights(SndItems.curpos);

	VW_UpdateScreen();
}

void DrawAllSoundLights(
	std::int16_t which)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	std::int16_t i;
	std::uint16_t Shape;

	for (i = 0; i < SndItems.amount; i++)
	{
		if (SndMenu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (SndItems.cursor.on)
			{
				if (i == which)
				{ // Is the cursor sitting on this pic?
					Shape += 2;
				}
			}

			switch (i)
			{
			case 0:
				if (sd_is_sound_enabled())
				{
					++Shape;
				}
				break;

			case 1:
				draw_carousel(
					i,
					&SndItems,
					SndMenu,
					get_sfx_type_string(sd_cfg_get_sfx_type())
				);
				continue;

			case 2:
				if (sd_cfg_get_is_sfx_digitized())
				{
					++Shape;
				}
				break;

			case 3:
				if (sd_is_music_enabled())
				{
					++Shape;
				}
				break;

			default:
				continue;
			}

			VWB_DrawPic(SndItems.x - 16, SndItems.y + i * SndItems.y_spacing - 1, Shape);
		}
	}
}

char LOADSAVE_GAME_MSG[2][25] = {
	"^ST1^CELoading Game\r^XX",
	"^ST1^CESaving Game\r^XX"
};

// --------------------------------------------------------------------------
// DrawLSAction() - DRAW LOAD/SAVE IN PROGRESS
// --------------------------------------------------------------------------
void DrawLSAction(
	std::int16_t which)
{
#if defined(__3DS__) || defined(_3DS)
    // The real save-game reader owns the new loading presentation. Avoid an
    // extra legacy progress page just before its first frame. Saving is unchanged.
    if (which == 0) { return; }
    if (bstone::n3ds_menu::active())
    {
        Message(which ? "SAVING GAME..." : "LOADING GAME...");
        bstone::n3ds_menu::set_header(which ? "SAVE GAME" : "LOAD GAME");
        bstone::n3ds_menu::set_footer("PLEASE WAIT");
        VW_UpdateScreen();
        return;
    }
#endif
	std::int8_t total[] = {19, 19};

	VW_FadeOut();
	screenfaded = true;
	DrawTopInfo(static_cast<sp_type>(sp_loading + which));

	VL_Bar(
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
		BLACK);

	DisplayPrepingMsg(LOADSAVE_GAME_MSG[which]);

	if (which)
	{
		PreloadUpdate(1, 1); // GFX: bar is full when saving...

	}
	WindowY = 181;
}

std::int16_t CP_LoadGame(
	std::int16_t quick)
{
	std::int16_t which;
	std::int16_t exit = 0;

	//
	// QUICKLOAD?
	//
	if (quick)
	{
		which = LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			DrawLSAction(0); // Testing...

			auto name_path = get_profile_read_path(name);

			loadedgame = LoadTheGame(name_path);
			return loadedgame;
		}
	}

restart:

	DrawLoadSaveScreen(0);

	do
	{
		which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);

		if (which >= 0 && SaveGamesAvail[which])
		{
			ShootSnd();

			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			auto name_path = get_profile_read_path(name);

			DrawLSAction(0);

			if (!LoadTheGame(name_path))
			{
				exit = 0;
				StartGame = 0;
				loadedgame = 0;
				playstate = ex_abort;
				goto restart;
			}

			loadedgame = true;
			StartGame = true;

			ShootSnd();

			//
			// CHANGE "READ THIS!" TO NORMAL COLOR
			//
			MainMenu[MM_READ_THIS].active = AT_ENABLED;
			exit = 1;
			break;
		}
	} while (which >= 0);

	if (which == -1)
	{
		MenuFadeOut();
	}

	if (loadedgame)
	{
		refresh_screen = false;
	}

	return exit;
}


///////////////////////////////////
//
// HIGHLIGHT CURRENT SELECTED ENTRY
//
void TrackWhichGame(
	std::int16_t w)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	static std::int16_t lastgameon = 0;

	PrintLSEntry(lastgameon, ENABLED_TEXT_COLOR);
	PrintLSEntry(w, HIGHLIGHT_TEXT_COLOR);

	lastgameon = w;
}

void DrawLoadSaveScreen(
	std::int16_t loadsave)
{
	std::int16_t i;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();

	fontnumber = 1;

	if (!loadsave)
	{
		DrawMenuTitle("Load Mission");
	}
	else
	{
		DrawMenuTitle("Save Mission");
	}

	DrawInstructions(IT_STANDARD);

	for (i = 0; i < 10; i++)
	{
		PrintLSEntry(i, ENABLED_TEXT_COLOR);
	}

	fontnumber = 4;
	DrawMenu(&LSItems, &LSMenu[0]);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}

// --------------------------------------------------------------------------
// PRINT LOAD/SAVE GAME ENTRY W/BOX OUTLINE
// --------------------------------------------------------------------------
void PrintLSEntry(
	std::int16_t w,
	std::int16_t color)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	SETFONTCOLOR(color, BKGDCOLOR);
	DrawOutline(LSM_X + LSItems.indent, LSM_Y + w * LSItems.y_spacing - 2, LSM_W - LSItems.indent, 8, color, color);

	fontnumber = 2;

	PrintX = LSM_X + LSItems.indent + 2;
	PrintY = LSM_Y + w * LSItems.y_spacing;

	if (SaveGamesAvail[w])
	{
		US_Print(SaveGameNames[w]);
	}
	else
	{
		US_Print("       ----- EMPTY -----");
	}

	fontnumber = 1;
}

std::int16_t CP_SaveGame(
	std::int16_t quick)
{
	std::int16_t which, exit = 0;
	char input[GAME_DESCRIPTION_LEN + 1];
	bool temp_caps = allcaps;
	US_CursorStruct TermCursor = {'@', 0, HIGHLIGHT_TEXT_COLOR, 2};

	allcaps = true;
	use_custom_cursor = true;
	US_CustomCursor = TermCursor;

	//
	// QUICKSAVE?
	//
	if (quick)
	{
		which = LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			DrawLSAction(1); // Testing...
			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			auto name_path = get_profile_dir() + name;

			SaveTheGame(name_path, &SaveGameNames[which][0]);

			return 1;
		}
	}

	DrawLoadSaveScreen(1);

	do
	{
		which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);
		if (which >= 0)
		{
			//
			// OVERWRITE EXISTING SAVEGAME?
			//
			if (SaveGamesAvail[which])
			{
				if (!Confirm(GAMESVD))
				{
					DrawLoadSaveScreen(1);
					continue;
				}
				else
				{
					DrawLoadSaveScreen(1);
					PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
					VW_UpdateScreen();
				}
			}

			ShootSnd();

			strcpy(input, &SaveGameNames[which][0]);

			auto name = get_saved_game_base_name();
			name += static_cast<char>('0' + which);

			fontnumber = 2;
			VWB_Bar(LSM_X + LSItems.indent + 1, LSM_Y + which * LSItems.y_spacing - 1, LSM_W - LSItems.indent - 1, 7, HIGHLIGHT_BOX_COLOR);
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, HIGHLIGHT_BOX_COLOR);
			VW_UpdateScreen();

			#if defined(__3DS__) || defined(_3DS)

						// Generar nombre de la partida automáticamente.
						const auto now = std::time(nullptr);
						const auto* date = std::localtime(&now);

						if (date)
						{
							std::snprintf(
								input,
								sizeof(input),
								"E%d N%02d %02d/%02d/%02d %02d:%02d",
								gamestate.episode + 1,
								gamestate.mapon,
								date->tm_mday,
								date->tm_mon + 1,
								(date->tm_year + 1900) % 100,
								date->tm_hour,
								date->tm_min
							);
						}
						else
						{
							std::snprintf(
								input,
								sizeof(input),
								"E%d N%02d",
								gamestate.episode + 1,
								gamestate.mapon
							);
						}

						const bool confirmed = true;

			#else

						const bool confirmed = US_LineInput(
							LSM_X + LSItems.indent + 2,
							LSM_Y + which * LSItems.y_spacing,
							input,
							input,
							true,
							GAME_DESCRIPTION_LEN,
							LSM_W - LSItems.indent - 10
						);

			#endif

            if (confirmed)
			{
				SaveGamesAvail[which] = 1;
				strcpy(&SaveGameNames[which][0], input);

				DrawLSAction(1);

				auto name_path = get_profile_dir() + name;
				SaveTheGame(name_path, input);

				ShootSnd();
				exit = 1;
			}
			else
			{
				VWB_Bar(
					LSM_X + LSItems.indent + 1,
					LSM_Y + which * LSItems.y_spacing - 1,
					LSM_W - LSItems.indent - 1,
					7,
					menu_background_color);

				PrintLSEntry(which, HIGHLIGHT_TEXT_COLOR);
				VW_UpdateScreen();
				menu_play_esc_pressed_sound();
				continue;
			}

			fontnumber = 1;
			break;
		}

	} while (which >= 0);

	MenuFadeOut();
	use_custom_cursor = false;
	allcaps = temp_caps;
	return exit;
}

void CP_ExitOptions(
	std::int16_t)
{
	StartGame = 1;
}

void CP_Control(std::int16_t)
{
    std::int16_t which;
    do
    {
        ClearMScreen();
        DrawMenuTitle("CONTROL SETTINGS");
        DrawInstructions(IT_STANDARD);
        DrawMenu(&CtlItems, CtlMenu);
        VW_UpdateScreen();
        MenuFadeIn();
        WaitKeyUp();
        which = HandleMenu(&CtlItems, CtlMenu, nullptr);
        if (which == 0)
        {
            in_set_control_scheme(static_cast<InControlScheme>(
                (static_cast<int>(in_get_control_scheme()) + 1) % static_cast<int>(InControlScheme::count)));
            ShootSnd();
        }
        else if (which == 1)
        {
            HelpPresenter(nullptr, false, HELPTEXT, false, "CONTROL GUIDE");
        }
    } while (which >= 0);
    MenuFadeOut();
}

void CP_Quit()
{
	if (Confirm(QuitToDosStr))
	{
		ExitGame();
	}

	DrawMainMenu();
}

// ---------------------------------------------------------------------------
// Clear Menu screens to dark red
// ---------------------------------------------------------------------------
void ClearMScreen()
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { bstone::n3ds_menu::clear_content(); return; }
#endif
	VWB_Bar(SCREEN_X, SCREEN_Y, SCREEN_W, SCREEN_H, menu_background_color);
}

// ---------------------------------------------------------------------------
// Un/Cache a LUMP of graphics
// ---------------------------------------------------------------------------
void CacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend)
{
	std::int16_t i;

	for (i = lumpstart; i <= lumpend; i++)
	{
		CA_CacheGrChunk(i);
	}
}

void UnCacheLump(
	std::int16_t lumpstart,
	std::int16_t lumpend)
{
	std::int16_t i;

	for (i = lumpstart; i <= lumpend; i++)
	{
		FREEFONT(i);
	}
}

void DrawWindow(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t wcolor)
{
	VWB_Bar(x, y, w, h, static_cast<std::uint8_t>(wcolor));
	DrawOutline(x, y, w, h, BORD2COLOR, DEACTIVE);
}

void DrawOutline(
	std::int16_t x,
	std::int16_t y,
	std::int16_t w,
	std::int16_t h,
	std::int16_t color1,
	std::int16_t color2)
{
	VWB_Hlin(x, x + w, y, static_cast<std::uint8_t>(color2));
	VWB_Vlin(y, y + h, x, static_cast<std::uint8_t>(color2));
	VWB_Hlin(x, x + w, y + h, static_cast<std::uint8_t>(color1));
	VWB_Vlin(y, y + h, x + w, static_cast<std::uint8_t>(color1));
}

void SetupControlPanel()
{
	const auto& assets_info = get_assets_info();

	// BBi
	SwitchItems.amount = (assets_info.is_ps() ? 7 : 9);
	SwitchItems.y = MENU_Y + (assets_info.is_ps() ? 11 : 3);
#if defined(__3DS__) || defined(_3DS)
    switch2_menu[SW2_NO_WEAPON_BOBBING].active = AT_ENABLED;
    switch2_menu[SW2_PS_MAP_IN_STATS].active = assets_info.is_ps() ? AT_ENABLED : AT_DISABLED;
#endif
	// BBi

	ControlPanelAlloc();

	fontnumber = 2;

	WindowH = 200;

	if (ingame)
	{
		MainMenu[MM_SAVE_MISSION].active = AT_ENABLED;
	}

	ReadGameNames();
}

void ReadGameNames()
{
	for (int i = 0; i < 10; ++i)
	{
		auto name = get_saved_game_base_name();
		name += static_cast<char>('0' + i);

		auto name_path = get_profile_read_path(name);

		bstone::FileStream stream{};

		if (!stream.open(name_path.c_str(), bstone::file_flags_shared))
		{
			continue;
		}

		SaveGamesAvail[i] = 1;

		int chunk_size = bstone::sg_find_chunk(bstone::SgKnownFourCc::desc(), stream);

		if (chunk_size > 0)
		{
			char temp[GAME_DESCRIPTION_LEN + 1];

			std::fill_n(
				temp,
				GAME_DESCRIPTION_LEN,
				'\0');

			auto temp_size = std::min(GAME_DESCRIPTION_LEN, chunk_size);

			stream.read(temp, temp_size);

			strcpy(&SaveGameNames[i][0], temp);
		}
		else
		{
			strcpy(&SaveGameNames[i][0], "DESCRIPTION LOST");
		}
	}
}

void CleanupControlPanel()
{
#if defined(__3DS__) || defined(_3DS)
    bstone::n3ds_menu::hide();
#endif
	if (!loadedgame)
	{
		FreeMusic();
	}
	ControlPanelFree();
	fontnumber = 4;
}

// ---------------------------------------------------------------------------
// ControlPanelFree() - This FREES the control panel lump from memory
//      and REALLOCS the ScaledDirectory
// ---------------------------------------------------------------------------
void ControlPanelFree()
{
	UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
	NewViewSize();
}

// ---------------------------------------------------------------------------
// ControlPanelAlloc() - This CACHEs the control panel lump into memory
//      and FREEs the ScaledDirectory.
// ---------------------------------------------------------------------------
void ControlPanelAlloc()
{
	CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
}

// ---------------------------------------------------------------------------
// ShadowPrint() - Shadow Prints given text @ a given x & y in default font
//
// NOTE: Font MUST already be loaded
// ---------------------------------------------------------------------------
void ShadowPrint(
	const char* strng,
	std::int16_t x,
	std::int16_t y)
{
	std::int16_t old_bc, old_fc;

	old_fc = fontcolor;
	old_bc = backcolor;

	PrintX = x + 1;
	PrintY = y + 1;

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_Print(strng);

	PrintX = x;
	PrintY = y;
	SETFONTCOLOR(old_fc, old_bc);
	US_Print(strng);
}

// ---------------------------------------------------------------------------
// HandleMenu() - Handle moving gun around a menu
// ---------------------------------------------------------------------------
std::int16_t HandleMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	void(*routine)(std::int16_t w))
{
#define box_on item_i->cursor.on
#if defined(__3DS__) || defined(_3DS)
    // A/B are physical held states, not keyboard edges. A completed action
    // may immediately re-enter HandleMenu (switches) or its parent (B). Wait
    // for that press to end before accepting the next one. WaitKeyUp pumps
    // input and the terminal's paced idle; there is no timed debounce, no
    // gameplay binding change, and D-pad hold/repeat remains unchanged.
    WaitKeyUp();
#endif
	std::int8_t key;
	static std::int16_t redrawitem = 1;

	std::int16_t i, x, y, basey, exit, which, flash_tics;
	ControlInfo ci;

	which = item_i->curpos;
	x = item_i->x;
	basey = item_i->y;
	y = basey + which * item_i->y_spacing;
	box_on = 1;
	DrawGun(item_i, items, x, &y, which, basey, routine);

	SetTextColor(items + which, 1);

	if (redrawitem
#if defined(__3DS__) || defined(_3DS)
        && !bstone::n3ds_menu::active()
#endif
    )
	{
		ShadowPrint(
			(items + which)->string.c_str(),
			item_i->x + item_i->indent,
			item_i->y + which * item_i->y_spacing);
	}

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
	{
		routine(which);
	}

	VW_UpdateScreen();

	flash_tics = 40;
	exit = 0;
	TimeCount = 0;
	IN_ClearKeysDown();

	do
	{
		CalcTics();
		flash_tics -= tics;

		CycleColors();

		//
		// CHANGE GUN SHAPE
		//

		if (flash_tics <= 0)
		{
			flash_tics = 40;

			box_on ^= 1;

			if (box_on)
			{
				DrawGun(item_i, items, x, &y, which, basey, routine);
			}
			else
			{
				EraseGun(item_i, items, x, y, which);
				if (routine)
				{
					routine(which);
				}
			}


			VW_UpdateScreen();
		}

		CheckPause();


		//
		// SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
		//

		key = LastASCII;
		if (key)
		{
			std::int16_t ok = 0;

			if (key >= 'a')
			{
				key -= 'a' - 'A';
			}

			for (i = which + 1; i < item_i->amount; i++)
			{
				if ((items + i)->active && (items + i)->string[0] == key)
				{
					EraseGun(item_i, items, x, y, which);
					which = i;
					item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing
					box_on = 1;
					DrawGun(item_i, items, x, &y, which, basey, routine);
					VW_UpdateScreen();

					ok = 1;
					IN_ClearKeysDown();
					break;
				}
			}

			//
			// DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
			//

			if (!ok)
			{
				for (i = 0; i < which; i++)
				{
					if ((items + i)->active && (items + i)->string[0] == key)
					{
						EraseGun(item_i, items, x, y, which);
						which = i;
						item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing
						box_on = 1;
						DrawGun(item_i, items, x, &y, which, basey, routine);
						VW_UpdateScreen();

						IN_ClearKeysDown();
						break;
					}
				}
			}
		}

		//
		// GET INPUT
		//

		ReadAnyControl(&ci);

		switch (ci.dir)
		{
			// ------------------------
			// MOVE UP
			//
		case dir_North:
			EraseGun(item_i, items, x, y, which);

			do
			{
				if (!which)
				{
					which = item_i->amount - 1;
				}
				else
				{
					which--;
				}

			} while (!(items + which)->active);

			item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing

			box_on = 1;
			DrawGun(item_i, items, x, &y, which, basey, routine);

			VW_UpdateScreen();

			TicDelay(20);
			break;

			// --------------------------
			// MOVE DOWN
			//
		case dir_South:
			EraseGun(item_i, items, x, y, which);

			do
			{
				if (which == item_i->amount - 1)
				{
					which = 0;
				}
				else
				{
					which++;
				}
			} while (!(items + which)->active);

			item_i->curpos = static_cast<std::int8_t>(which); // jtr -testing

			box_on = 1;
			DrawGun(item_i, items, x, &y, which, basey, routine);

			VW_UpdateScreen();

			TicDelay(20);
			break;

			// Carousel (left).
			//
			case dir_West:
			{
				auto carousel_func = items[which].carousel_func_;

				if (carousel_func)
				{
					carousel_func(which, true, false);
				}

				break;
			}

			// Carousel (right).
			//
			case dir_East:
			{
				auto carousel_func = items[which].carousel_func_;

				if (carousel_func)
				{
					carousel_func(which, false, true);
				}

				break;
			}

		default:
			break;
		}

		if (ci.button0 || Keyboard[ScanCode::sc_space] || Keyboard[ScanCode::sc_return])
		{
			exit = 1;
		}

		if (ci.button1 || Keyboard[ScanCode::sc_escape])
		{
			exit = 2;
		}

	} while (!exit);

	IN_ClearKeysDown();

	//
	// ERASE EVERYTHING
	//

	box_on = 0;
	redrawitem = 1;
	EraseGun(item_i, items, x, y, which);

	if (routine)
	{
		routine(which);
	}

	VW_UpdateScreen();

	item_i->curpos = static_cast<std::int8_t>(which);
#if defined(__3DS__) || defined(_3DS)
    // Keep the native menu over palette-only fade frames. The destination
    // releases it when its finished content reaches VW_UpdateScreen.
    if (item_i == &MainItems) { bstone::n3ds_menu::leave_on_next_update(); }
#endif

	switch (exit)
	{
	case 1:
		//
		// CALL THE ROUTINE
		//
		if ((items + which)->routine)
		{
			// Make sure there's room to save when CP_SaveGame() is called.
			//
			if (reinterpret_cast<std::size_t>(items[which].routine) == reinterpret_cast<std::size_t>(CP_SaveGame))
			{
				if (!CheckDiskSpace(DISK_SPACE_NEEDED, CANT_SAVE_GAME_TXT, cds_menu_print))
				{
					return which;
				}
			}

			//
			// ALREADY IN A GAME?
			//
			const auto& assets_info = get_assets_info();

			if (assets_info.is_ps() && ingame && ((items + which)->routine == CP_NewGame))
			{
				if (!Confirm(CURGAME))
				{
					MenuFadeOut();
					return 0;
				}
			}

			ShootSnd();
			MenuFadeOut();
			(items + which)->routine(0);
		}
		return which;

	case 2:
		menu_play_esc_pressed_sound();
		return -1;
	}

	return 0;     // JUST TO SHUT UP THE ERROR MESSAGES!
}

// ---------------------------------------------------------------------------
// ERASE GUN & DE-HIGHLIGHT STRING
// ---------------------------------------------------------------------------
void EraseGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t y,
	std::int16_t which)
{
	static_cast<void>(x);
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif

	VWB_Bar(item_i->cursor.x, y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, menu_background_color);
	SetTextColor(items + which, 0);

	ShadowPrint((items + which)->string.c_str(), item_i->x + item_i->indent, y);
}

// ---------------------------------------------------------------------------
// DrawGun() - DRAW GUN AT NEW POSITION
// ---------------------------------------------------------------------------
void DrawGun(
	CP_iteminfo* item_i,
	CP_itemtype* items,
	std::int16_t x,
	std::int16_t* y,
	std::int16_t which,
	std::int16_t basey,
	void(*routine)(std::int16_t w))
{
	static_cast<void>(x);

#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active())
    {
        *y = basey + which * item_i->y_spacing;
        bstone::n3ds_menu::select(which);
        if (routine) { routine(which); }
        return;
    }
#endif
	*y = basey + which * item_i->y_spacing;

	VWB_Bar(item_i->cursor.x, *y + item_i->cursor.y_ofs, item_i->cursor.width, item_i->cursor.height, HIGHLIGHT_BOX_COLOR);
	SetTextColor(items + which, 1);

	ShadowPrint(
		(items + which)->string.c_str(),
		item_i->x + item_i->indent,
		item_i->y + which * item_i->y_spacing);

	//
	// CALL CUSTOM ROUTINE IF IT IS NEEDED
	//

	if (routine)
	{
		routine(which);
	}
}

// ---------------------------------------------------------------------------
// TicDelay() - DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
// ---------------------------------------------------------------------------
void TicDelay(
	std::int16_t count)
{
	ControlInfo ci;

	TimeCount = 0;

	do
	{
		ReadAnyControl(&ci);
#if defined(__3DS__) || defined(_3DS)
        bstone::n3ds_menu::idle();
#endif
	} while (TimeCount < count && ci.dir != dir_None);
}

// ---------------------------------------------------------------------------
// DrawMenu() - Draw a menu
//
//       This also calculates the Y position of the current items in the
//                      CP_itemtype structures.
// ---------------------------------------------------------------------------
void DrawMenu(
	CP_iteminfo* item_i,
	CP_itemtype* items)
{
	std::int16_t i, which = item_i->curpos;

	WindowX = PrintX = item_i->x + item_i->indent;
	WindowY = PrintY = item_i->y;

	WindowW = 320;
	WindowH = 200;
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::show_list(*item_i, items);
        return;
    }
#endif

	for (i = 0; i < item_i->amount; i++)
	{
		SetTextColor(items + i, which == i);

		ShadowPrint(
			(items + i)->string.c_str(),
			static_cast<std::int16_t>(WindowX),
			item_i->y + i * item_i->y_spacing);
	}
}

// ---------------------------------------------------------------------------
// SET TEXT COLOR (HIGHLIGHT OR NO)
// ---------------------------------------------------------------------------
void SetTextColor(
	CP_itemtype* items,
	std::int16_t hlight)
{
	if (hlight)
	{
		SETFONTCOLOR(color_hlite[items->active], TERM_BACK_COLOR);
	}
	else
	{
		SETFONTCOLOR(color_norml[items->active], TERM_BACK_COLOR);
	}
}

// ---------------------------------------------------------------------------
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
// ---------------------------------------------------------------------------
void WaitKeyUp()
{
	for (auto quit = false; !quit; )
	{
		ControlInfo ci;

		ReadAnyControl(&ci);

		quit = !(
			ci.button0 != 0 ||
			ci.button1 != 0 ||
			ci.button2 != 0 ||
			ci.button3 != 0 ||
			Keyboard[ScanCode::sc_space] ||
			Keyboard[ScanCode::sc_return] ||
			Keyboard[ScanCode::sc_escape]);
#if defined(__3DS__) || defined(_3DS)
        if (!quit) { bstone::n3ds_menu::idle(); }
#endif
	}
}

// ---------------------------------------------------------------------------
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
// ---------------------------------------------------------------------------
void ReadAnyControl(
	ControlInfo* ci)
{
	IN_ReadControl(0, ci);

	//
	// UNDO some of the ControlInfo vars that were init
	// with IN_ReadControl() for the mouse...
	//
	if (ControlTypeUsed == ctrl_Mouse)
	{
		//
		// Clear directions & buttons (if enabled or not)
		//
		ci->dir = dir_None;
		ci->button0 = 0;
		ci->button1 = 0;
		ci->button2 = 0;
		ci->button3 = 0;
	}


	#if defined(__3DS__) || defined(_3DS)

		const u32 keys = hidKeysHeld();

		if (keys & KEY_UP)
			ci->dir = dir_North;
		else if (keys & KEY_DOWN)
			ci->dir = dir_South;
		else if (keys & KEY_LEFT)
			ci->dir = dir_West;
		else if (keys & KEY_RIGHT)
			ci->dir = dir_East;

		ci->button0 = (keys & KEY_A) != 0;
		ci->button1 = (keys & KEY_B) != 0;

	#endif
}

////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////

std::int16_t Confirm(const char* string)
{
#if defined(__3DS__) || defined(_3DS)
    const bool owns_terminal = !bstone::n3ds_menu::active();
    if (owns_terminal) { bstone::n3ds_menu::begin(get_assets_info().is_ps(), ingame); }
    const auto terminal_guard = bstone::make_scope_exit([&]()
    { if (owns_terminal) { bstone::n3ds_menu::end(); } });
#endif
    std::int16_t xit = 0;
    std::int16_t x, y;
    std::int16_t tick = 0;
    std::int16_t whichsnd[2] = {ESCPRESSEDSND, SHOOTSND};

#if defined(__3DS__) || defined(_3DS)
    Message(string == QuitToDosStr ? "LOG OFF AND EXIT THE GAME?" : string);
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::set_header("CONFIRMATION");
        bstone::n3ds_menu::set_footer("A CONFIRM   B CANCEL");
        VW_UpdateScreen();
    }
#else
    Message(string);
#endif

    SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
    CA_CacheGrChunk(STARTFONT + fontnumber);

    IN_ClearKeysDown();

#if defined(__3DS__) || defined(_3DS)
    // Guardar el estado inicial para evitar que el
    // botón que abrió el diálogo lo confirme también.
    u32 previous_keys = hidKeysHeld() & (KEY_A | KEY_B);

    bool answer_yes = false;
    bool answer_no = false;
#endif

    x = static_cast<std::int16_t>(PrintX);
    y = static_cast<std::int16_t>(PrintY);

    TimeCount = 0;

    do
    {
#if defined(__3DS__) || defined(_3DS)
        if (bstone::n3ds_menu::active()) { CycleColors(); }
#endif
        if (TimeCount >= 10
#if defined(__3DS__) || defined(_3DS)
            && !bstone::n3ds_menu::active()
#endif
        )
        {
            switch (tick)
            {
                case 0:
                    VWB_Bar(
                        x, y, 8, 13,
                        BORDER_MED_COLOR);
                    break;

                case 1:
                    PrintX = x;
                    PrintY = y;
                    US_Print("_");
                    break;
            }

            VW_UpdateScreen();

            tick ^= 1;
            TimeCount = 0;
        }

        IN_CheckAck();

#if defined(__3DS__) || defined(_3DS)

        const u32 keys = hidKeysHeld() & (KEY_A | KEY_B);
        const u32 pressed = keys & ~previous_keys;

        previous_keys = keys;

        if (pressed & KEY_A)
        {
            answer_yes = true;
        }
        else if (pressed & KEY_B)
        {
            answer_no = true;
        }

#endif

    } while (
#if defined(__3DS__) || defined(_3DS)
        !answer_yes &&
        !answer_no &&
#endif
        !Keyboard[ScanCode::sc_y] &&
        !Keyboard[ScanCode::sc_n] &&
        !Keyboard[ScanCode::sc_escape]
    );

    if (
#if defined(__3DS__) || defined(_3DS)
        answer_yes ||
#endif
        Keyboard[ScanCode::sc_y])
    {
        xit = 1;
        ShootSnd();
    }

    while (
        Keyboard[ScanCode::sc_y] ||
        Keyboard[ScanCode::sc_n] ||
        Keyboard[ScanCode::sc_escape])
    {
        IN_CheckAck();
    }

#if defined(__3DS__) || defined(_3DS)

    // Esperar a que se suelten A y B para que
    // el menú siguiente no reutilice la pulsación.
    while (hidKeysHeld() & (KEY_A | KEY_B))
    {
        IN_CheckAck();
    }

#endif

    IN_ClearKeysDown();

    menu_play_sound(whichsnd[xit]);

    FREEFONT(STARTFONT + fontnumber);

    return xit;
}


// ---------------------------------------------------------------------------
// PRINT A MESSAGE IN A WINDOW
// ---------------------------------------------------------------------------
void Message(
	const char* string)
{
#if defined(__3DS__) || defined(_3DS)
    bstone::n3ds_menu::suspend_loading();
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::clear_content();
        bstone::n3ds_menu::set_header("INFORMATION");
        bstone::n3ds_menu::set_footer("A CONTINUE   B BACK");
        fontnumber = 4;
        CA_CacheGrChunk(STARTFONT + fontnumber);
        const auto* face = reinterpret_cast<const fontstruct*>(grsegs[STARTFONT + fontnumber].data());
        std::vector<std::string> lines;
        std::string line;
        std::string word;
        auto append_word = [&]()
        {
            if (word.empty()) { return; }
            const auto trial = line.empty() ? word : line + " " + word;
            int width = 0, height = 0;
            VW_MeasurePropString(trial.c_str(), &width, &height);
            if (width > 280 && !line.empty()) { lines.push_back(line); line = word; }
            else { line = trial; }
            word.clear();
        };
        for (const char* p = string; ; ++p)
        {
            if (*p == ' ' || *p == '\n' || *p == '\r' || !*p)
            {
                append_word();
                if (*p == '\n' || *p == '\r' || !*p)
                { lines.push_back(line); line.clear(); }
                if (!*p) { break; }
            }
            else { word += *p; }
        }
        const int spacing = face->height + 3;
        // Center in the expanded logical terminal body (y=28..179).
        int y = std::max(28, 104 - int(lines.size()) * spacing / 2);
        SETFONTCOLOR(ENABLED_TEXT_COLOR, TERM_BACK_COLOR);
        for (const auto& value : lines)
        {
            int width = 0, height = 0;
            VW_MeasurePropString(value.c_str(), &width, &height);
            PrintX = WindowX = (320 - width) / 2;
            PrintY = y;
            US_Print(value.c_str());
            y += spacing;
        }
        VW_UpdateScreen();
        return;
    }
#endif
	std::int16_t h = 0, w = 0, mw = 0;
	std::size_t i;
	fontstruct* font;

	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT + 1);

	font = reinterpret_cast<fontstruct*>(grsegs[STARTFONT + fontnumber].data());

	h = font->height;
	for (i = 0; i < strlen(string); i++)
	{
		if (string[i] == '\n')
		{
			if (w > mw)
			{
				mw = w;
			}
			w = 0;
			h += font->height;
		}
		else
		{
			w += font->width[static_cast<int>(string[i])];
		}
	}

	if (w + 10 > mw)
	{
		mw = w + 10;
	}

	const auto OldPrintY = PrintY = (WindowH / 2) - h / 2;
	const auto OldPrintX = PrintX = WindowX = 160 - mw / 2;

	// bump down and to right for shadow

	PrintX++;
	PrintY++;
	WindowX++;

	BevelBox(
		static_cast<std::int16_t>(WindowX - 6),
		static_cast<std::int16_t>(PrintY - 6),
		mw + 10,
		h + 10,
		BORDER_HI_COLOR,
		BORDER_MED_COLOR,
		BORDER_LO_COLOR);

	SETFONTCOLOR(BORDER_LO_COLOR, BORDER_MED_COLOR);
	US_Print(string);

	PrintY = OldPrintY;
	WindowX = PrintX = OldPrintX;

	SETFONTCOLOR(BORDER_TEXT_COLOR, BORDER_MED_COLOR);
	US_Print(string);

	FREEFONT(STARTFONT + 1);

	VW_UpdateScreen();
}

// --------------------------------------------------------------------------
// Searches for an "^XX" and replaces with a 0 (NULL)
// --------------------------------------------------------------------------
void TerminateStr(char* pos)
{
	pos = strstr(pos, "^XX");

	if (pos != nullptr)
	{
		*pos = '\0';
	}
}

// ---------------------------------------------------------------------------
// Caches and prints a message in a window.
// ---------------------------------------------------------------------------
void CacheMessage(
	std::uint16_t MessageNum)
{
	char* string;

	CA_CacheGrChunk(MessageNum);
	string = (char*)grsegs[MessageNum].data();

	TerminateStr(string);

	Message(string);

	FREEFONT(MessageNum);
}

void StartCPMusic(
	std::int16_t song)
{
	int chunk;

	lastmenumusic = song;

	sd_music_off();
	chunk = song;
	sd_start_music(chunk);
}

void FreeMusic()
{
	sd_music_off();
}


#ifdef CACHE_KEY_DATA
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
std::uint8_t far* IN_GetScanName(
	ScanCode scan)
{
	std::uint8_t* p;
	ScanCode* s;

	for (s = ExtScanCodes, p = ExtScanNames; *s; p += 7, s++)
	{
		if (*s == scan)
		{
			return (std::uint8_t*)p;
		}
	}

	return (std::uint8_t*)(ScanNames + (scan << 1));
}
#else
// ---------------------------------------------------------------------------
// IN_GetScanName() - Returns a string containing the name of the
//      specified scan code
// ---------------------------------------------------------------------------
const std::string& IN_GetScanName(
	ScanCode scan)
{
	for (auto i = 0; ext_scan_codes[i] != ScanCode::sc_none; ++i)
	{
		if (ext_scan_codes[i] == scan)
		{
			return ext_scan_names[i];
		}
	}

	return scan_names[static_cast<int>(scan)];
}
#endif

// ---------------------------------------------------------------------------
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
// ---------------------------------------------------------------------------
void CheckPause()
{
	if (Paused)
	{
		switch (SoundStatus)
		{
		case 0:
			sd_music_on(true);
			break;

		case 1:
			sd_music_off();
			break;
		}

		SoundStatus ^= 1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused = false;
	}
}

// -------------------------------------------------------------------------
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
// -------------------------------------------------------------------------
void DrawMenuGun(
	CP_iteminfo* iteminfo)
{
	std::int16_t x, y;

	x = iteminfo->cursor.x;
	y = iteminfo->y + iteminfo->curpos * iteminfo->y_spacing + iteminfo->cursor.y_ofs;

	VWB_Bar(x, y, iteminfo->cursor.width, iteminfo->cursor.height, HIGHLIGHT_BOX_COLOR);
}

void ShootSnd()
{
	sd_play_ui_sound(SHOOTSND);
}

void ShowPromo()
{
	vid_is_movie = true;

	const auto PROMO_MUSIC = HIDINGA_MUS;

	// Load and start music
	//
	sd_start_music(PROMO_MUSIC);

	// Show promo screen 1
	//
	MenuFadeOut();
	CA_CacheScreen(PROMO1PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase * 20);

	// Show promo screen 2
	//
	MenuFadeOut();
	CA_CacheScreen(PROMO2PIC);
	VW_UpdateScreen();
	MenuFadeIn();
	IN_UserInput(TickBase * 20);

	// Music off and freed!
	//
	StopMusic();

	vid_is_movie = false;
}

void ExitGame()
{
	VW_FadeOut();

	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw_v3_0() && !gp_no_intro_outro() && !gp_no_screens())
	{
		ShowPromo();
	}

	sd_music_off();
	sd_stop_sfx_sound();
	Quit();
}

// BBi
int volume_index = 0;

struct Volume
{
	using Get = int (*)();
	Get get;

	using Set = void (*)(int);
	Set set;
};

Volume const volumes[2] = {
	{sd_get_sfx_volume, sd_set_sfx_volume},
	{sd_get_music_volume, sd_set_music_volume}};

void draw_volume_control(
	int index,
	int volume,
	bool is_enabled)
{
	std::int16_t slider_color = is_enabled ? ENABLED_TEXT_COLOR : DISABLED_TEXT_COLOR;

	std::int16_t outline_color = is_enabled ? HIGHLIGHT_TEXT_COLOR : DEACTIAVED_TEXT_COLOR;

	int y = 82 + (index * 40);

	VWB_Bar(74, static_cast<std::int16_t>(y), 160, 8, HIGHLIGHT_BOX_COLOR);

	DrawOutline(
		73,
		static_cast<std::int16_t>(y - 1),
		161,
		9,
		outline_color,
		outline_color
	);

	VWB_Bar(
		static_cast<std::int16_t>(74 + (((160 - 16) * volume) / sd_max_volume)),
		static_cast<std::int16_t>(y),
		16,
		8,
		static_cast<std::uint8_t>(slider_color)
	);
}

void draw_volume_controls()
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active())
    {
        bstone::n3ds_menu::show_meters(volume_index, volumes[0].get(), volumes[1].get(), sd_max_volume);
        return;
    }
#endif
	for (int i = 0; i < 2; ++i)
	{
		draw_volume_control(i, volumes[i].get(), i == volume_index);
	}
}

void cp_sound_volume(
	std::int16_t)
{
	ClearMScreen();
	DrawMenuTitle("SOUND VOLUME");
	DrawInstructions(IT_SOUND_VOLUME);
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { draw_volume_controls(); }
#endif

	fontnumber = 4;

	SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);

	PrintX = 150;
	PrintY = 60;
	US_Print("SFX");

	PrintX = 145;
	PrintY = 105;
	US_Print("MUSIC");

	for (int i = 0; i < 2; ++i)
	{
		PrintX = 36;
		PrintY = static_cast<std::uint16_t>(81 + (i * 40));
		US_Print("MUTE");

		PrintX = 242;
		US_Print("LOUD");
	}

	draw_volume_controls();

	VW_UpdateScreen();
	MenuFadeIn();

	ControlInfo ci;

	int old_volumes[2] = {-1, -1};

	for (bool quit = false; !quit; )
	{
		bool update_volumes = false;
		bool redraw_controls = false;

		ReadAnyControl(&ci);

		switch (ci.dir)
		{
		case dir_North:
			if (volume_index == 1)
			{
				redraw_controls = true;
				volume_index = 0;
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_up_arrow])
			{
				in_handle_events();
#if defined(__3DS__) || defined(_3DS)
                bstone::n3ds_menu::idle();
#endif
			}
			break;

		case dir_South:
			if (volume_index == 0)
			{
				redraw_controls = true;
				volume_index = 1;
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_down_arrow])
			{
				in_handle_events();
#if defined(__3DS__) || defined(_3DS)
                bstone::n3ds_menu::idle();
#endif
			}
			break;

		case dir_West:
			if (volumes[volume_index].get() > sd_min_volume)
			{
				redraw_controls = true;
				update_volumes = true;
				volumes[volume_index].set(volumes[volume_index].get() - 1);
				draw_volume_controls();
				VW_UpdateScreen();
			}

			while (Keyboard[ScanCode::sc_left_arrow])
			{
				in_handle_events();
#if defined(__3DS__) || defined(_3DS)
                bstone::n3ds_menu::idle();
#endif
			}
			break;

		case dir_East:
			if (volumes[volume_index].get() < sd_max_volume)
			{
				redraw_controls = true;
				update_volumes = true;
				volumes[volume_index].set(volumes[volume_index].get() + 1);
			}

			while (Keyboard[ScanCode::sc_right_arrow])
			{
				in_handle_events();
#if defined(__3DS__) || defined(_3DS)
                bstone::n3ds_menu::idle();
#endif
			}
			break;

		default:
			break;
		}

		if (update_volumes)
		{
			update_volumes = false;

			if (old_volumes[0] != volumes[0].get())
			{
				sd_set_sfx_volume();
				menu_play_move_gun_1_sound();
			}

			if (old_volumes[1] != volumes[1].get())
			{
				sd_set_music_volume();
			}
		}

		if (redraw_controls)
		{
			redraw_controls = false;
			draw_volume_controls();
			VW_UpdateScreen();
		}

        CycleColors();
		quit = (ci.button1 || Keyboard[ScanCode::sc_escape]);
	}

	menu_play_esc_pressed_sound();
	WaitKeyUp();
	MenuFadeOut();
}

///
void draw_video_descriptions(
	std::int16_t which)
{
	const char* instructions[] = {
		"5:3 FULL SCREEN OR 4:3 WITH SIDE BARS",
		"TOGGLES TERMINAL CRT ANIMATION",
#ifdef BSTONE_ENABLE_AI_WEAPONS
		"OPTIONAL PACK; MISSING WEAPONS USE ORIGINALS",
#endif
	};

	fontnumber = 2;

	WindowX = 48;
	WindowY = 144;
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(
		WindowX,
		WindowY - 1,
		WindowW,
		WindowH,
		menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);

	--WindowX;
	--WindowY;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instructions[which]);
}

void video_draw_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	ClearMScreen();
	DrawMenuTitle("VIDEO SETTINGS");
	DrawInstructions(IT_STANDARD);
	DrawMenu(&video_items, video_menu);
	VW_UpdateScreen();
}

void video_draw_switch(
	std::int16_t which)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	std::uint16_t Shape;

	for (int i = 0; i < video_items.amount; i++)
	{
		if (video_menu[i].string[0])
		{
			Shape = C_NOTSELECTEDPIC;

			if (video_items.cursor.on)
			{
				if (i == which)
				{
					Shape += 2;
				}
			}

			switch (i)
			{
			case mvl_widescreen:
				if (vid_cfg_is_widescreen())
				{
					Shape++;
				}
				break;

			case mvl_menu_crt:
				if (vid_cfg_menu_crt())
				{
					Shape++;
				}
				break;

			default:
				break;
			}

			VWB_DrawPic(
				video_items.x - 16,
				video_items.y + (i * video_items.y_spacing) - 1,
				Shape);
		}
	}

	draw_video_descriptions(which);
}

///
void draw_carousel(
	const int item_index,
	CP_iteminfo* item_i,
	CP_itemtype* items,
	const std::string& text)
{
	const int which = item_i->curpos;

	int item_text_width;
	int item_text_height;

	VW_MeasurePropString(
		items[item_index].string.c_str(),
		&item_text_width,
		&item_text_height
	);

	int carousel_text_width;
	int carousel_text_height;

	VW_MeasurePropString(
		text.c_str(),
		&carousel_text_width,
		&carousel_text_height
	);

	const auto max_height = item_i->y_spacing;

	const auto arrow_width = 3;
	const auto arrow_height = 5;
	const auto arrow_y = item_i->y + (item_index * max_height) + (max_height / 2) - (arrow_height / 2);

	const auto left_arrow_x = item_i->x + item_i->indent + item_text_width + 3;

	const auto arrow_color = static_cast<std::uint8_t>(color_norml[items->active]);

	VL_Plot(left_arrow_x + 0, arrow_y - 0, arrow_color);
	VL_Vlin(left_arrow_x + 1, arrow_y - 1, 3, arrow_color);
	VL_Vlin(left_arrow_x + 2, arrow_y - 2, 5, arrow_color);

	WindowW = 320;
	WindowH = 200;

	const auto carousel_text_x = left_arrow_x + arrow_width + 2;

	WindowX = static_cast<std::int16_t>(carousel_text_x);
	WindowY = static_cast<std::int16_t>(item_i->y + (item_index * max_height));

	PrintX = WindowX;
	PrintY = WindowY;

	SetTextColor(items + item_index, item_index == which);

	ShadowPrint(text.c_str(), static_cast<std::int16_t>(WindowX), static_cast<std::int16_t>(WindowY));

	const auto right_arrow_x = carousel_text_x + carousel_text_width + 2;

	VL_Vlin(right_arrow_x + 0, arrow_y - 2, 5, arrow_color);
	VL_Vlin(right_arrow_x + 1, arrow_y - 1, 3, arrow_color);
	VL_Plot(right_arrow_x + 2, arrow_y - 0, arrow_color);
}



void cp_video(
	std::int16_t)
{
#if defined(__3DS__) || defined(_3DS)
#ifdef BSTONE_ENABLE_AI_WEAPONS
    bstone::n3ds_weapons::refresh();
    (void)vid_cfg_ai_weapons();
#endif
    video_items.amount = video_menu_item_count;
    video_items.curpos = 0;
#endif
	std::int16_t which;

	CA_CacheScreen(BACKGROUND_SCREENPIC);
	video_draw_menu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which = HandleMenu(&video_items, video_menu, video_draw_switch);

		switch (which)
		{

		case mvl_widescreen:
			vid_cfg_set_is_widescreen(!vid_cfg_is_widescreen());
			ShootSnd();
			video_draw_switch(video_items.curpos);
			vl_update_widescreen();
			SetupWalls();
			NewViewSize();
			SetPlaneViewSize();
			VL_RefreshScreen();
			break;

		case mvl_menu_crt:
			vid_cfg_menu_crt(!vid_cfg_menu_crt());
			ShootSnd();
			video_draw_switch(video_items.curpos);
			VL_RefreshScreen();
			break;

#ifdef BSTONE_ENABLE_AI_WEAPONS
        case mvl_ai_weapons:
        {
            const bool was_enabled = vid_cfg_ai_weapons();
            if (was_enabled) { vid_cfg_ai_weapons(false); }
            else
            {
                bstone::n3ds_weapons::refresh();
                vid_cfg_ai_weapons(true);
            }
            if (!bstone::n3ds_weapons::available())
            {
                const auto message = std::string{"AI ENHANCED PACK UNAVAILABLE.\n"} +
                    bstone::n3ds_weapons::status() + "\n"
                    "INSTALL A VALID PACK FOR THIS GAME\n"
                    "IN /3DS/BSTONE/AI-WEAPONS/.\n"
                    "ORIGINAL WEAPONS REMAIN ACTIVE.";
                Message(message.c_str());
                IN_Ack();
                WaitKeyUp();
                video_draw_menu();
            }
            ShootSnd();
            VL_RefreshScreen();
            break;
        }

#endif

		default:
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}

void draw_switch2_description(
	std::int16_t which)
{
	const char* instr[] =
	{
		"TOGGLES INTRO/OUTRO",
		"TOGGLES FADE IN/OUT EFFECT",
		"TOGGLES WEAPON BOBBING",
		"TOGGLES VANILLA FIZZLE FX",
		"TOGGLES MAP IN STATS (PLANET STRIKE)",
	};

	const auto& assets_info = get_assets_info();

	fontnumber = 2;

	WindowX = 48;
	WindowY = (assets_info.is_ps() ? 134 : 144);
	WindowW = 236;
	WindowH = 8;

	VWB_Bar(WindowX, WindowY - 1, WindowW, WindowH, menu_background_color);

	SETFONTCOLOR(TERM_SHADOW_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);

	WindowX--;
	WindowY--;

	SETFONTCOLOR(INSTRUCTIONS_TEXT_COLOR, TERM_BACK_COLOR);
	US_PrintCentered(instr[which]);
}

void draw_all_switch2_lights(
	std::int16_t which)
{
#if defined(__3DS__) || defined(_3DS)
    if (bstone::n3ds_menu::active()) { return; }
#endif
	std::uint16_t shape;

	for (auto i = 0; i < switches2_items.amount; ++i)
	{
		if (switch2_menu[i].string[0])
		{
			shape = C_NOTSELECTEDPIC;

			//
			// DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
			//

			if (switches2_items.cursor.on)
			{
				// Is the cursor sitting on this pic?
				if (i == which)
				{
					shape += 2;
				}
			}

			switch (i)
			{
			case SW2_NO_INTRO_OUTRO:
				if (gp_no_intro_outro())
				{
					shape += 1;
				}
				break;

			case SW2_NO_FADE_IN_OR_OUT:
				if (gp_no_fade_in_or_out())
				{
					shape += 1;
				}
				break;

			case SW2_NO_WEAPON_BOBBING:
				if (gp_no_weapon_bobbing())
				{
					shape += 1;
				}
				break;

			case SW2_VANILLA_FIZZLE_FX:
				if (gp_vanilla_fizzle_fx())
				{
					shape += 1;
				}
				break;

			case SW2_PS_MAP_IN_STATS:
				if (gp_ps_map_in_stats())
				{
					shape += 1;
				}
				break;
			}

			VWB_DrawPic(switches2_items.x - 16, switches2_items.y + (i * switches2_items.y_spacing) - 1, shape);
		}
	}

	draw_switch2_description(which);
}

void draw_switch2_menu()
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);

	ClearMScreen();
	DrawMenuTitle("GAME SWITCHES 2");
	DrawInstructions(IT_STANDARD);

	fontnumber = 2;

	DrawMenu(&switches2_items, &switch2_menu[0]);
	draw_all_switch2_lights(switches2_items.curpos);

	VW_UpdateScreen();
}

void cp_switches2(
	std::int16_t)
{
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	draw_switch2_menu();
	MenuFadeIn();
	WaitKeyUp();

	auto which = 0;

	do
	{
		which = HandleMenu(&switches2_items, &switch2_menu[0], draw_all_switch2_lights);

		switch (which)
		{
		case SW2_NO_INTRO_OUTRO:
			gp_no_intro_outro(!gp_no_intro_outro());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_NO_FADE_IN_OR_OUT:
			gp_no_fade_in_or_out(!gp_no_fade_in_or_out());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_NO_WEAPON_BOBBING:
			gp_no_weapon_bobbing(!gp_no_weapon_bobbing());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_VANILLA_FIZZLE_FX:
			gp_vanilla_fizzle_fx(!gp_vanilla_fizzle_fx());
			ShootSnd();
			draw_switch2_menu();
			break;

		case SW2_PS_MAP_IN_STATS:
			gp_ps_map_in_stats(!gp_ps_map_in_stats());
			ShootSnd();
			draw_switch2_menu();
			break;
		}
	} while (which >= 0);

	MenuFadeOut();
}
///

///
void MenuFadeOut()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 44, 0, 0, 10);
	}
	else
	{
		VL_FadeOut(0, 255, 40, 44, 44, 10);
	}
}

void MenuFadeIn()
{
	VL_FadeIn(0, 255, vgapal, 10);
}

void menu_enable_all_episodes()
{
	for (auto i = 1; i < 6; ++i)
	{
		NewEmenu[i].active = AT_ENABLED;
		EpisodeSelect[i] = 1;
	}
}
// BBi

std::string cp_get_terminal_label(const CP_iteminfo& info, const CP_itemtype* items, int index)
{
    if (&info == &LSItems)
    {
        return std::to_string(index + 1) + "  " + (SaveGamesAvail[index] ?
            std::string(SaveGameNames[index]) : std::string("-- EMPTY --"));
    }
    auto label = items[index].string;
    std::replace(label.begin(), label.end(), '\n', ' ');
    if (label.empty()) { return label; }
    bool value = false;
    bool toggle = false;
    if (&info == &SwitchItems)
    {
        // This legacy CVar selects the full/local X stats map. It does not
        // rotate the permanent native lower-screen map.
        if (index == SW_ROTATED_AUTOMAP)
        { return "STATS MAP: " + std::string(am_rotatable() ? "LOCAL" : "FULL"); }
        const bool states[] = {!gp_no_shading(), !gp_hide_attacker_info(),
            !gp_is_ceiling_solid(), !gp_is_flooring_solid(), gp_no_wall_hit_sfx(),
            gp_is_always_run(), gp_quit_on_escape(), gp_use_heart_beat_sfx(), am_rotatable()};
        value = states[index]; toggle = true;
    }
    else if (&info == &switches2_items)
    {
        if (index == SW2_PS_MAP_IN_STATS && get_assets_info().is_aog()) { return {}; }
        const bool states[] = {gp_no_intro_outro(), gp_no_fade_in_or_out(),
            gp_no_weapon_bobbing(), gp_vanilla_fizzle_fx(), gp_ps_map_in_stats()};
        value = states[index]; toggle = true;
    }
    else if (&info == &SndItems)
    {
        if (index == 1) { return label + ": < " + get_sfx_type_string(sd_cfg_get_sfx_type()) + " >"; }
        value = index == 0 ? sd_is_sound_enabled() : index == 2 ?
            sd_cfg_get_is_sfx_digitized() : sd_is_music_enabled();
        toggle = true;
    }
    else if (&info == &CtlItems)
    { return index == 0 ? std::string{"CONTROL SCHEME: "} + in_control_scheme_name(in_get_control_scheme()) : "BUTTON GUIDE"; }
    else if (&info == &video_items)
    {
#ifdef BSTONE_ENABLE_AI_WEAPONS
        if (index == mvl_ai_weapons)
        {
            return "AI ENHANCED WEAPON DESIGNS: " + std::string(vid_cfg_ai_weapons() ? "ON" :
                bstone::n3ds_weapons::available() ? "OFF" : "OFF [NO PACK]");
        }
#endif
        if (index == mvl_menu_crt) { return "MENU CRT: " + std::string(vid_cfg_menu_crt() ? "ON" : "OFF"); }
        return "VIEW ASPECT: " + std::string(vid_cfg_is_widescreen() ? "5:3 NATIVE" : "4:3");
    }
    else if (&info == &NewEitems && !EpisodeSelect[index]) { label += " [LOCKED]"; }
    if (toggle) { label += value ? ": ON" : ": OFF"; }
    return label;
}
