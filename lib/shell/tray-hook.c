/* shell.tray-hook - bridge for messages from Shell_NotifyIcon ================
 *
 * Copyright (C) 2007 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"




#define SHARED(X) X __attribute__((section(".CerejaShellTray"), shared))

static HMODULE g_HModule = NULL;
static HHOOK SHARED(g_HHook) = NULL;
static HWND SHARED(g_TrayWindow) = NULL;
static HWND SHARED(g_CallbackWindow) = NULL;








static LRESULT CALLBACK
HookProc(int ncode, WPARAM wp, LPARAM lp)
{
	if (ncode == HC_ACTION) {
		CWPSTRUCT* cwps = (CWPSTRUCT*)lp;

		if ((cwps->hwnd == g_TrayWindow)
		    && (cwps->message == WM_COPYDATA))
		{
			SendMessage(g_CallbackWindow,
			            WM_COPYDATA,
			            cwps->wParam,
			            cwps->lParam);
		}
	}
	return CallNextHookEx(g_HHook, ncode, wp, lp);
}




Crj_PUBLIC(BOOL)
_Crj_ShellTray_StartHook(HWND tray_window, HWND callback_window)
{
	if (g_HModule == NULL) return FALSE;
	if (g_HHook != NULL) return FALSE;
	if (tray_window == callback_window) return FALSE;

	g_HHook = SetWindowsHookEx(WH_CALLWNDPROC, HookProc, g_HModule,
	                           GetWindowThreadProcessId(tray_window,NULL));
	if (g_HHook == NULL) return FALSE;

	g_TrayWindow = tray_window;
	g_CallbackWindow = callback_window;
	return TRUE;
}


Crj_PUBLIC(BOOL)
_Crj_ShellTray_EndHook(void)
{
	BOOL succeededp;

	if (g_HHook == NULL) return FALSE;

	succeededp = (UnhookWindowsHookEx(g_HHook) != 0);
	g_HHook = NULL;
	g_TrayWindow = NULL;
	g_CallbackWindow = NULL;
	return succeededp;
}




BOOL WINAPI
DllMain(HINSTANCE dll_instance,
        DWORD reason,
        LPVOID reserved __attribute__((unused)))
{
	switch (reason) {
	default:
		/* NOP */
		break;
	case DLL_PROCESS_ATTACH:
		g_HModule = dll_instance;
		DisableThreadLibraryCalls(dll_instance);
		break;
	case DLL_PROCESS_DETACH:
		_Crj_ShellTray_EndHook();
		break;
	}

	return TRUE;
}

/* __END__ */
