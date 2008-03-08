/* Extension: shell - misc. shell-related API (C part) =================== {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"

#include <powrprof.h>








/* Misc. ============================================================== {{{1 */

static BOOL
confirm_user_permissionp(const char* message)
{
	return IDYES == MessageBox(NULL, message, Crj_APPNAME,
	                           (MB_YESNO | MB_DEFBUTTON2
	                            | MB_TASKMODAL
	                            | MB_ICONINFORMATION
	                            | MB_TOPMOST));
}




#define SHUTDOWN_REASON 0

static void
set_privilege_to_shutdown(lua_State* L)
{
	HANDLE htoken;
	TOKEN_PRIVILEGES tkp;
	BOOL result;

	result = OpenProcessToken(GetCurrentProcess(),
	                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
	                          &htoken);
	if (!result) goto E_OpenProcessToken;

	result = LookupPrivilegeValue(NULL,
	                              SE_SHUTDOWN_NAME,
	                              &(tkp.Privileges[0].Luid));
	if (!result) goto E_LookupPrivilegeValue;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(htoken, FALSE, &tkp, 0, NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS) goto E_AdjustTokenPrivileges;

	CloseHandle(htoken);
	return;

E_AdjustTokenPrivileges:
E_LookupPrivilegeValue:
	CloseHandle(htoken);
E_OpenProcessToken:
	luaL_error(L, "Adjusting privilege to shutdown is failed");
	return;
}




/* Running start-up programs ---------------------------------------------- {{{
 *
 * NOTE: _shell_run_startup_programs runs as another thread,
 * use Window API only.
 */

/* NTOE: `path' MUST be writable. */
static void
spawn_process(TCHAR* path, BOOL waitp)
{
	BOOL result;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOWNORMAL;

	result = CreateProcess(NULL, path,
	                       NULL, NULL, FALSE,
	                       (CREATE_DEFAULT_ERROR_MODE
	                        | NORMAL_PRIORITY_CLASS),
	                       NULL, NULL,
	                       &si, &pi);
	if (result != 0) {
		CloseHandle(pi.hThread);
		if (waitp)
			WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
	}
	return;
}


enum {
	RRI_NORMAL = 0,
	RRI_DELETE_AFTER_EXECUTE = 1,
	RRI_WAIT_FOR_QUIT = 2
};

static void
run_registry_items(HKEY hkey, LPCTSTR path, int flags)
{
	HKEY hsubkey;
	REGSAM access_rights;
	TCHAR value_name[MAX_PATH];
	DWORD value_name_count;
	TCHAR data[MAX_PATH];
	DWORD data_count;
	DWORD type;
	LONG result;
	int i;

	access_rights = ((flags & RRI_DELETE_AFTER_EXECUTE)
	                 ? (KEY_READ | KEY_SET_VALUE)
	                 : KEY_READ);
	result = RegOpenKeyEx(hkey, path, 0, access_rights, &hsubkey);
	if (result != ERROR_SUCCESS)
		return;

	i = 0;
	while (TRUE) {
		value_name_count = Crj_NUMBER_OF(value_name);
		data_count = sizeof(data);
		result = RegEnumValue(hsubkey, i,
		                      value_name, &value_name_count,
		                      NULL, &type,
		                      (LPBYTE)data, &data_count);
		if (result != ERROR_SUCCESS)
			break;

		if ((type == REG_SZ) || (type == REG_EXPAND_SZ)) {
			data[Crj_NUMBER_OF(data) - 1] = TEXT('\0');
			spawn_process(data, (flags & RRI_WAIT_FOR_QUIT));
		}

		i++;
	}

	if (flags & RRI_DELETE_AFTER_EXECUTE) {
		while (TRUE) {
			value_name_count = Crj_NUMBER_OF(value_name);
			result = RegEnumValue(hsubkey, 0,
			                      value_name, &value_name_count,
			                      NULL, &type,
			                      NULL, NULL);
			if (result != ERROR_SUCCESS)
				break;

			if ((type == REG_SZ) || (type == REG_EXPAND_SZ)) {
				value_name[Crj_NUMBER_OF(value_name) - 1]
				  = TEXT('\0');
				RegDeleteValue(hsubkey, value_name);
			}
		}
	}

	RegCloseKey(hsubkey);
	return;
}


static void
run_folder_items(int folder_csidl)
{
	TCHAR path[MAX_PATH + (1 + 3) + 1];  /* `\', `*.*' and NUL */
	TCHAR* last_dirsep;
	HRESULT hr;
	WIN32_FIND_DATA find_data;
	HANDLE hsearch;

	hr = SHGetFolderPathT(NULL,folder_csidl,NULL,SHGFP_TYPE_CURRENT,path);
	if (!SUCCEEDED(hr))
		return;

	last_dirsep = path + (_tcslen(path) - 1);
	if (*last_dirsep == TEXT('\\')) {
		_tcscat(last_dirsep, TEXT("*.*"));
	} else {
		_tcscat(last_dirsep, TEXT("\\*.*"));
		last_dirsep++;
	}

	hsearch = FindFirstFile(path, &find_data);
	if (hsearch == INVALID_HANDLE_VALUE)
		return;
	*last_dirsep = TEXT('\0');  /* now `path' represents a directory  */
	do {
		if (!(find_data.dwFileAttributes
		      & (FILE_ATTRIBUTE_SYSTEM
		         | FILE_ATTRIBUTE_DIRECTORY
		         | FILE_ATTRIBUTE_HIDDEN)))
		{
			ShellExecuteT(NULL, NULL, find_data.cFileName,
			              NULL, path, SW_SHOWNORMAL);
		}
	} while (FindNextFile(hsearch, &find_data) != 0);
	FindClose(hsearch);
	return;
}


#define PATH_RUN TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define PATH_RUNONCE \
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce")

static DWORD WINAPI
_shell_run_startup_programs(LPVOID parameters __attribute__((unused)))
{
	run_registry_items(HKEY_LOCAL_MACHINE, PATH_RUNONCE,
	                   RRI_DELETE_AFTER_EXECUTE | RRI_WAIT_FOR_QUIT);
	run_registry_items(HKEY_LOCAL_MACHINE, PATH_RUN, RRI_NORMAL);
	run_registry_items(HKEY_CURRENT_USER, PATH_RUN, RRI_NORMAL);

	run_folder_items(CSIDL_COMMON_STARTUP);
	run_folder_items(CSIDL_STARTUP);

	run_registry_items(HKEY_CURRENT_USER, PATH_RUNONCE,
	                   RRI_DELETE_AFTER_EXECUTE);
	return 0;
}

/* }}} */








/* Public Functions =================================================== {{{1 */

static int
shell_message(lua_State* L)
{
	const char* caption;
	const char* text;
	UINT flags = 0;
	Crj_ParseArgs(L, "s s | i", &caption, &text, &flags);

	lua_pushinteger(L, MessageBox(NULL, text, caption, flags));
	return 1;
}


static int
shell_execute(lua_State* L)
{
	const char* operation;
	const char* file;
	const char* parameters = NULL;
	const char* directory = NULL;
	int showcmd = SW_SHOWNORMAL;
	int result;
	if (lua_gettop(L) == 1) {  /* is called with single parameter? */
		operation = NULL;
		Crj_ParseArgs(L, "s", &file);
	} else {
		Crj_ParseArgs(L, "z s | z z i",
		              &operation, &file,
		              &parameters, &directory, &showcmd);
	}

	result = (int)ShellExecute(NULL, operation, file, parameters,
	                           directory, showcmd);
	if (result > 32) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, "ShellExecute is failed: ");
		Crj_PushWindowsError(L, 0);
		lua_concat(L, 2);
	}
	return 1;
}




static int
shell_run_startup_programs(lua_State* L __attribute__((unused)))
{
	CloseHandle(CreateThread(NULL, 0, _shell_run_startup_programs,
	                         NULL, 0, NULL));
	return 0;
}




static int
shell_system_hibernate(lua_State* L)
{
	if (confirm_user_permissionp("Are you sure to hibernate?")) {
		SetSuspendState(TRUE, FALSE, FALSE);
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}

static int
shell_system_lock(lua_State* L)
{
	lua_pushboolean(L, LockWorkStation());
	return 1;
}

static int
shell_system_logoff(lua_State* L)
{
	if (confirm_user_permissionp("Are you sure to log off?")) {
		ExitWindowsEx(EWX_LOGOFF, SHUTDOWN_REASON);
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}

static int
shell_system_reboot(lua_State* L)
{
	if (confirm_user_permissionp("Are you sure to reboot?")) {
		set_privilege_to_shutdown(L);
		ExitWindowsEx(EWX_REBOOT, SHUTDOWN_REASON);
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}

static int
shell_system_shutdown(lua_State* L)
{
	if (confirm_user_permissionp("Are you sure to shutdown?")) {
		set_privilege_to_shutdown(L);
		ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF, SHUTDOWN_REASON);
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}

static int
shell_system_suspend(lua_State* L)
{
	if (confirm_user_permissionp("Are you sure to suspend?")) {
		SetSuspendState(FALSE, FALSE, FALSE);
		lua_pushboolean(L, TRUE);
	} else {
		lua_pushboolean(L, FALSE);
	}
	return 1;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"execute", shell_execute},
	{"message", shell_message},
	{"run_startup_programs", shell_run_startup_programs},
	{"system_hibernate", shell_system_hibernate},
	{"system_lock", shell_system_lock},
	{"system_logoff", shell_system_logoff},
	{"system_reboot", shell_system_reboot},
	{"system_shutdown", shell_system_shutdown},
	{"system_suspend", shell_system_suspend},
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
