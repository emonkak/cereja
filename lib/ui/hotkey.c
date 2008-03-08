/* Extension: ui.hotkey - hotkey feature (C part) ======================== {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"




#define REGKEY_ACTION_TABLE "cereja:ui.hotkey:action_table"
#define HOTKEY_ID_MIN 0x0000
#define HOTKEY_ID_MAX 0xBFFF

static HWND s_HandlerWindow;
static WPARAM s_HotkeyCount;  /* used as new hotkey id */








/* Handler Window ===================================================== {{{1 */

#define HANDLER_WINDOW_CLASS TEXT("cereja.ui.hotkey")
#define HANDLER_WINDOW_TITLE NULL

static lua_State* s_L;




static LRESULT CALLBACK
handler_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	default:
		break;

	case WM_HOTKEY:
		if (wp < s_HotkeyCount) {
			int n;

			n = lua_gettop(s_L);
			lua_getfield(s_L, LUA_REGISTRYINDEX,
			             REGKEY_ACTION_TABLE);
			lua_pushinteger(s_L, wp);
			lua_gettable(s_L, -2);
			if (Crj_PCall(s_L, 0, 0) != 0) {
				Crj_NoticeError(s_L, "ui.hotkey",
				                lua_tostring(s_L, -1));
			}
			lua_settop(s_L, n);
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}




static void
handler_window_startup(lua_State* L, HMODULE hmodule)
{
	WNDCLASS wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = handler_window_proc;
	wc.hInstance = hmodule;
	wc.lpszClassName = HANDLER_WINDOW_CLASS;
	if (!RegisterClass(&wc)) {
		lua_pushstring(L, "RegisterClass is failed");
		goto E_RegisterClass;
	}

	s_HandlerWindow = CreateWindowEx(0,
	                                 HANDLER_WINDOW_CLASS,
	                                 HANDLER_WINDOW_TITLE,
	                                 0,
	                                 0, 0, 0, 0,
	                                 HWND_MESSAGE,
	                                 NULL,
	                                 hmodule,
	                                 NULL);
	if (s_HandlerWindow == NULL) {
		lua_pushstring(L, "CreateWindowEx is failed");
		goto E_CreateWindowEx;
	}

	s_L = L;
	s_HotkeyCount = HOTKEY_ID_MIN;
	return;

E_CreateWindowEx:
	UnregisterClass(HANDLER_WINDOW_CLASS, hmodule);
E_RegisterClass:
	/* error message (pushed at the above) */
	lua_pushstring(L, " (");
	Crj_PushWindowsError(L, 0);
	lua_pushstring(L, ")");
	lua_concat(L, 4);
	lua_error(L);
	return;
}


static void
handler_window_cleanup(lua_State* L __attribute__((unused)), HMODULE hmodule)
{
	DestroyWindow(s_HandlerWindow);
	s_HandlerWindow = NULL;

	UnregisterClass(HANDLER_WINDOW_CLASS, hmodule);
}








/* Public Functions =================================================== {{{1 */

static int
ui_hotkey__register(lua_State* L)
{
	int vk;
	int modifiers;
	int action;
	Crj_ParseArgs(L, "i i O/f", &vk, &modifiers, &action);

	if (HOTKEY_ID_MAX < s_HotkeyCount) {
		lua_pushstring(L, "new hotkey cannot be defined anymore");
		goto E_HotkeyCount;
	}

	if (!RegisterHotKey(s_HandlerWindow, s_HotkeyCount, modifiers, vk)) {
		lua_pushstring(L, "RegisterHotKey is failed");
		goto E_RegisterHotKey;
	}

		/* action_table[s_HotkeyCount] = action */
	lua_getfield(L, LUA_REGISTRYINDEX, REGKEY_ACTION_TABLE);
	lua_pushinteger(L, s_HotkeyCount);
	lua_pushvalue(L, action);
	lua_settable(L, -3);
	lua_pop(L, 1);

	s_HotkeyCount++;
	return 0;

E_RegisterHotKey:
E_HotkeyCount:
	/* error message (pushed at the above) */
	lua_pushstring(L, " (");
	Crj_PushWindowsError(L, 0);
	lua_pushstring(L, ")");
	lua_concat(L, 4);
	return lua_error(L);
}




static int
ui_hotkey_reset(lua_State* L __attribute__((unused)))
{
	WPARAM i;

	for (i = HOTKEY_ID_MIN; i < s_HotkeyCount; i++)
		UnregisterHotKey(s_HandlerWindow, i);
	s_HotkeyCount = HOTKEY_ID_MIN;
	return 0;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"_register", ui_hotkey__register},
	{"reset", ui_hotkey_reset},
	{NULL, NULL}
};








/* Main =============================================================== {{{1 */

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

		lua_newtable(L);
		lua_setfield(L, LUA_REGISTRYINDEX, REGKEY_ACTION_TABLE);

		handler_window_startup(L, hmodule);
		break;
	case Crj_EXT_UNLOADING:
		handler_window_cleanup(L, hmodule);
		break;
	}

	return 0;
}

/* __END__ */
/* vim: foldmethod=marker
 */
