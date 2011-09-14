/* Extension: shell.tray - tray service ================================== {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

/* Overview
 * --------
 *
 * This module/extension provides the services on `tray'.
 * This receives and manages the data sent via Shell_NotifyIcon.
 *
 * In this module/extension, `tray' means the `taskbar notification area'.
 * Although the latter is the right term,
 * it is too long and there is no `taskbar' for cereja.
 *
 *
 * NOTIFYICONDATA [SENT_NID]
 * -------------------------
 *
 * When Shell_TrayWnd receives NOTIFYICONDATA from the system,
 * the version of data is same as the one of the current system
 * even if the version of sent data is different.
 *
 * For example, in Windows XP Professional SP2,
 * the received data is always NOTIFYICONDATAW version 6.0.
 *
 * cereja assumes to be run under Windows XP,
 * so we can assume that NOTIFYICONDATA is always NOTIFYICONDATAW version 6.0.
 *
 *
 * NIS_HIDDED and NIS_SHAREDICON [NIS_FLAGS]
 * -----------------------------------------
 *
 * Although every NOTIFYICONDATA.hIcon must be CopyIcon'ed,
 * most hIcon can be drawn without CopyIcon.
 * So both flags are just ignored.
 * See <http://nicht.s8.xrea.com/2006/02/16/13/58/diary> for the reason.
 */

#include "cereja.h"
#include "shell/tray.h"

#include <docobj.h>








/* Misc. ============================================================== {{{1 */

#define POINTER_FROM(p,offset) (((BYTE*)(p)) + (offset))

#define TCSCPY(dest, src)                              \
        (_tcsncpy(dest, src, Crj_NUMBER_OF(dest)),     \
         (dest[Crj_NUMBER_OF(dest) - 1] = TEXT('\0')))

#define NOTIFYICONDATA_NIS_HIDDENP(nid)	\
        (((nid)->uFlags & NIF_STATE)	\
         && ((nid)->dwState & (nid)->dwStateMask & NIS_HIDDEN))

#define NOTIFYICONDATA_NIS_SHAREDICONP(nid)	\
        (((nid)->uFlags & NIF_STATE)	\
         && ((nid)->dwState & (nid)->dwStateMask & NIS_SHAREDICON))

#define VALID_ICONP(icon) IsWindow((icon)->hWnd)

static BOOL s_AnotherShellRunP;


static void
publish(lua_State* L, DWORD event, int icon_index, UINT flags)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "shell");
	lua_getfield(L, -1, "tray");

	lua_getfield(L, -1, "publish");
	lua_pushnumber(L, event);
	lua_pushnumber(L, icon_index);
	lua_pushnumber(L, flags);
	lua_call(L, 3, 0);

	lua_pop(L, 2);
}




/* Shell Service Objects ---------------------------------------------- {{{2 */
/* SSODL := ShellServiceObjectDelayLoad */

#define REGKEY_SSODL_LIST "cereja:shell.tray:ssodl_list"


#define SUBKEY_SHELLSERVICEOBJECTDELAYED \
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion") \
        TEXT("\\ShellServiceObjectDelayLoad")


/* BUGS: No definition found in MinGW headers. */
#define IOleCommandTarget_Exec(T,pCmdGroup,nCmdID,nCmdExecOpt,pvaIn,pvaOut) \
        (*((T)->lpVtbl->Exec))(T,pCmdGroup,nCmdID,nCmdExecOpt,pvaIn,pvaOut)

#define FORCE_DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
static FORCE_DEFINE_GUID(IID_IOleCommandTarget,
                         0xb722bccb,
                         0x4e68,0x101b,
                         0xa2,0xbc,0x00,0xaa,0x00,0x40,0x47,0x70);


static void
load_shell_service_object_delay_load(lua_State* L)
{
	HKEY hkey_ssodl;
	LONG result;
	int index;

	if (s_AnotherShellRunP)
		return;

	OleInitialize(NULL);

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
	                      SUBKEY_SHELLSERVICEOBJECTDELAYED,
	                      0,
	                      KEY_READ,
	                      &hkey_ssodl);

	lua_newtable(L);  /* REGKEY_SSODL_LIST */
	index = 0;
	while (result == ERROR_SUCCESS) {
		TCHAR value_name_dummy[MAX_PATH];
		DWORD value_name_count_dummy = Crj_NUMBER_OF(value_name_dummy);
		TCHAR data[MAX_PATH];
		DWORD data_count = sizeof(data);
		DWORD type_dummy;

		result = RegEnumValue(hkey_ssodl,
		                      index,
		                      value_name_dummy,&value_name_count_dummy,
		                      NULL,
		                      &type_dummy,
		                      (LPBYTE)data, &data_count);
		if (result == ERROR_SUCCESS) {
			/* BUGS: use p_command_target
			 *       to avoid gcc's -Wstrict-aliasing */
			CLSID clsid;
			IOleCommandTarget* command_target = NULL;
			IOleCommandTarget** p_command_target = &command_target;
			HRESULT hresult;

			data[Crj_NUMBER_OF(data) - 1] = TEXT('\0');
			CLSIDFromString(data, &clsid);
			hresult = CoCreateInstance(&clsid,
			                           NULL,
			                           (CLSCTX_INPROC_SERVER
			                            | CLSCTX_INPROC_HANDLER),
			                           &IID_IOleCommandTarget,
			                           (void**)p_command_target);
			if (SUCCEEDED(hresult)) {
				IOleCommandTarget_Exec(command_target,
				                      &CGID_ShellServiceObject,
				                       OLECMDID_NEW,
				                       OLECMDEXECOPT_DODEFAULT,
				                       NULL, NULL);

				lua_pushlightuserdata(L, command_target);
				lua_rawseti(L, -2, 1 + lua_objlen(L, -2));
			}
		}
		index++;
	}

	RegCloseKey(hkey_ssodl);
	lua_setfield(L, LUA_REGISTRYINDEX, REGKEY_SSODL_LIST);
}


static void
unload_shell_service_object_delay_load(lua_State* L)
{
	if (s_AnotherShellRunP)
		return;

	lua_getfield(L, LUA_REGISTRYINDEX, REGKEY_SSODL_LIST);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		IOleCommandTarget* command_target;

		command_target = lua_touserdata(L, -1);
		IOleCommandTarget_Exec(command_target,
		                       &CGID_ShellServiceObject,
		                       OLECMDID_SAVE,
		                       OLECMDEXECOPT_DODEFAULT,
		                       NULL, NULL);
		lua_pop(L, 1);
	}
	lua_pop(L, 2);

	OleUninitialize();
}




/* Tray Icon ---------------------------------------------------------- {{{2 */

static UINT
TrayIcon_Update(CrjTrayIcon* self, const CrjNOTIFYICONDATA* nid)
{
	UINT flags = 0;

	if (nid->uFlags & NIF_ICON) {  /* BUGS: should CopyIcon */
		if (self->hIcon != nid->hIcon)
			flags |= NIF_ICON;
		self->hIcon = nid->hIcon;
	}

	if (nid->uFlags & NIF_MESSAGE) {
		if (self->uCallbackMessage != nid->uCallbackMessage)
			flags |= NIF_MESSAGE;
		self->uCallbackMessage = nid->uCallbackMessage;
	}

	if (nid->uFlags & NIF_TIP) {
		if (_tcscmp(self->szTip, nid->szTip) != 0)
			flags |= NIF_TIP;
		TCSCPY(self->szTip, nid->szTip);
	}

	if (nid->uFlags & NIF_INFO) {
		if ((_tcscmp(self->szInfo, nid->szInfo) != 0)
		    || (_tcscmp(self->szInfo, nid->szInfo) != 0))
			flags |= NIF_INFO;
		TCSCPY(self->szInfo, nid->szInfo);
		TCSCPY(self->szInfoTitle, nid->szInfoTitle);
		self->dwInfoFlags = nid->dwInfoFlags;
		self->uTimeout = nid->uTimeout;
	}
	return flags;
}


static CrjTrayIcon*
TrayIcon_New(const CrjNOTIFYICONDATA* nid)
{
	CrjTrayIcon* self;

	self = Crj_MALLOC(sizeof(*self));

	self->hWnd = nid->hWnd;
	self->uID = nid->uID;
	self->uCallbackMessage = WM_NULL;
	self->hIcon = NULL;
	self->szTip[0] = TEXT('\0');
	self->szInfo[0] = TEXT('\0');
	self->uTimeout = 0;
	self->szInfoTitle[0] = TEXT('\0');
	self->dwInfoFlags = 0;

	TrayIcon_Update(self, nid);
	return self;
}


static void
TrayIcon_Delete(CrjTrayIcon* self)
{
	Crj_FREE(self);
	return;
}




/* Tray Data ---------------------------------------------------------- {{{2 */

static CrjTrayData*
TrayData_New(void)
{
	CrjTrayData* self;

	self = Crj_MALLOC(sizeof(*self));
	self->count = 0;
	self->icons = NULL;
	return self;
}


static void
TrayData_Delete(CrjTrayData* self)
{
	while (0 <= --(self->count))
		TrayIcon_Delete(self->icons[self->count]);
	Crj_FREE(self->icons);
	Crj_FREE(self);
	return;
}


static int
TrayData_FindIconIndex(CrjTrayData* self, const CrjNOTIFYICONDATA* nid)
{
	int i;

	for (i = 0; i < self->count; i++) {
		if ((self->icons[i]->hWnd == nid->hWnd)
		    && (self->icons[i]->uID == nid->uID))
			return i;
	}

	return -1;
}


static BOOL
TrayData_AddIcon(CrjTrayData* self, const CrjNOTIFYICONDATA* nid,
                 int* r_icon_index, UINT* r_flags)
{
	CrjTrayIcon* icon;

	/* NULL hIcon (e.g. Volume Icon) */
	if (nid->hIcon == NULL)
		return FALSE;

	/* [NIS_FLAGS] */
	if (NOTIFYICONDATA_NIS_HIDDENP(nid) || NOTIFYICONDATA_NIS_SHAREDICONP(nid))
		return FALSE;

	/* Some applications (e.g. Samurize) send NIM_ADD more than once,
	 * so ignore such case.  */
	if (0 <= TrayData_FindIconIndex(self, nid))
		return FALSE;

	icon = TrayIcon_New(nid);
	self->icons = Crj_REALLOC(self->icons,
	                          ((self->count + 1)
	                           * sizeof(self->icons[0])));
	self->icons[self->count] = icon;
	self->count += 1;

	/* Some applications don't send szTip until WM_MOUSEMOVE sent. */
	PostMessage(icon->hWnd, icon->uCallbackMessage,
	            (WPARAM)(icon->uID), WM_MOUSEMOVE);
	*r_icon_index = self->count;
	*r_flags = (UINT)(-1);  /* treat all members has been changed. */
	return TRUE;
}


static BOOL
TrayData_RemoveIcon(CrjTrayData* self, const CrjNOTIFYICONDATA* nid,
                    int* r_icon_index, UINT* r_flags)
{
	int i;

	i = TrayData_FindIconIndex(self, nid);
	if (i < 0)
		return FALSE;

	TrayIcon_Delete(self->icons[i]);

	for (i = i + 1; i < self->count; i++)
		self->icons[i - 1] = self->icons[i];
	self->count -= 1;
	/* Crj_REALLOC ... */
	*r_icon_index = 0;  /* FIXME: nonsense icon_index */
	*r_flags = 0;  /* treat no member has been changed. */
	return TRUE;
}


static BOOL
TrayData_UpdateIcon(CrjTrayData* self, const CrjNOTIFYICONDATA* nid,
                    int* r_icon_index, UINT* r_flags)
{
	int i;

	i = TrayData_FindIconIndex(self, nid);
	if (i < 0)
		return FALSE;

	*r_icon_index = i+1;
	*r_flags = TrayIcon_Update(self->icons[i], nid);
	return TRUE;
}


static BOOL
TrayData_CleanIcon(CrjTrayData* self, int target_index)
{
	int from;
	int to;
	BOOL cleanedp;

	if ((0 <= target_index) && (target_index < self->count)) {
		if (VALID_ICONP(self->icons[target_index]))
			return FALSE;
	}

	from = 0;
	to = 0;
	while (from < self->count) {
		if (!VALID_ICONP(self->icons[from])) {
			TrayIcon_Delete(self->icons[from]);
			from++;
			continue;
		}

		self->icons[to] = self->icons[from];
		from++;
		to++;
	}

	cleanedp = (self->count != to);
	self->count = to;
	return cleanedp;
}




/* Tray Window -------------------------------------------------------- {{{2 */

#define TRAY_WINDOW_CLASS_NORMAL_U "Shell_TrayWnd"
#define TRAY_WINDOW_CLASS_NORMAL TEXT(TRAY_WINDOW_CLASS_NORMAL_U)
#define TRAY_WINDOW_CLASS_ALTERNATIVE TEXT("cereja:shell.tray")
#define TRAY_WINDOW_CLASS (s_AnotherShellRunP                   \
                           ? TRAY_WINDOW_CLASS_ALTERNATIVE      \
                           : TRAY_WINDOW_CLASS_NORMAL)
#define TRAY_WINDOW_TITLE NULL

static HWND s_TrayWindow = NULL;
static CrjTrayData* s_TrayData = NULL;


/* Hook to get messages to the tray of the other shell ````````````````` {{{ */

static HMODULE s_HookDLLModule = NULL;


static const char*
Hook_Start(lua_State* L, HWND callback_window)
{
	HWND tray_window;
	FARPROC start_hook;
	FARPROC end_hook;
	const char* hook_dll_path;
	const char* error_message;
	const char* GET_HOOK_DLL_PATH_SCRIPT
	  = "return package.findfile('shell\\\\tray-hook', package.cpath)";

	tray_window = FindWindow(TRAY_WINDOW_CLASS_NORMAL_U, NULL);
	if (tray_window == NULL) {
		error_message = "Tray window is not found.";
		goto E_find_tray_window;
	}

	if (!!luaL_dostring(L, GET_HOOK_DLL_PATH_SCRIPT)) {
		error_message = "Hook DLL is not found (#1).";
		goto E_get_hook_dll_path;
	}
	hook_dll_path = lua_tostring(L, -1);
	if (hook_dll_path == NULL) {
		error_message = "Hook DLL is not found (#2).";
		goto E_get_hook_dll_path;
	}

	s_HookDLLModule = LoadLibrary(hook_dll_path);
	lua_pop(L, 1);  /* hook_dll_path */
	if (s_HookDLLModule == NULL) {
		error_message = "Hook DLL is failed to load (#1).";
		goto E_load_hook_dll;
	}

	start_hook=GetProcAddress(s_HookDLLModule,"_Crj_ShellTray_StartHook");
	end_hook = GetProcAddress(s_HookDLLModule, "_Crj_ShellTray_EndHook");
	if ((start_hook == NULL) || (end_hook == NULL)) {
		error_message = "Hook DLL seems to be invalid one.";
		goto E_get_procedures;
	}

	if (!(*((BOOL(*)(HWND,HWND))start_hook))(tray_window,callback_window)){
		error_message = "Hook DLL failed to hook.";
		goto E_start_hook;
	}

	return NULL;

E_start_hook:
E_get_procedures:
	FreeLibrary(s_HookDLLModule);
	s_HookDLLModule = NULL;
E_load_hook_dll:
E_get_hook_dll_path:
E_find_tray_window:
	return error_message;
}


static const char*
Hook_End(void)
{
	FARPROC end_hook;
	BOOL result;
	const char* error_message = NULL;

	end_hook = GetProcAddress(s_HookDLLModule, "_Crj_ShellTray_EndHook");
	if (end_hook != NULL) {
		result = (*((BOOL (*)(void))end_hook))();
		if (!result)
			error_message = "Hook DLL is failed to unhook.";
	} else {
		error_message = "Hook DLL seems to be invalid one (#2).";
	}

	FreeLibrary(s_HookDLLModule);
	s_HookDLLModule = NULL;
	return error_message;
}

/* }}} */


static BOOL
tray_message_proc(lua_State* L, DWORD message, const CrjNOTIFYICONDATA* nid)
{
	BOOL processedp;
	int icon_index;
	UINT flags;

	if (nid->cbSize != sizeof(CrjNOTIFYICONDATA))  /* [SENT_NID] */
		return FALSE;

	switch (message) {
	default:
		return FALSE;
	case NIM_ADD:
		processedp = TrayData_AddIcon(s_TrayData, nid,
		                              &icon_index, &flags);
		break;
	case NIM_DELETE:
		processedp = TrayData_RemoveIcon(s_TrayData, nid,
		                                 &icon_index, &flags);
		break;
	case NIM_MODIFY:
		processedp = TrayData_UpdateIcon(s_TrayData, nid,
		                                 &icon_index, &flags);

		if (!processedp) {
			processedp = TrayData_AddIcon(s_TrayData, nid,
		                                      &icon_index, &flags);
			message = NIM_ADD;
		}
		break;
	case NIM_SETFOCUS:
		return FALSE;  /* FIXME: NIY */
	case NIM_SETVERSION:
		return FALSE;  /* FIXME: NIY */
	}

	if (processedp)
		publish(L, message, icon_index, flags);
	return TRUE;
}

static LRESULT CALLBACK
tray_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static lua_State* s_L;

	switch (msg) {
	default:
		break;

	case WM_CREATE:
		s_L = (lua_State*)(((CREATESTRUCT*)lp)->lpCreateParams);
		break;

	case WM_COPYDATA: {
		COPYDATASTRUCT* cds;

		cds = (COPYDATASTRUCT*)lp;
		if (cds->dwData == 1) {
			DWORD tray_msg;
			const CrjNOTIFYICONDATA* nid;

			tray_msg = *(DWORD*)POINTER_FROM(cds->lpData, 4);
			nid = (CrjNOTIFYICONDATA*)POINTER_FROM(cds->lpData, 8);
			return tray_message_proc(s_L, tray_msg, nid);
		}
		} break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}


static void
create_tray_window(lua_State* L, HMODULE hmodule)
{
	WNDCLASS wc;
	const char* error_message;

	s_TrayData = TrayData_New();

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = tray_window_proc;
	wc.hInstance = hmodule;
	wc.lpszClassName = (s_AnotherShellRunP
	                    ? TRAY_WINDOW_CLASS_ALTERNATIVE
	                    : TRAY_WINDOW_CLASS_NORMAL);
	if (!RegisterClass(&wc)) {
		error_message = "RegisterClass";
		goto E_RegisterClass;
	}
	s_TrayWindow = CreateWindowEx((WS_EX_NOACTIVATE
				       | WS_EX_TOOLWINDOW),
				      TRAY_WINDOW_CLASS,
				      TRAY_WINDOW_TITLE,
				      0,
				      0, 0, 0, 0,
				      HWND_MESSAGE,
				      NULL,
				      hmodule,
				      L);
	if (s_TrayWindow == NULL) {
		error_message = "CreateWindowEx";
		goto E_CreateWindowEx;
	}

	if (s_AnotherShellRunP) {
		error_message = Hook_Start(L, s_TrayWindow);
		if (error_message != NULL)
			goto E_Hook_Start;
	}

	PostMessage(HWND_BROADCAST,
		    RegisterWindowMessage(TEXT("TaskbarCreated")),
		    0, 0);
	return;

E_Hook_Start:
	DestroyWindow(s_TrayWindow);
	s_TrayWindow = NULL;
E_CreateWindowEx:
	UnregisterClass(TRAY_WINDOW_CLASS, hmodule);
E_RegisterClass:
	TrayData_Delete(s_TrayData);
	s_TrayData = NULL;

	lua_pushstring(L, error_message);
	lua_pushstring(L, " is failed (");
	Crj_PushWindowsError(L, 0);
	lua_pushstring(L, ")");
	lua_concat(L, 4);
	lua_error(L);
	return;
}


static void
destroy_tray_window(lua_State* L __attribute__((unused)), HMODULE hmodule)
{
	const char* error_message = NULL;

	if (s_AnotherShellRunP)
		error_message = Hook_End();

	DestroyWindow(s_TrayWindow);
	s_TrayWindow = NULL;

	UnregisterClass(TRAY_WINDOW_CLASS, hmodule);

	TrayData_Delete(s_TrayData);
	s_TrayData = NULL;

	if (error_message != NULL) {
		lua_pushstring(L, error_message);
		lua_error(L);
	}
}








/* Public Functions =================================================== {{{1 */

#define VALID_INDEXP(index,count) (((1<=(index)) && ((index)<=(count))) \
                                   || ((-(count)<=(index)) && ((index)<=-1)))
#define ADJUST_INDEX(index,count) ((1<=(index)) ? (index)-1 : (index)+(count))


static int
shell_tray_clean_icon(lua_State* L)
{
	int index;
	Crj_ParseArgs(L, "i", &index);

	if (!VALID_INDEXP(index, s_TrayData->count))
		return 0;
	index = ADJUST_INDEX(index, s_TrayData->count);

	if (TrayData_CleanIcon(s_TrayData, index))
		publish(L, NIM_DELETE, 0, 0);  /* FIXME: nonsense icon_index */
	return 0;
}


static int
shell_tray_get_icon_count(lua_State* L)
{
	lua_pushinteger(L, s_TrayData->count);
	return 1;
}

static int
shell_tray_get_icon_data(lua_State* L)
{
	#define DEFINE_UTF8_STRING(var,original) \
		char var[Utf8_NATIVE_RATIO * (_tcslen(original)+1)]; \
		if (Utf8_FromNative(var,Crj_NUMBER_OF(var),original,-1) == 0) \
			var[0] = '\0';
	int index;
	const char* member;
	Crj_ParseArgs(L, "i s", &index, &member);

	if (!VALID_INDEXP(index, s_TrayData->count))
		luaL_error(L, "Invalid icon index `%d'", index);
	index = ADJUST_INDEX(index, s_TrayData->count);

	if (!strcmp(member, "hWnd")) {
		lua_pushlightuserdata(L, s_TrayData->icons[index]->hWnd);
	} else if (!strcmp(member, "uID")) {
		lua_pushinteger(L, s_TrayData->icons[index]->uID);
	} else if (!strcmp(member, "uCallbackMessage")) {
		lua_pushinteger(L, s_TrayData->icons[index]->uCallbackMessage);
	} else if (!strcmp(member, "hIcon")) {
		lua_pushlightuserdata(L, s_TrayData->icons[index]->hIcon);
	} else if (!strcmp(member, "szTip")) {
		DEFINE_UTF8_STRING(szTipU,  s_TrayData->icons[index]->szTip);
		lua_pushstring(L, szTipU);
	} else if (!strcmp(member, "szInfo")) {
		DEFINE_UTF8_STRING(szInfoU, s_TrayData->icons[index]->szInfo);
		lua_pushstring(L, szInfoU);
	} else if (!strcmp(member, "uTimeout")) {
		lua_pushinteger(L, s_TrayData->icons[index]->uTimeout);
	} else if (!strcmp(member, "szInfoTitle")) {
		DEFINE_UTF8_STRING(szInfoTitleU,
		                   s_TrayData->icons[index]->szInfoTitle);
		lua_pushstring(L, szInfoTitleU);
	} else if (!strcmp(member, "dwInfoFlags")) {
		lua_pushinteger(L, s_TrayData->icons[index]->dwInfoFlags);
	} else {
		luaL_error(L, "Invalid member name `%s'", member);
	}
	return 1;
}


static int
shell_tray_move_icon(lua_State* L)
{
	int index;
	int dest;
	int step;
	int i;
	CrjTrayIcon* tmp;
	Crj_ParseArgs(L, "i i", &index, &dest);

	if (!VALID_INDEXP(index, s_TrayData->count)) return 0;
	if (!VALID_INDEXP(dest, s_TrayData->count)) return 0;

	index = ADJUST_INDEX(index, s_TrayData->count);
	dest = ADJUST_INDEX(dest, s_TrayData->count);
	if (index == dest) return 0;

	step = (index < dest ? 1 : -1);
	tmp = s_TrayData->icons[index];
	for (i = index; i != dest; i += step)
		s_TrayData->icons[i] = s_TrayData->icons[i+step];
	s_TrayData->icons[dest] = tmp;

	publish(L, NIM_MODIFY, 1, 0);  /* FIXME: what icon_index to be sent? */
	return 0;
}


static int
shell_tray_reset_icons(lua_State* L)
{
	int count;

	count = s_TrayData->count;
	s_TrayData->count = 0;
		/* FIXME: should publish for each deletion? */
		/* FIXME: nonsense icon_index */
	publish(L, NIM_DELETE, 0, 0);
	while (0 <= --count)
		TrayIcon_Delete(s_TrayData->icons[count]);
	/* Crj_REALLOC ... */

	PostMessage(HWND_BROADCAST,
	            RegisterWindowMessage(TEXT("TaskbarCreated")),
	            0,
	            0);
	return 0;
}


static int
shell_tray__reorder_icons(lua_State* L)
{
	int table;
	int i;
	CrjTrayIcon* icons[s_TrayData->count];
	Crj_ParseArgs(L, "O/t", &table);
	if ((int)lua_objlen(L, table) != s_TrayData->count) {
		luaL_error(L, "Table length must be %d, but got %d.",
		           s_TrayData->count, lua_objlen(L, table));
	}
	memcpy(icons, s_TrayData->icons, sizeof(icons[0]) * s_TrayData->count);

	for (i = 1; i <= s_TrayData->count; i++) {
		lua_rawgeti(L, table, i);
		s_TrayData->icons[i - 1] = icons[lua_tointeger(L, -1) - 1];
		lua_pop(L, 1);
	}

	publish(L, NIM_MODIFY, 1, 0);  /* FIXME: what icon_index to be sent? */
	return 0;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"clean_icon", shell_tray_clean_icon},
	{"get_icon_count", shell_tray_get_icon_count},
	{"get_icon_data", shell_tray_get_icon_data},
	{"move_icon", shell_tray_move_icon},
	{"reset_icons", shell_tray_reset_icons},
	{"_reorder_icons", shell_tray__reorder_icons},
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
		s_AnotherShellRunP = (FindWindow(TRAY_WINDOW_CLASS_NORMAL_U,
		                                 NULL)
		                      != NULL);

		create_tray_window(L, hmodule);

		lua_getfield(L, 1, "namespace");
		luaL_register(L, NULL, PUBLIC_FUNCTIONS);
		lua_pushlightuserdata(L, s_TrayData);
		lua_setfield(L, -2, "data");
		#define REGISTER_CONSTANT(id) \
			{ \
				lua_pushinteger(L, id); \
				lua_setfield(L, -2, #id); \
			}
		REGISTER_CONSTANT(NIM_ADD)
		REGISTER_CONSTANT(NIM_DELETE)
		REGISTER_CONSTANT(NIM_MODIFY)
		REGISTER_CONSTANT(NIF_ICON)
		REGISTER_CONSTANT(NIF_INFO)
		REGISTER_CONSTANT(NIF_MESSAGE)
		REGISTER_CONSTANT(NIF_TIP)
		#undef REGISTER_CONSTANT
		lua_pop(L, 1);

		load_shell_service_object_delay_load(L);
		break;
	case Crj_EXT_UNLOADING:
		unload_shell_service_object_delay_load(L);

		destroy_tray_window(L, hmodule);
		break;
	}

	return 0;
}

/* __END__ */
/* vim: foldmethod=marker
 */
