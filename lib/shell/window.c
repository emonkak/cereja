/* Extension: shell.window - shell window procedure service  {{{1 */

#include "cereja.h"



/* Misc.  {{{1 */

#define SHELL_WINDOW_CLASS L"cereja.shell.window"
#define SHELL_WINDOW_TITLE L"cereja"

#define RSH_UNREGISTER 0
#define RSH_REGISTER 1
#define RSH_PROGMAN 2
#define RSH_TASKMAN 3


static HWND s_ShellWindow = NULL;
static BOOL (__stdcall * RegisterShellHook)(HWND, DWORD);
static UINT WM_ShellHook;

typedef struct
{
	HWND hwnd;
	RECT rc;
} SHELLHOOKINFO, *LPSHELLHOOKINFO;



static void
publish(lua_State* L, UINT msg,  UINT event, HWND hwnd)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "shell");
	lua_getfield(L, -1, "window");
	lua_getfield(L, -1, "publish");

	lua_pushnumber(L, msg);
	lua_pushnumber(L, event);
	lua_pushlightuserdata(L, hwnd);
	lua_call(L, 3, 0);

	lua_pop(L, 2);
}


static LRESULT CALLBACK
shell_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static lua_State* L;
	UINT shell_event;

	switch (msg) {
	default:
		if (msg == WM_ShellHook)
			shell_event = wp & 0x7fff;
			switch (shell_event) {
			case HSHELL_WINDOWCREATED:
			case HSHELL_WINDOWDESTROYED:
			case HSHELL_WINDOWACTIVATED:
			case HSHELL_REDRAW:
				publish(L, msg, shell_event, (HWND)lp);
				break;
			case HSHELL_GETMINRECT:
				publish(L, msg, shell_event, ((SHELLHOOKINFO*)lp)->hwnd);
				break;
			}
		break;
	case WM_CREATE:
		L = (lua_State*)(((CREATESTRUCT*)lp)->lpCreateParams);
		break;
	case WM_POWERBROADCAST:
		switch (wp) {
		case PBT_APMPOWERSTATUSCHANGE:
		case PBT_APMQUERYSTANDBY:
		case PBT_APMQUERYSTANDBYFAILED:
		case PBT_APMQUERYSUSPEND:
		case PBT_APMRESUMEAUTOMATIC:
		case PBT_APMRESUMECRITICAL:
		case PBT_APMRESUMESTANDBY:
		case PBT_APMRESUMESUSPEND:
		case PBT_APMSTANDBY:
		case PBT_APMSUSPEND:
			publish(L, msg, wp, NULL);
			break;
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}


static void
create_shell_window(lua_State* L, HMODULE hmodule)
{
	WNDCLASS wc;
	const char* error_message;

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = shell_window_proc;
	wc.hInstance = hmodule;
	wc.lpszClassName = SHELL_WINDOW_CLASS;

	if (!RegisterClass(&wc)) {
		error_message = "RegisterClass";
		goto E_RegisterClass;
	}

	s_ShellWindow = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
								   SHELL_WINDOW_CLASS,
								   SHELL_WINDOW_TITLE,
								   0,
								   0, 0, 0, 0,
								   NULL,
								   NULL,
								   hmodule,
								   L);
	if (s_ShellWindow == NULL) {
		error_message = "CreateWindowEx";
		goto E_CreateWindowEx;
	}
	
	RegisterShellHook = (BOOL (__stdcall *)(HWND, DWORD))
		GetProcAddress(GetModuleHandle(L"shell32.DLL"), (LPCSTR)((long)0x00B5));

	if (RegisterShellHook) {
		RegisterShellHook(s_ShellWindow, RSH_REGISTER);
		RegisterShellHook(s_ShellWindow, RSH_TASKMAN);
	}
	WM_ShellHook = RegisterWindowMessage(L"SHELLHOOK");

	return;

E_CreateWindowEx:
	UnregisterClass(SHELL_WINDOW_CLASS, hmodule);
E_RegisterClass:
	lua_pushstring(L, error_message);
	lua_pushstring(L, " is failed (");
	Crj_PushWindowsError(L, 0);
	lua_pushstring(L, ")");
	lua_concat(L, 4);
	lua_error(L);
	return;
}


static void
destroy_shell_window(lua_State* L __attribute__((unused)), HMODULE hmodule)
{
	if (RegisterShellHook)
		RegisterShellHook(s_ShellWindow, RSH_UNREGISTER);

	DestroyWindow(s_ShellWindow);
	s_ShellWindow = NULL;

	UnregisterClass(SHELL_WINDOW_CLASS, hmodule);
}



/* Public Functions  {{{1 */

static const luaL_Reg PUBLIC_FUNCTIONS[] = {
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
		create_shell_window(L, hmodule);

		lua_getfield(L, 1, "namespace");
		luaL_register(L, NULL, PUBLIC_FUNCTIONS);

		lua_pushinteger(L, WM_ShellHook);
		lua_setfield(L, LUA_GLOBALSINDEX, "WM_ShellHook");

		lua_pop(L, 1);
		break;
	case Crj_EXT_UNLOADING:
		destroy_shell_window(L, hmodule);
		break;
	}

	return 0;
}



/* __END__  {{{1 */
/* vim: foldmethod=marker
 */
