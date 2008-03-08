/* Extension: ui.monitor - monitor-related API =========================== {{{1
 *
 * Copyright (C) 2007 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"








/* Misc. ============================================================== {{{1 */

#define RECT_X(rect) ((rect).left)
#define RECT_Y(rect) ((rect).top)
#define RECT_WIDTH(rect) ((rect).right - (rect).left)
#define RECT_HEIGHT(rect) ((rect).bottom - (rect).top)








/* Public Functions =================================================== {{{1 */

static BOOL CALLBACK
enum_monitor_proc(HMONITOR hmonitor,
                  HDC hdc __attribute__((unused)),
                  LPRECT rect __attribute__((unused)),
                  LPARAM data)
{
	lua_State* L = (lua_State*)data;
	int table_length;

	table_length = lua_objlen(L, -1);
	lua_pushlightuserdata(L, hmonitor);
	lua_rawseti(L, -2, table_length + 1);
	return TRUE;
}

static int
ui_monitor_find_all(lua_State* L)
{
	lua_newtable(L);
	EnumDisplayMonitors(NULL, NULL, enum_monitor_proc, (LPARAM)L);
	return 1;
}




static int
ui_monitor_find_by_point(lua_State* L)
{
	POINT pt;
	DWORD flags = MONITOR_DEFAULTTONEAREST;
	Crj_ParseArgs(L, "l l | L", &(pt.x), &(pt.y), &flags);

	lua_pushlightuserdata(L, MonitorFromPoint(pt, flags));
	return 1;
}

static int
ui_monitor_find_by_rect(lua_State* L)
{
	RECT rect;
	DWORD flags = MONITOR_DEFAULTTONEAREST;
	Crj_ParseArgs(L, "l l l l | L",
	              &(rect.left), &(rect.top), &(rect.right), &(rect.bottom),
	              &flags);

	lua_pushlightuserdata(L, MonitorFromRect(&rect, flags));
	return 1;
}

static int
ui_monitor_find_by_window(lua_State* L)
{
	HWND hwnd = NULL;
	DWORD flags = MONITOR_DEFAULTTONEAREST;
	Crj_ParseArgs(L, "| u L", &hwnd, &flags);
	if (hwnd == NULL)
		hwnd = GetForegroundWindow();

	lua_pushlightuserdata(L, MonitorFromWindow(hwnd, flags));
	return 1;
}




static int
ui_monitor_get_info(lua_State* L)
{
	HMONITOR hmonitor;
	MONITORINFO info;
	Crj_ParseArgs(L, "u", &hmonitor);

	info.cbSize = sizeof(info);
	GetMonitorInfo(hmonitor, &info);
	Crj_BuildValues(L, "{=s{llll} =s{llll} =sL}",
	                "placement",
	                  RECT_X(info.rcMonitor),
	                  RECT_Y(info.rcMonitor),
	                  RECT_WIDTH(info.rcMonitor),
	                  RECT_HEIGHT(info.rcMonitor),
	                "workarea",
	                  RECT_X(info.rcWork),
	                  RECT_Y(info.rcWork),
	                  RECT_WIDTH(info.rcWork),
	                  RECT_HEIGHT(info.rcWork),
	                "flags",
	                  info.dwFlags);
	return 1;
}




static int
ui_monitor_get_vscreen_placement(lua_State* L)
{
	Crj_BuildValues(L, "{llll}",
	                GetSystemMetrics(SM_XVIRTUALSCREEN),
	                GetSystemMetrics(SM_YVIRTUALSCREEN),
	                GetSystemMetrics(SM_CXVIRTUALSCREEN),
	                GetSystemMetrics(SM_CYVIRTUALSCREEN));
	return 1;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"find_all", ui_monitor_find_all},
	{"find_by_point", ui_monitor_find_by_point},
	{"find_by_rect", ui_monitor_find_by_rect},
	{"find_by_window", ui_monitor_find_by_window},
	{"get_info", ui_monitor_get_info},
	{"get_vscreen_placement", ui_monitor_get_vscreen_placement},
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

		lua_pushinteger(L, MONITOR_DEFAULTTONEAREST);
		lua_setfield(L, -2, "DEFAULT_NEAREST");
		lua_pushinteger(L, MONITOR_DEFAULTTONULL);
		lua_setfield(L, -2, "DEFAULT_NULL");
		lua_pushinteger(L, MONITOR_DEFAULTTOPRIMARY);
		lua_setfield(L, -2, "DEFAULT_PRIMARY");

		lua_pop(L, 1);
		break;
	case Crj_EXT_UNLOADING:
		break;
	}

	return 0;
}

/* __END__ */
/* vim: foldmethod=marker
 */
