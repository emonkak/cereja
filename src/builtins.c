/* Built-in API (C part)
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"
#include <sys/stat.h>








/* cereja part ======================================================== {{{1 */

/* cereja._load_extension
 *
 * Load the given extension.
 *
 * Parameters:
 *   extpath (string)
 *     A path to the extension to load.
 *
 *   namespace (table)
 *     The namespace of the caller module.
 *
 * Return values:
 *   magic value (light userdata (HMODULE))
 *     HMODULE of the loaded extension.  This value must be memoized to unload
 *     the extension at the end of Lua session.
 */

static int
cereja__load_extension(lua_State* L)
{
	const char* extpath;
	int namespace;
	HMODULE hmodule;
	Crj_ExtMainF* ext_main;
	Crj_ParseArgs(L, "s O/t", &extpath, &namespace);

	hmodule = LoadLibrary(extpath);
	if (hmodule == NULL)
		luaL_error(L, "Extension `%s' is failed to load", extpath);
	ext_main = (Crj_ExtMainF*)GetProcAddress(hmodule, "Crj_ExtMain");
	if (ext_main == NULL)
		luaL_error(L, "DLL `%s' is not a valid extension", extpath);

	lua_pushcfunction(L, ext_main);
	Crj_BuildValues(L, "{=si =su =sO}",
	                "event", Crj_EXT_LOADING,
	                "hmodule", hmodule,
	                "namespace", namespace);
	if (Crj_PCall(L, 1, 0) != 0) {
		FreeLibrary(hmodule);
		lua_error(L);
	}

	lua_pushlightuserdata(L, hmodule);
	return 1;
}




static int
cereja_exit(lua_State* L __attribute__((unused)))
{
	PostMessage(FindWindow(Crj_MAIN_WINDOW_CLASS, Crj_MAIN_WINDOW_TITLE),
	            Crj_WM_EXIT, 0, 0);
	return 0;
}




static int
cereja_notice(lua_State* L)
{
	int category;
	const char* noticer;
	const char* message;
	Crj_ParseArgs(L, "i s s", &category, &noticer, &message);

	Crj_Notice(L, category, noticer, message);
	return 0;
}




static const luaL_Reg REG_CEREJA[] = {
	{"_load_extension", cereja__load_extension},
	{"exit", cereja_exit},
	{"notice", cereja_notice},
	{NULL, NULL}
};








/* Patch part ========================================================= {{{1 */

	/* replacement of the Lua's one -- support multibyte strings */
static int
os_getenv(lua_State* L)
{
	const char* varname;

	varname = luaL_checkstring(L, 1);
	lua_pushstring(L, Utf8_GetEnvA(varname));  /* push nil if not found */
	return 1;
}

static int
os_pathexistsp(lua_State* L)
{
	const char* path;
	struct _stat info;
	Crj_ParseArgs(L, "s", &path);

	lua_pushboolean(L, (_stat(path, &info) == 0));
	return 1;
}

static const luaL_Reg REG_OS[] = {
	{"getenv", os_getenv},
	{"pathexistsp", os_pathexistsp},
	{NULL, NULL}
};




#define DEFINE_BIT_FUNCTION(name, OP)               \
        static int                                      \
        bit_##name(lua_State* L)                        \
        {                                               \
                lua_Number a;                           \
                lua_Number b;                           \
                long ai;                                \
                long bi;                                \
                Crj_ParseArgs(L, "n n", &a, &b);        \
                                                        \
                lua_number2integer(ai, a);              \
                lua_number2integer(bi, b);              \
                                                        \
                lua_pushinteger(L, ai OP bi);           \
                return 1;                               \
        }

DEFINE_BIT_FUNCTION(and, &)
DEFINE_BIT_FUNCTION(lshift, <<)
DEFINE_BIT_FUNCTION(or, |)
DEFINE_BIT_FUNCTION(rshift, >>)
DEFINE_BIT_FUNCTION(xor, ^)

static int
bit_not(lua_State* L)
{
	lua_Number a;
	long ai;
	Crj_ParseArgs(L, "n", &a);

	lua_number2integer(ai, a);

	lua_pushinteger(L, ~ai);
	return 1;
}

static const luaL_Reg REG_BIT[] = {
	{"band", bit_and},
	{"blshift", bit_lshift},
	{"bnot", bit_not},
	{"bor", bit_or},
	{"brshift", bit_rshift},
	{"bxor", bit_xor},
	{NULL, NULL}
};








/* _Crj_OpenBuiltins ================================================== {{{1 */

static void
get_libcereja_dir(lua_State* L, char* buf, int size)
{
	int length;

	/* GetModuleFileName returns the string length without the last NUL. */
	length = GetModuleFileName(GetModuleHandle(TEXT("libcereja")),
	                           buf, size);
	if (length == 0)
		luaL_error(L, "libcereja-directory cannot be obtained");
	if (size <= length+1) {
	  buf[size - 1] = '\0';
	  luaL_error(
	    L,
	    "libcereja-directory: insufficient buffer (given %d, copied `%s')",
	    size, buf
	  );
	}
}

static void
load_lua_part(lua_State* L)
{
	char path[MAX_PATH * Utf8_NATIVE_RATIO];
	char* p;

	get_libcereja_dir(L, path, Crj_NUMBER_OF(path));
	p = strrchr(path, '.');
	if ((p == NULL) || (strlen(p) != 4)) {  /* 4 = `.dll' */
		luaL_error(L, "libcereja-directory is somewhat strange (`%s')",
		           path);
	}
	strcpy(p, ".lua");  /* modify `.dll' as `.lua' */

	if (luaL_loadfile(L, path) || Crj_PCall(L, 0, 0))
		lua_error(L);
	return;
}




Crj_PUBLIC(void)
_Crj_OpenBuiltins(lua_State* L)
{
	/* patch part */
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, REG_BIT);
	lua_pop(L, 1);  /* global environment */

	lua_getglobal(L, "os");
	luaL_register(L, NULL, REG_OS);
	lua_pop(L, 1);  /* os */

	lua_pushlightuserdata(L, NULL);
	lua_setglobal(L, "NULL");


	/* cereja part */
	lua_newtable(L);
	lua_setglobal(L, "cereja");

	lua_getglobal(L, "cereja");
	{
		char path[MAX_PATH * Utf8_NATIVE_RATIO];

		Crj_UserDirectory(L, path, Crj_NUMBER_OF(path));
		lua_pushstring(L, path);
		lua_setfield(L, -2, "user_directory");

		Crj_SystemDirectory(L, path, Crj_NUMBER_OF(path));
		lua_pushstring(L, path);
		lua_setfield(L, -2, "system_directory");

		lua_pushboolean(L, (FindWindow("Shell_TrayWnd",NULL) != NULL));
		lua_setfield(L, -2, "on_other_shellp");

		lua_pushinteger(L, Crj_NOTICE_DEBUG);
		lua_setfield(L, -2, "NOTICE_DEBUG");
		lua_pushinteger(L, Crj_NOTICE_INFO);
		lua_setfield(L, -2, "NOTICE_INFO");
		lua_pushinteger(L, Crj_NOTICE_WARNING);
		lua_setfield(L, -2, "NOTICE_WARNING");
		lua_pushinteger(L, Crj_NOTICE_ERROR);
		lua_setfield(L, -2, "NOTICE_ERROR");

		luaL_register(L, NULL, REG_CEREJA);

		load_lua_part(L);
	}
	lua_pop(L, 1);
}








/* _Crj_CloseBuiltins ================================================= {{{1 */

static void
unload_an_extension(lua_State* L, HMODULE hmodule)
{
	Crj_ExtMainF* ext_main;

	ext_main = (Crj_ExtMainF*)GetProcAddress(hmodule, "Crj_ExtMain");

	lua_pushcfunction(L, ext_main);
	Crj_BuildValues(L, "{=si =su}",
	                "event", Crj_EXT_UNLOADING,
	                "hmodule", hmodule);
	if (Crj_PCall(L, 1, 0) != 0) {
		Crj_NoticeError(L, "core", lua_tostring(L, -1));
		lua_pop(L, 1);  /* error message */
	}

	/* FreeLibrary(hmodule);
	 * -- Don't touch, because it will be automatically FreeLibrary'ed
	 *    when the process exits.
	 */
}

static void
unload_all_extensions(lua_State* L)
{
	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "_loaded_extensions");

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {  /* key=extname, value=hmodule */
		unload_an_extension(L, lua_touserdata(L, -1));

		lua_pop(L, 1);
	}

	lua_pop(L, 2);
}


Crj_PUBLIC(void)
_Crj_CloseBuiltins(lua_State* L)
{
	unload_all_extensions(L);
}

/* __END__ */
/* vim: foldmethod=marker
 */
