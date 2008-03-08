/* Test: Built-in API (C part)
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"




TEST_PROC(cereja__load_extension_success)
{
	lua_State* L;
	int flag;
	int result;

	L = luaL_newstate();
	TEST_ASSERT_PTR_NOT_NULL(L);
	luaL_openlibs(L);
	_Crj_OpenBuiltins(L);

	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "_load_extension");
	Crj_BuildValues(L, "s {=ss =su}",
	                ".\\tests\\builtins_c_dll.dll",
	                "mode", "success",
	                "flag", &flag);
	flag = '`';
	result = lua_pcall(L, 2, 1, 0);

	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x'), flag);

	/* implicit unloading:
	 * _load_extension doesn't memoize,
	 * so there is no implicit unloading. */
	_Crj_CloseBuiltins(L);
	lua_close(L);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x') /*+('w'+'a'+'n'+'e')*/, flag);
}


TEST_PROC(cereja__load_extension_twice)
{
	lua_State* L;
	int flag;
	int result;

	L = luaL_newstate();
	TEST_ASSERT_PTR_NOT_NULL(L);
	luaL_openlibs(L);
	_Crj_OpenBuiltins(L);

	/* (1) load test extension */
	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "_load_extension");
	Crj_BuildValues(L, "s {=ss =su}",
	                ".\\tests\\builtins_c_dll.dll",
	                "mode", "success",
	                "flag", &flag);
	flag = '`';
	result = lua_pcall(L, 2, 1, 0);

	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x'), flag);

	/* (2) load test extension again. */
	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "_load_extension");
	Crj_BuildValues(L, "s {=ss =su}",
	                ".\\tests\\builtins_c_dll.dll",
	                "mode", "success",
	                "flag", &flag);
	/* flag -- don't initizlize */
	result = lua_pcall(L, 2, 1, 0);

	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT('`' + ('w'+'a'+'x') + ('w'+'a'+'x'), flag);
		/* Because `_load_extension' doesn't check
		 * whether the given extension has been loaded before.
		 */

	/* (3) implicit unloading:
	 * _load_extension doesn't memoize,
	 * so there is no implicit unloading. */
	_Crj_CloseBuiltins(L);
	lua_close(L);
	TEST_ASSERT_INT(('`' + ('w'+'a'+'x') + ('w'+'a'+'x')
	                 /* +('w'+'a'+'n'+'e') +('w'+'a'+'n'+'e') */ ),
	                flag);
}


TEST_PROC(cereja__load_extension_failure)
{
	lua_State* L;
	int flag;
	int result;

	L = luaL_newstate();
	TEST_ASSERT_PTR_NOT_NULL(L);
	luaL_openlibs(L);
	_Crj_OpenBuiltins(L);

	lua_getglobal(L, "cereja");
	lua_getfield(L, -1, "_load_extension");
	Crj_BuildValues(L, "s {=ss =su}",
	                ".\\tests\\builtins_c_dll.dll",
	                "mode", "failure",
	                "flag", &flag);
	flag = 0;
	result = lua_pcall(L, 2, 1, 0);

	TEST_ASSERT(0 != result);
	TEST_ASSERT_INT(0, flag);

	_Crj_CloseBuiltins(L);
	lua_close(L);

	TEST_ASSERT_INT(0, flag);
}




/* __END__ */
