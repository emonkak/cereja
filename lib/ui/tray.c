/* Extension: ui.tray - user interface for tray ========================== {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */
/* FIXME: use_large_iconp
 * FIXME: visual style
 */

#include "cereja.h"
#include "shell/tray.h"








/* Misc. ============================================================== {{{1 */

/* Etc. --------------------------------------------------------------- {{{2 */

#define REGKEY_KEYTABLE "cereja:ui.tray:key_table"




/* BUGS: assumes sizeof(void*), sizeof(long) and sizeof(function*) are same. */
#define POINTERIZE(x) ((void*)(long)(x))


#define RECT_WIDTH(rect) (rect.right - rect.left)
#define RECT_HEIGHT(rect) (rect.bottom - rect.top)


#define KEY_PRESSEDP(vk) (GetKeyState(vk) < 0)
#define KEY_COMBO(vk,modifiers) (((modifiers)<<8) | (vk))


#define DO_COMMAND(hwnd,id) SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(id,0), 0)


static HFONT
CreateMenuFont(void)
{
	NONCLIENTMETRICS ncm;

	ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

	return CreateFontIndirect(&(ncm.lfMenuFont));
}


static void
GetMonitorRectByWindow(RECT* p_rect, HWND hwnd)
{
	MONITORINFO info;

	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST),
	               &info);
	*p_rect = info.rcMonitor;
	return;
}




static void
clean_icon(lua_State* L, int icon_index)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "shell");
	lua_getfield(L, -1, "tray");

	lua_getfield(L, -1, "clean_icon");
	lua_pushnumber(L, icon_index);
	lua_call(L, 1, 0);  /* FIXME: should be protected? */

	lua_pop(L, 2);
}


static void
move_icon(lua_State* L, int icon_index, int dest)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "shell");
	lua_getfield(L, -1, "tray");

	lua_getfield(L, -1, "move_icon");
	lua_pushnumber(L, icon_index);
	lua_pushnumber(L, dest);
	lua_call(L, 2, 0);  /* FIXME: should be protected? */

	lua_pop(L, 2);
}


static BOOL
hide_when_inactivep(lua_State* L)
{
	BOOL result;

	lua_getfield(L, LUA_GLOBALSINDEX, "ui");
	lua_getfield(L, -1, "tray");
	lua_getfield(L, -1, "hide_when_inactivep");
	result = lua_toboolean(L, -1);
	lua_pop(L, 3);

	return result;
}




static void
KeyTable_New(lua_State* L, const char* regkey)
{
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, regkey);
}


static void
KeyTable_Delete(lua_State* L, const char* regkey)
{
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, regkey);
}


static void
KeyTable_Reset(lua_State* L, const char* regkey)
{
	KeyTable_Delete(L, regkey);
	KeyTable_New(L, regkey);
}


static void
KeyTable_Add(lua_State* L, const char* regkey,
             int vk, int modifiers, int command)
{
	lua_getfield(L, LUA_REGISTRYINDEX, regkey);
	lua_pushinteger(L, command);
	lua_rawseti(L, -2, KEY_COMBO(vk, modifiers));
	lua_pop(L, 1);  /* regkey */
}


static void
KeyTable_Translate(lua_State* L, const char* regkey,
                   HWND hwnd, UINT msg,
                   WPARAM wp, LPARAM lp __attribute__((unused)))
{
	if (msg == WM_KEYDOWN) {
		int vk;
		int modifiers;
		int command;

		vk = wp;

		modifiers = 0;
		if (KEY_PRESSEDP(VK_SHIFT))
			modifiers |= MOD_SHIFT;
		if (KEY_PRESSEDP(VK_MENU))
			modifiers |= MOD_ALT;
		if (KEY_PRESSEDP(VK_CONTROL))
			modifiers |= MOD_CONTROL;
		if (KEY_PRESSEDP(VK_LWIN) || KEY_PRESSEDP(VK_RWIN))
			modifiers |= MOD_WIN;

		lua_getfield(L, LUA_REGISTRYINDEX, regkey);
		lua_rawgeti(L, -1, KEY_COMBO(vk, modifiers));
		command = lua_tointeger(L, -1);
		if (command)
			DO_COMMAND(hwnd, command);
		lua_pop(L, 1);  /* command */
		lua_pop(L, 1);  /* regkey */
	}
}




/* UI Window ---------------------------------------------------------- {{{2 */

#define UI_WINDOW_CLASS TEXT("cereja.ui.tray")
#define UI_WINDOW_TITLE NULL

#define UIM_TRAY_UPDATE (WM_USER + 1)

#define UIC_SHOW 1
#define UIC_HIDE 2
#define UIC_CURSOR_NEXT 3
#define UIC_CURSOR_PREV 4
#define UIC_CURSOR_FIRST 5
#define UIC_CURSOR_LAST 6
#define UIC_SEND_LEFT_CLICK 7
#define UIC_SEND_LEFT_DOUBLE_CLICK 8
#define UIC_SEND_RIGHT_CLICK 9
#define UIC_SEND_RIGHT_DOUBLE_CLICK 10
#define UIC_SEND_MIDDLE_CLICK 11
#define UIC_SEND_MIDDLE_DOUBLE_CLICK 12
#define UIC_ICON_MOVE_NEXT 13
#define UIC_ICON_MOVE_PREV 14
#define UIC_ICON_MOVE_FIRST 15
#define UIC_ICON_MOVE_LAST 16
#define UIC_ICON_SHIFT_NEXT 17
#define UIC_ICON_SHIFT_PREV 18

#define CURSOR_INVALID (-20061104)


static HWND s_UIWindow;
static lua_State* s_L;
static const CrjTrayData* s_TrayData;
static int s_Cursor = CURSOR_INVALID;

#define NO_ICON_MESSAGE TEXT("(*no tray icon*)")
static HFONT s_MenuFont;
static int s_BorderWidth, s_BorderHeight;
static int s_EdgeWidth, s_EdgeHeight;
static const int s_ItemPaddingWidth = 8, s_ItemPaddingHeight = 4;
static const int s_IconTipGap = 8;
static int s_IconSize = 16;
static int s_ItemMaxHeight;




/* ,======================.
 * || Icon | |    Tip    ||
 * ||------+-+-----------||
 * ||   :  | |     :     ||
 * ||   :  | |     :     ||
 * `======================'
 */
static void
ui_window_resize(HWND hwnd)
{
	HDC hdc;
	RECT rect = {0, 0, 0, 0};
	int whole_width;
	int whole_height;
	HFONT original_font;

	hdc = GetDC(hwnd);
	original_font = SelectObject(hdc, s_MenuFont);
	if (s_TrayData->count == 0) {
		DrawText(hdc, NO_ICON_MESSAGE, -1, &rect,
		         DT_CALCRECT | (DT_NOPREFIX | DT_SINGLELINE));

		whole_width = (s_EdgeWidth
		               + s_BorderWidth
		               + s_ItemPaddingWidth
		               + RECT_WIDTH(rect)
		               + s_ItemPaddingWidth
		               + s_BorderWidth
		               + s_EdgeWidth);
		whole_height = (s_EdgeHeight
		                + s_BorderHeight
		                + s_ItemPaddingHeight
		                + RECT_HEIGHT(rect)
		                + s_ItemPaddingHeight
		                + s_BorderHeight
		                + s_EdgeHeight);
		/* s_ItemMaxWidth = ...; */
		/* s_ItemMaxHeight = ...; */
	} else {
		int item_max_width;
		int i;

		item_max_width = 0;
		s_ItemMaxHeight = s_IconSize;
		for (i = 0; i < s_TrayData->count; i++) {
			DrawText(hdc,
			         s_TrayData->icons[i]->szTip,
			         -1,
			         &rect,
			         DT_CALCRECT | (DT_NOPREFIX | DT_SINGLELINE));
			if (item_max_width < RECT_WIDTH(rect))
				item_max_width = RECT_WIDTH(rect);
			if (s_ItemMaxHeight < RECT_HEIGHT(rect))
				s_ItemMaxHeight = RECT_HEIGHT(rect);
		}

		whole_width = (s_EdgeWidth
		               + s_BorderWidth
		               + s_ItemPaddingWidth
		               + s_IconSize + s_IconTipGap + item_max_width
		               + s_ItemPaddingWidth
		               + s_BorderWidth
		               + s_EdgeWidth);
		whole_height = (s_EdgeHeight
		                + s_BorderHeight
		                + s_TrayData->count
		                  * (s_ItemPaddingHeight
		                     + s_ItemMaxHeight
		                     + s_ItemPaddingHeight)
		                + s_BorderHeight
		                + s_EdgeHeight);
	}
	SelectObject(hdc, original_font);
	ReleaseDC(hwnd, hdc);

	GetMonitorRectByWindow(&rect, GetForegroundWindow());
	MoveWindow(hwnd,
	           (RECT_WIDTH(rect) - whole_width) / 2,
	           (RECT_HEIGHT(rect) - whole_height) / 2,
	           whole_width, whole_height,
	           TRUE);
}


static void
ui_window_paint(HWND hwnd, HDC hdc)
{
	RECT rect;
	HFONT original_font;
	COLORREF cursor_text_color;
	COLORREF normal_text_color;
	HGDIOBJ border_brush;
	HGDIOBJ cursor_background_brush;

	if (hwnd == GetForegroundWindow()) {  /* active? */
		cursor_text_color = GetSysColor(COLOR_HIGHLIGHTTEXT);
		normal_text_color = GetSysColor(COLOR_MENUTEXT);
		border_brush = GetSysColorBrush(COLOR_ACTIVEBORDER);
		cursor_background_brush = GetSysColorBrush(COLOR_HIGHLIGHT);
	} else {
		cursor_text_color = GetSysColor(COLOR_GRAYTEXT);
		normal_text_color = GetSysColor(COLOR_GRAYTEXT);
		border_brush = GetSysColorBrush(COLOR_INACTIVEBORDER);
		cursor_background_brush = GetStockObject(NULL_BRUSH);
	}

	GetClientRect(hwnd, &rect);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, normal_text_color);
	original_font = SelectObject(hdc, s_MenuFont);

	FillRect(hdc, &rect, GetSysColorBrush(COLOR_MENU));
	DrawEdge(hdc, &rect, EDGE_RAISED,
	         BF_TOPLEFT | BF_BOTTOMRIGHT | BF_FLAT);
	InflateRect(&rect, -s_EdgeWidth, -s_EdgeHeight);
	FrameRect(hdc, &rect, border_brush);
	InflateRect(&rect, -s_BorderWidth, -s_BorderHeight);

	if (s_TrayData->count == 0) {
		InflateRect(&rect, -s_ItemPaddingWidth, -s_ItemPaddingHeight);
		DrawText(hdc, NO_ICON_MESSAGE, -1, &rect,
		         DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
	} else {
		int height_diff = ((s_IconSize < s_ItemMaxHeight)
		                   ? ((s_ItemMaxHeight - s_IconSize) / 2)
		                   : 0);
		int i;

		InflateRect(&rect, -s_ItemPaddingWidth, -s_ItemPaddingHeight);
		rect.bottom = rect.top + s_ItemMaxHeight;
		for (i = 0; i < s_TrayData->count; i++) {
			if (i == s_Cursor) {
				RECT r = rect;

				InflateRect(&r,
				            s_ItemPaddingWidth * 2 / 3,
				            s_ItemPaddingHeight * 2 / 3);
				FillRect(hdc, &r, cursor_background_brush);
				SetTextColor(hdc, cursor_text_color);
			}

			if (s_TrayData->icons[i]->hIcon != NULL) {
				DrawIconEx(hdc,
				           rect.left,
				           rect.top + height_diff,
				           s_TrayData->icons[i]->hIcon,
				           s_IconSize, s_IconSize,
				           0,
				           NULL,
				           DI_NORMAL);
			} else {
				RECT r;

				r.left = rect.left;
				r.right = r.left + s_IconSize;
				r.top = rect.top + height_diff;
				r.bottom = r.top + s_IconSize;
				DrawFrameControl(hdc, &r, DFC_CAPTION,
				                 DFCS_CAPTIONCLOSE);
			}
			rect.left += s_IconSize + s_IconTipGap;
			DrawText(hdc,
			         s_TrayData->icons[i]->szTip,
			         -1,
			         &rect,
			         DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			rect.left -= s_IconSize + s_IconTipGap;

			if (i == s_Cursor)
				SetTextColor(hdc, normal_text_color);

			rect.top += (s_ItemPaddingHeight
			             + s_ItemMaxHeight
			             + s_ItemPaddingHeight);
			rect.bottom = rect.top + s_ItemMaxHeight;
		}
	}

	SelectObject(hdc, original_font);
}




static void
send_click_message(const CrjTrayIcon* icon, int message)
{
	SetForegroundWindow(icon->hWnd);

	#define POST(icon,msg) \
	        PostMessage((icon)->hWnd, (icon)->uCallbackMessage, \
	                    (WPARAM)((icon)->uID), (LPARAM)(msg))
	switch (message) {
	default:
		break;

	case UIC_SEND_LEFT_CLICK:
		POST(icon, WM_LBUTTONDOWN);
		POST(icon, WM_LBUTTONUP);
		break;
	case UIC_SEND_LEFT_DOUBLE_CLICK:
		POST(icon, WM_LBUTTONDOWN);
		POST(icon, WM_LBUTTONUP);
		POST(icon, WM_LBUTTONDBLCLK);
		POST(icon, WM_LBUTTONUP);
		break;

	case UIC_SEND_RIGHT_CLICK:
		POST(icon, WM_RBUTTONDOWN);
		POST(icon, WM_RBUTTONUP);
		break;
	case UIC_SEND_RIGHT_DOUBLE_CLICK:
		POST(icon, WM_RBUTTONDOWN);
		POST(icon, WM_RBUTTONUP);
		POST(icon, WM_RBUTTONDBLCLK);
		POST(icon, WM_RBUTTONUP);
		break;

	case UIC_SEND_MIDDLE_CLICK:
		POST(icon, WM_MBUTTONDOWN);
		POST(icon, WM_MBUTTONUP);
		break;
	case UIC_SEND_MIDDLE_DOUBLE_CLICK:
		POST(icon, WM_MBUTTONDOWN);
		POST(icon, WM_MBUTTONUP);
		POST(icon, WM_MBUTTONDBLCLK);
		POST(icon, WM_MBUTTONUP);
		break;
	}
	#undef POST
}




static LRESULT CALLBACK
ui_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	default:
		break;
	case WM_CREATE:
		s_MenuFont = CreateMenuFont();
		s_BorderWidth = GetSystemMetrics(SM_CXBORDER);
		s_BorderHeight = GetSystemMetrics(SM_CYBORDER);
		s_EdgeWidth = GetSystemMetrics(SM_CXEDGE);
		s_EdgeHeight = GetSystemMetrics(SM_CYEDGE);
		ui_window_resize(hwnd);
		KeyTable_New(s_L, REGKEY_KEYTABLE);
		break;
	case WM_DESTROY:
		KeyTable_Delete(s_L, REGKEY_KEYTABLE);
		DeleteObject(s_MenuFont);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc_original;
		HDC hdc_memory;
		HBITMAP hbmp;
		RECT rect;

		GetClientRect(hwnd, &rect);
		hdc_original = BeginPaint(hwnd, &ps);
		hdc_memory = CreateCompatibleDC(hdc_original);
		hbmp = SelectObject(hdc_memory,
		                    CreateCompatibleBitmap(hdc_original,
		                                           rect.right,
		                                           rect.bottom));

		ui_window_paint(hwnd, hdc_memory);
		BitBlt(hdc_original, 0, 0, rect.right, rect.bottom,
		       hdc_memory, 0, 0,
		       SRCCOPY);

		DeleteObject(SelectObject(hdc_memory, hbmp));
		DeleteDC(hdc_memory);
		EndPaint(hwnd, &ps);
		} return 0;
	case WM_PRINT:
	case WM_PRINTCLIENT:
		ui_window_paint(hwnd, (HDC)wp);
		break;
	case WM_ACTIVATE:
		if (wp == WA_INACTIVE) {
			if (hide_when_inactivep(s_L))
				DO_COMMAND(hwnd, UIC_HIDE);
		}
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_WINDOWPOSCHANGED:
		if (((WINDOWPOS*)lp)->flags & SWP_HIDEWINDOW)
			s_Cursor = CURSOR_INVALID;
		break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		default:
			break;
		case UIC_SHOW:
			ShowWindow(hwnd, SW_SHOW);
			SetForegroundWindow(hwnd);
			break;
		case UIC_HIDE:
			ShowWindow(hwnd, SW_HIDE);
			break;
		case UIC_CURSOR_NEXT:
			if (!IsWindowVisible(hwnd))
				break;
			if (s_Cursor == CURSOR_INVALID) {
				if (0 < s_TrayData->count)
					s_Cursor = 0;
				else
					s_Cursor = CURSOR_INVALID;
			} else {
				s_Cursor++;
				if (s_TrayData->count <= s_Cursor)
					s_Cursor -= s_TrayData->count;
			}

			if (s_Cursor != CURSOR_INVALID)
				clean_icon(s_L, s_Cursor+1);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case UIC_CURSOR_PREV:
			if (!IsWindowVisible(hwnd))
				break;
			if (s_Cursor == CURSOR_INVALID) {
				if (0 < s_TrayData->count)
					s_Cursor = s_TrayData->count - 1;
				else
					s_Cursor = CURSOR_INVALID;
			} else {
				s_Cursor--;
				if (s_Cursor < 0)
					s_Cursor += s_TrayData->count;
			}

			if (s_Cursor != CURSOR_INVALID)
				clean_icon(s_L, s_Cursor+1);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case UIC_CURSOR_FIRST:
			if (!IsWindowVisible(hwnd))
				break;
			if (s_Cursor == CURSOR_INVALID) {
				if (0 < s_TrayData->count)
					s_Cursor = 0;
				else
					s_Cursor = CURSOR_INVALID;
			} else {
				s_Cursor = 0;
			}

			if (s_Cursor != CURSOR_INVALID)
				clean_icon(s_L, s_Cursor+1);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case UIC_CURSOR_LAST:
			if (!IsWindowVisible(hwnd))
				break;
			if (s_Cursor == CURSOR_INVALID) {
				if (0 < s_TrayData->count)
					s_Cursor = s_TrayData->count - 1;
				else
					s_Cursor = CURSOR_INVALID;
			} else {
				s_Cursor = s_TrayData->count - 1;
			}

			if (s_Cursor != CURSOR_INVALID)
				clean_icon(s_L, s_Cursor+1);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case UIC_SEND_LEFT_CLICK:  /* FALLTHRU */
		case UIC_SEND_LEFT_DOUBLE_CLICK:  /* FALLTHRU */
		case UIC_SEND_RIGHT_CLICK:  /* FALLTHRU */
		case UIC_SEND_RIGHT_DOUBLE_CLICK:  /* FALLTHRU */
		case UIC_SEND_MIDDLE_CLICK:  /* FALLTHRU */
		case UIC_SEND_MIDDLE_DOUBLE_CLICK:
			if (!IsWindowVisible(hwnd))
				break;
			if (s_Cursor == CURSOR_INVALID)
				break;
			send_click_message(s_TrayData->icons[s_Cursor],
			                   LOWORD(wp));
			DO_COMMAND(hwnd, UIC_HIDE);
			break;
		case UIC_ICON_MOVE_NEXT:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, (s_Cursor)+1, (s_Cursor+1)+1);
			break;
		case UIC_ICON_MOVE_PREV:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, (s_Cursor)+1, (s_Cursor-1)+1);
			break;
		case UIC_ICON_MOVE_FIRST:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, (s_Cursor)+1, 1);
			break;
		case UIC_ICON_MOVE_LAST:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, (s_Cursor)+1, -1);
			break;
		case UIC_ICON_SHIFT_NEXT:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, -1, 1);
			break;
		case UIC_ICON_SHIFT_PREV:
			if (!IsWindowVisible(hwnd)) break;
			if (s_Cursor == CURSOR_INVALID) break;
			move_icon(s_L, 1, -1);
			break;
		}
		break;
	case UIM_TRAY_UPDATE:
		switch (wp) {
		default:
			break;
		case NIM_DELETE:
			if (s_Cursor != CURSOR_INVALID) {
				if (s_TrayData->count == 0) {
					s_Cursor = CURSOR_INVALID;
				} else if (s_TrayData->count <= s_Cursor) {
					s_Cursor = s_TrayData->count - 1;
				}
			}
			/* FALLTHRU */
		case NIM_ADD:
			ui_window_resize(hwnd);
			/* FALLTHRU */
		case NIM_MODIFY:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		}
		break;
	}

	KeyTable_Translate(s_L, REGKEY_KEYTABLE, hwnd, msg, wp, lp);
	return DefWindowProc(hwnd, msg, wp, lp);
}


static void
create_ui_window(lua_State* L, HMODULE hmodule)
{
	WNDCLASS wc;
	const char* error_message;

	s_L = L;

	lua_getfield(L, LUA_GLOBALSINDEX, "shell");
	lua_getfield(L, -1, "tray");
	lua_getfield(L, -1, "data");
	s_TrayData = lua_touserdata(L, -1);
	lua_pop(L, 3);
	if (s_TrayData == NULL) {
		error_message = "shell.tray.data";
		goto E_shell_tray_data;
	}

	ZeroMemory(&wc, sizeof(wc));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hmodule;
	wc.lpfnWndProc = ui_window_proc;
	wc.lpszClassName = UI_WINDOW_CLASS;
	if (!RegisterClass(&wc)) {
		error_message = "RegisterClass";
		goto E_RegisterClass;
	}
	s_UIWindow = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
	                            UI_WINDOW_CLASS,
	                            UI_WINDOW_TITLE,
	                            WS_POPUP,
	                            0, 0, 0, 0,
	                            NULL,
	                            NULL,
	                            hmodule,
	                            NULL);
	if (s_UIWindow == NULL) {
		error_message = "CreateWindowEx";
		goto E_CreateWindowEx;
	}
	return;

E_CreateWindowEx:
	UnregisterClass(UI_WINDOW_CLASS, hmodule);
E_RegisterClass:
E_shell_tray_data:
	lua_pushstring(L, error_message);
	lua_pushstring(L, " is failed (");
	Crj_PushWindowsError(L, 0);
	lua_pushstring(L, ")");
	lua_concat(L, 4);
	lua_error(L);
	return;
}


static void
destroy_ui_window(lua_State* L __attribute__((unused)), HMODULE hmodule)
{
	DestroyWindow(s_UIWindow);
	s_UIWindow = NULL;
	UnregisterClass(UI_WINDOW_CLASS, hmodule);
}








/* Public Functions =================================================== {{{1 */

static int
ui_tray__hotkey_add(lua_State* L)
{
	int vk;
	int modifiers;
	int command;
	Crj_ParseArgs(L, "i i i", &vk, &modifiers, &command);

	KeyTable_Add(L, REGKEY_KEYTABLE, vk, modifiers, command);
	return 0;
}


static int
ui_tray_hotkey_reset(lua_State* L)
{
	KeyTable_Reset(L, REGKEY_KEYTABLE);
	return 0;
}




static int
ui_tray_set_visibility(lua_State* L)
{
	BOOL visiblep;

	if (0 < lua_gettop(L))
		visiblep = lua_toboolean(L, 1);
	else
		visiblep = !IsWindowVisible(s_UIWindow);

	DO_COMMAND(s_UIWindow, (visiblep ? UIC_SHOW : UIC_HIDE));
	return 0;
}




static int
ui_tray__subscriber(lua_State* L)
{
	DWORD event;
	int icon_index;
	UINT flags;
	Crj_ParseArgs(L, "L i I", &event, &icon_index, &flags);

	PostMessage(s_UIWindow, UIM_TRAY_UPDATE, event, flags);
	return 0;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"_hotkey_add", ui_tray__hotkey_add},
	{"hotkey_reset", ui_tray_hotkey_reset},
	{"set_visibility", ui_tray_set_visibility},
	{"_subscriber", ui_tray__subscriber},
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
		create_ui_window(L, hmodule);

		lua_getfield(L, 1, "namespace");
		luaL_register(L, NULL, PUBLIC_FUNCTIONS);
		lua_pushlightuserdata(L, s_UIWindow);
		lua_setfield(L, -2, "window");
		#define REGISTER_CONSTANT(id) \
			lua_pushinteger(L, UIC_##id); \
			lua_setfield(L, -2, "CMD_" #id);
		REGISTER_CONSTANT(SHOW);
		REGISTER_CONSTANT(HIDE);
		REGISTER_CONSTANT(CURSOR_NEXT);
		REGISTER_CONSTANT(CURSOR_PREV);
		REGISTER_CONSTANT(CURSOR_FIRST);
		REGISTER_CONSTANT(CURSOR_LAST);
		REGISTER_CONSTANT(SEND_LEFT_CLICK);
		REGISTER_CONSTANT(SEND_LEFT_DOUBLE_CLICK);
		REGISTER_CONSTANT(SEND_RIGHT_CLICK);
		REGISTER_CONSTANT(SEND_RIGHT_DOUBLE_CLICK);
		REGISTER_CONSTANT(SEND_MIDDLE_CLICK);
		REGISTER_CONSTANT(SEND_MIDDLE_DOUBLE_CLICK);
		REGISTER_CONSTANT(ICON_MOVE_NEXT)
		REGISTER_CONSTANT(ICON_MOVE_PREV)
		REGISTER_CONSTANT(ICON_MOVE_FIRST)
		REGISTER_CONSTANT(ICON_MOVE_LAST)
		REGISTER_CONSTANT(ICON_SHIFT_NEXT)
		REGISTER_CONSTANT(ICON_SHIFT_PREV)
		#undef REGISTER_CONSTANT
		lua_pop(L, 1);
		break;
	case Crj_EXT_UNLOADING:
		destroy_ui_window(L, hmodule);
		break;
	}

	return 0;
}

/* __END__ */
/* vim: foldmethod=marker
 */
