/* Extension: ui.window - window-related API ============================= {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"

#include <ctype.h>








/* Misc. ============================================================== {{{1 */

static HWND
GetTargetWindow(HWND hwnd)
{
	if (hwnd == NULL)
		return GetForegroundWindow();
	else
		return hwnd;
}


static BOOL
IsTopmostP(HWND hwnd)
{
	return !!(GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
}


static HWND
GetNextAppWindow(HWND base_hwnd, BOOL nontopmost_onlyp)
{
	HWND next_hwnd = base_hwnd;
	DWORD style;
	DWORD exstyle;

	while (TRUE) {
		next_hwnd = GetNextWindow(next_hwnd, GW_HWNDNEXT);
		if (next_hwnd == NULL)
			break;

		style = GetWindowLongPtr(next_hwnd, GWL_STYLE);
		if ((!(style & WS_VISIBLE)) || (style & WS_DISABLED))
			continue;
		exstyle = GetWindowLongPtr(next_hwnd, GWL_EXSTYLE);
		if (exstyle & WS_EX_TOOLWINDOW)
			continue;
		if (nontopmost_onlyp && IsTopmostP(next_hwnd))
			continue;

		break;
	}

	return next_hwnd;
}


/* BUGS: GetWindow(topmost_toplevel_window, GW_HWNDLAST) does not work as
 * documented in MSDN.  It should return the lowest window that is topmost and
 * top-level window, but actually, it returns the lowest top-level window that
 * may be or may not be topmost.
 */
static HWND
GetBottomTopmostWindow(HWND base_hwnd)
{
	HWND prev_hwnd;
	HWND next_hwnd;

	prev_hwnd = NULL;
	next_hwnd = base_hwnd;
	while (TRUE) {
		if (!IsTopmostP(next_hwnd))
			break;

		prev_hwnd = next_hwnd;
		next_hwnd = GetNextWindow(next_hwnd, GW_HWNDNEXT);
		if (next_hwnd == NULL)
			return NULL;
	}

	return prev_hwnd;
}




#define RECT_X(rect) (rect.left)
#define RECT_Y(rect) (rect.top)
#define RECT_WIDTH(rect) (rect.right - rect.left)
#define RECT_HEIGHT(rect) (rect.bottom - rect.top)








/* Public Functions =================================================== {{{1 */

/* Size --------------------------------------------------------------- {{{2 */

static int
ui_window_minimizedp(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushboolean(L, IsIconic(hwnd));
	return 1;
}

static int
ui_window_maximizedp(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushboolean(L, IsZoomed(hwnd));
	return 1;
}


static int
ui_window_resizablep(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushboolean(L, GetWindowLongPtr(hwnd, GWL_STYLE) & WS_SIZEBOX);
	return 1;
}




static int ui_window_lower(lua_State*);  /* forward */
static int
ui_window_minimize(lua_State* L)
{
	HWND hwnd = NULL;
	BOOL use_alt_methodp = FALSE;
	Crj_ParseArgs(L, "| u Q", &hwnd, &use_alt_methodp);
	hwnd = GetTargetWindow(hwnd);

	if (IsIconic(hwnd)) {
		PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	} else {
		if (!use_alt_methodp) {
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		} else {
			/* NOTE: To avoid trimming the working set of the
			 * target window, use ShowWindow with SW_SHOWMINIMIZED
			 * or SW_SHOWMINNOACTIVE to minimize the target window.
			 * See <http://support.microsoft.com/kb/293215/en-us>
			 * for more detail.
			 *
			 * But this method does not maintain Z-order properly,
			 * so that we have to do it manually by using
			 * ui.window.lower.
			 */
			if (IsTopmostP(hwnd)) {
				/* Place topmost hwnd to the bottom of topmost
				 * windows.  Because ui.window.lower emulates
				 * Alt-Esc and it does not maintain Z-order as
				 * WM_SYSCOMMAND/SC_MINIMIZE does, so that we
				 * have to do manually in this case.
				 */
				HWND bottom_hwnd =GetBottomTopmostWindow(hwnd);
				if (bottom_hwnd != NULL) {
					SetWindowPos(hwnd, bottom_hwnd,
					             0, 0, 0, 0,
					             (SWP_ASYNCWINDOWPOS
					              | SWP_NOACTIVATE
					              | SWP_NOMOVE
					              | SWP_NOOWNERZORDER
					              | SWP_NOSIZE));
				}
			}

			lua_pushcfunction(L, ui_window_lower);
			lua_pushlightuserdata(L, hwnd);
			lua_pushboolean(L, !IsHungAppWindow(hwnd));
			Crj_Call(L, 2, 0);

			ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
		}
	}
	return 0;
}

static int
ui_window_maximize(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	PostMessage(hwnd, WM_SYSCOMMAND,
	            (IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE),
	            0);
	return 0;
}

static int
ui_window_restore(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	return 0;
}


static int
ui_window_resize(lua_State* L)
{
	HWND hwnd = NULL;
	int width;
	int height;
	Crj_ParseArgs(L, "i i | u", &width, &height, &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetWindowPos(hwnd, NULL, 0, 0, width, height,
	             (SWP_ASYNCWINDOWPOS
	              | SWP_NOACTIVATE
	              | SWP_NOMOVE
	              | SWP_NOOWNERZORDER
	              | SWP_NOZORDER));
	return 0;
}




/* Position ----------------------------------------------------------- {{{2 */

static int
ui_window__move(lua_State* L)
{
	HWND hwnd = NULL;
	int x;
	int y;
	Crj_ParseArgs(L, "i i | u", &x, &y, &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetWindowPos(hwnd, NULL, x, y, 0, 0,
	             (SWP_ASYNCWINDOWPOS
	              | SWP_NOACTIVATE
	              | SWP_NOOWNERZORDER
	              | SWP_NOSIZE
	              | SWP_NOZORDER));
	return 0;
}




static int
ui_window_always_on_topp(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushboolean(L, IsTopmostP(hwnd));
	return 1;
}

static int
ui_window_set_always_on_top(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetWindowPos(hwnd, (IsTopmostP(hwnd) ? HWND_NOTOPMOST : HWND_TOPMOST),
	             0, 0, 0, 0,
	             (SWP_ASYNCWINDOWPOS
	              | SWP_NOACTIVATE
	              | SWP_NOMOVE
	              | SWP_NOOWNERZORDER
	              | SWP_NOSIZE));
	return 0;
}




static int
ui_window_raise(lua_State* L)
{
	HWND hwnd = NULL;
	HWND rootowner;
	HWND lap;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	rootowner = GetAncestor(hwnd, GA_ROOTOWNER);
	if (rootowner != NULL)
		hwnd = rootowner;
	lap = GetLastActivePopup(hwnd);
	if (lap != NULL)
		hwnd = lap;

	if (hwnd != GetForegroundWindow()) {
		SetForegroundWindow(hwnd);
	} else {
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
		             (SWP_ASYNCWINDOWPOS
		              | SWP_NOACTIVATE
		              | SWP_NOMOVE
		              | SWP_NOOWNERZORDER
		              | SWP_NOSIZE));
	}
	return 0;
}

static int
ui_window_lower(lua_State* L)  /* emulate Alt-Esc. */
{
	HWND hwnd = NULL;
	HWND root;
	HWND next_hwnd;
	BOOL syncp = FALSE;
	Crj_ParseArgs(L, "| u Q", &hwnd, &syncp);
	hwnd = GetTargetWindow(hwnd);

	root = GetAncestor(hwnd, GA_ROOT);
	if (root != NULL)
		hwnd = root;

	if (!IsTopmostP(hwnd)) {
		if (hwnd == GetForegroundWindow()) {
			next_hwnd = GetNextAppWindow(hwnd, FALSE);
			if (next_hwnd != NULL)
				SetForegroundWindow(next_hwnd);
		}
		SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
		             (((!syncp) ? SWP_ASYNCWINDOWPOS : 0)
		              | SWP_NOACTIVATE
		              | SWP_NOMOVE
		              | SWP_NOOWNERZORDER
		              | SWP_NOSIZE));
	} else {
		if (hwnd == GetForegroundWindow()) {
			next_hwnd = GetNextAppWindow(hwnd, TRUE);
			if (next_hwnd != NULL)
				SetForegroundWindow(next_hwnd);
		}
	}
	return 0;
}




/* Etc ---------------------------------------------------------------- {{{2 */

static int
ui_window_validp(lua_State* L)
{
	HWND hwnd;
	  /* hwnd must be given, because the active window is always valid. */
	Crj_ParseArgs(L, "u", &hwnd);

	lua_pushboolean(L, IsWindow(hwnd));
	return 1;
}




static int
ui_window_find_by_name(lua_State* L)
{
	const char* class_name = NULL;
	const char* title_name = NULL;
	Crj_ParseArgs(L, "| z z", &class_name, &title_name);

	lua_pushlightuserdata(L, FindWindow(class_name, title_name));
	return 1;
}

static int
ui_window_find_by_point(lua_State* L)
{
	POINT pt;
	Crj_ParseArgs(L, "l l", &(pt.x), &(pt.y));

	lua_pushlightuserdata(L, WindowFromPoint(pt));
	return 1;
}




static int
ui_window_post_message(lua_State* L)
{
	HWND hwnd = NULL;
	UINT msg = WM_NULL;
	WPARAM wp = 0;
	LPARAM lp = 0;
	Crj_ParseArgs(L, "| u I I L", &hwnd, &msg, &wp, &lp);
	hwnd = GetTargetWindow(hwnd);

	PostMessage(hwnd, msg, wp, lp);
	return 0;
}




static int
ui_window_set_alpha(lua_State* L)
{
	int alpha;
	HWND hwnd = NULL;
	LONG exstyle;
	Crj_ParseArgs(L, "i | u", &alpha, &hwnd);
	hwnd = GetTargetWindow(hwnd);

	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (exstyle & WS_EX_LAYERED) {
		SetWindowLong(hwnd, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);
	} else {
		SetWindowLong(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
		if (alpha < 0)
			alpha = 0;
		if (255 < alpha)
			alpha = 255;
		SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
	}
	InvalidateRect(hwnd, NULL, TRUE);
	return 0;
}




static int
ui_window_get_minimized_metrics(lua_State* L)
{
	MINIMIZEDMETRICS mm;
	
	ZeroMemory(&mm, sizeof(mm));
	mm.cbSize = sizeof(mm);
	SystemParametersInfo(SPI_GETMINIMIZEDMETRICS, sizeof(mm), &mm, 0);
	Crj_BuildValues(L, "{=si =si =si =si}",
	                "width", mm.iWidth,
	                "horizontal_gap", mm.iHorzGap,
	                "vertical_gap", mm.iVertGap,
	                "arrange", mm.iArrange);
	return 1;
}

static int
ui_window_set_minimized_metrics(lua_State* L)
{
	MINIMIZEDMETRICS mm;

	ZeroMemory(&mm, sizeof(mm));
	mm.cbSize = sizeof(mm);
	Crj_ParseArgs(L, "{=si =si =si =si}",
	              "width", &(mm.iWidth),
	              "horizontal_gap", &(mm.iHorzGap),
	              "vertical_gap", &(mm.iVertGap),
	              "arrange", &(mm.iArrange));
	SystemParametersInfo(SPI_SETMINIMIZEDMETRICS, sizeof(mm), &mm, 0);
	return 1;
}




static int
ui_window_get_foreground_window(lua_State* L)
{
	lua_pushlightuserdata(L, GetForegroundWindow());
	return 1;
}




static int
ui_window_get_placement(lua_State* L)
{
	HWND hwnd = NULL;
	RECT rect;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	GetWindowRect(hwnd, &rect);
	lua_pushinteger(L, RECT_X(rect));
	lua_pushinteger(L, RECT_Y(rect));
	lua_pushinteger(L, RECT_WIDTH(rect));
	lua_pushinteger(L, RECT_HEIGHT(rect));
	return 4;
}




static int
ui_window_get_class_name(lua_State* L)
{
	HWND hwnd = NULL;
	TCHAR buft[256];  /* this limit is documented in MSDN. */
	char bufu[Crj_NUMBER_OF(buft) * Utf8_NATIVE_RATIO];
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	if (!GetClassName(hwnd, buft, Crj_NUMBER_OF(buft)))
		return luaL_error(L, "GetClassName failed");
	buft[Crj_NUMBER_OF(buft) - 1] = TEXT('\0');
	if (!Utf8_FromNative(bufu, Crj_NUMBER_OF(bufu), buft, -1))
		return luaL_error(L, "Utf8_FromNative failed");
	lua_pushstring(L, bufu);
	return 1;
}


static int
ui_window_get_title_name(lua_State* L)
{
	HWND hwnd = NULL;
	int length;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetLastError(ERROR_SUCCESS);
	length = GetWindowTextLength(hwnd);
	if (0 < length) {
		TCHAR buft[length + 1];  /* the last NUL is not counted. */
		char bufu[Crj_NUMBER_OF(buft) * Utf8_NATIVE_RATIO];
		int last;

		SetLastError(ERROR_SUCCESS);
		last = GetWindowText(hwnd, buft, Crj_NUMBER_OF(buft));
		if ((!last) && (GetLastError() != ERROR_SUCCESS))
			return luaL_error(L, "GetWindowText failed");
		buft[Crj_NUMBER_OF(buft) - 1] = TEXT('\0');

		if (!Utf8_FromNative(bufu, Crj_NUMBER_OF(bufu), buft, -1))
			return luaL_error(L, "Utf8_FromNative failed");
		lua_pushstring(L, bufu);
	} else {
		if (GetLastError() != ERROR_SUCCESS)
			return luaL_error(L, "GetWindowText failed");
		lua_pushstring(L, "");
	}
	return 1;
}




static int
ui_window_get_style(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushnumber(L, (lua_Number)GetWindowLongPtr(hwnd, GWL_STYLE));
	return 1;
}

static int
ui_window_set_style(lua_State* L)
{
	LONG style;
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "l | u", &style,  &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetWindowLong(hwnd, GWL_STYLE, style);
	return 0;
}

static int
ui_window_get_exstyle(lua_State* L)
{
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "| u", &hwnd);
	hwnd = GetTargetWindow(hwnd);

	lua_pushnumber(L, (lua_Number)GetWindowLongPtr(hwnd, GWL_EXSTYLE));
	return 1;
}

static int
ui_window_set_exstyle(lua_State* L)
{
	LONG style;
	HWND hwnd = NULL;
	Crj_ParseArgs(L, "l | u", &style,  &hwnd);
	hwnd = GetTargetWindow(hwnd);

	SetWindowLong(hwnd, GWL_EXSTYLE, style);
	return 0;
}




static BOOL CALLBACK
_ui_window_enumerate_proc(HWND hwnd, LPARAM lp)
{
	lua_State* L = (lua_State*)lp;
	BOOL result;

	lua_pushvalue(L, -1);  /* user callback proc */
	lua_pushlightuserdata(L, hwnd);
	if (Crj_PCall(L, 1, 1) != 0)
		return FALSE;
	result = lua_toboolean(L, -1);
	lua_pop(L, 1);
	if (!result)
		lua_pushnil(L);  /* indicates intentional breaking. */
	return result;
}

static int
ui_window_enumerate(lua_State* L)
{
	int proc;
	Crj_ParseArgs(L, "O/f", &proc);

	lua_pushvalue(L, proc);
	if (!EnumWindows(_ui_window_enumerate_proc, (LPARAM)L)) {
		if (lua_isnil(L, -1)) {
			lua_pushboolean(L, FALSE);
			return 1;
		} else if (lua_isstring(L, -1)) {
			;  /* Use the error message pushed by proc. */
		} else {
			Crj_PushWindowsError(L, GetLastError());
		}
		lua_error(L);
	}
	lua_pushboolean(L, TRUE);
	return 1;
}




static int
ui_window_refresh(lua_State* L)
{
	HWND hwnd = NULL;
	UINT flags = 0;
	Crj_ParseArgs(L, "| I u", &flags, &hwnd);
	flags |= SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER;
	hwnd = GetTargetWindow(hwnd);

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, flags);
	return 0;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"minimizedp", ui_window_minimizedp},
	{"maximizedp", ui_window_maximizedp},
	{"resizablep", ui_window_resizablep},
	{"minimize", ui_window_minimize},
	{"maximize", ui_window_maximize},
	{"restore", ui_window_restore},
	{"resize", ui_window_resize},

	{"_move", ui_window__move},
	{"always_on_topp", ui_window_always_on_topp},
	{"set_always_on_top", ui_window_set_always_on_top},
	{"raise", ui_window_raise},
	{"lower", ui_window_lower},

	{"validp", ui_window_validp},
	{"find_by_name", ui_window_find_by_name},
	{"find_by_point", ui_window_find_by_point},
	{"post_message", ui_window_post_message},
	{"set_alpha", ui_window_set_alpha},
	{"get_minimized_metrics", ui_window_get_minimized_metrics},
	{"set_minimized_metrics", ui_window_set_minimized_metrics},
	{"get_foreground_window", ui_window_get_foreground_window},
	{"get_placement", ui_window_get_placement},
	{"get_class_name", ui_window_get_class_name},
	{"get_title_name", ui_window_get_title_name},
	{"get_style", ui_window_get_style},
	{"set_style", ui_window_set_style},
	{"get_exstyle", ui_window_get_exstyle},
	{"set_exstyle", ui_window_set_exstyle},
	{"enumerate", ui_window_enumerate},
	{"refresh", ui_window_refresh},

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
		break;
	case Crj_EXT_UNLOADING:
		break;
	}

	return 0;
}

/* __END__ */
/* vim: foldmethod=marker
 */
