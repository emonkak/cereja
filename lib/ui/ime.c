/* Extension: ui.ime - ime-related API  {{{1
 *
 * __COPYRIGHT_NOTICE__
 * $Id$
 */

#include "cereja.h"
#include <windows.h>
#include <imm.h>



/* Misc.  {{{1 */



/* Public Functions  {{{1 */

static int
ui_ime_get_state(lua_State* L)
{
HWND hwnd;
	HIMC himc;
	DWORD conversion;
	DWORD sentence;

	hwnd = GetForegroundWindow();
	himc = ImmGetContext(hwnd);
	ImmGetConversionStatus(himc, &conversion, &sentence);
	Crj_BuildValues(L, "{Qll}",
					ImmGetOpenStatus(himc),
					conversion,
					sentence);

	char text[256];
	sprintf(text, "[%p] [%d] [%lx] [%lx]",
			(void*)hwnd,
			ImmGetOpenStatus(himc),
			conversion,
			sentence
			);
	MessageBox(NULL, text, "cereja", MB_OK);

	ImmReleaseContext(hwnd, himc);

	return 1;
}


static int
ui_ime_set_ime(lua_State* L)
{
	HWND hwnd;
	HIMC himc;
	BOOL is_open;

	Crj_ParseArgs(L, "| Q", &is_open);

	hwnd = GetForegroundWindow();
	himc = ImmGetContext(hwnd);
	ImmSetOpenStatus(himc, is_open);

	ImmReleaseContext(hwnd, himc);

	return 0;
}


/*
static int
ui_ime_toggle_ime(lua_State* L)
{
	HWND hwnd;
	HIMC himc;

	hwnd = GetFocus();
	himc = ImmGetContext(GetFocus());
	ImmSetOpenStatus(himc, !ImmGetOpenStatus(himc));

	ImmReleaseContext(hwnd, himc);

	return 0;
}
*/


static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"get_state", ui_ime_get_state},
	{"set_ime", ui_ime_set_ime},
	//{"toggle_ime", ui_ime_toggle_ime},
	{NULL, NULL}
};



/* Main  {{{1 */

Crj_PUBLIC(int)
Crj_ExtMain(lua_State* L)
{
	CrjExtEvent event;
	HMODULE hmodule;
	Crj_ParseArgs(L, "{=si =su}",
	              "event", &event,
	              "hmodule", &hmodule);

	switch (event) {
	default:
		luaL_error(L, "Event %d is unknown", event);
		break;
	case Crj_EXT_LOADING:
		lua_getfield(L, 1, "namespace");
		luaL_register(L, NULL, PUBLIC_FUNCTIONS);
		lua_pop(L, 1);
		break;
	case Crj_EXT_UNLOADING:
		break;
	}

	return 0;
}

/* __END__ {{{1 */
/* vim: foldmethod=marker
 */
