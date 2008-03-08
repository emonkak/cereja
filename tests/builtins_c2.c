/* Test: Built-in API (C part) (#2 / using some of the Lua part)
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"




TEST_PROC(cereja_load_extension_implicit_unloading)
{
	lua_State* L;
	int flag;
	int result;

	L = luaL_newstate();
	TEST_ASSERT_PTR_NOT_NULL(L);
	luaL_openlibs(L);
	_Crj_OpenBuiltins(L);

	luaL_dostring(L, "package.cpath = '.\\\\?.dll;' .. package.cpath");
	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "load_extension");
	Crj_BuildValues(L, "s {=ss =su}",
	                ".\\tests\\builtins_c_dll",
	                "mode", "success",
	                "flag", &flag);
	flag = '`';
	result = lua_pcall(L, 2, 1, 0);

	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x'), flag);

	/* implicit unloading:
	 * load_extension does memoize, so there is an implicit unloading. */
	_Crj_CloseBuiltins(L);
	lua_close(L);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x') + ('w'+'a'+'n'+'e'), flag);
}




/* __END__ */
