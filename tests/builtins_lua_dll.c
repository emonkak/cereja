/* Test: Built-in API (Lua part) (Extension for Lua-part test)
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"
#include <assert.h>




Crj_PUBLIC(int)
Crj_ExtMain(lua_State* L)
{
	CrjExtEvent event;
	int namespace;
	const char* mode;
	int value;
	Crj_ParseArgs(L, "{=si}", "event", &event);
	lua_getfield(L, 1, "namespace");
	namespace = (lua_istable(L, 2) ? 2 : 0);

	switch (event) {
	default:
		luaL_error(L, "Event %d is unknown", event);
		break;
	case Crj_EXT_LOADING:
		if (!namespace)
		  luaL_error(L, "Namespace must be given on Crj_EXT_LOADING");
		Crj_ParseTable(L, namespace, "{=ss =si}",
		               "mode", &mode,
		               "flag", &value);

		if (!!strcmp(mode, "success"))
			luaL_error(L, "intended failure");

		lua_pushinteger(L, value + 'w'+'a'+'x');
		lua_setfield(L, namespace, "flag");
		break;
	case Crj_EXT_UNLOADING:
		/* Do nothing:
		 * The Lua state is being closed now,
		 * and no Lua code will run after close.
		 * So the behavior in here cannot be tested by Lua code.
		 */
		break;
	}

	return 0;
}




/* __END__ */
