/* Test: Built-in API (C part) (Extension)
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"
#include <assert.h>




Crj_PUBLIC(int)
Crj_ExtMain(lua_State* L)
{
	static const char* s_mode;
	static int* s_flag;
	CrjExtEvent event;
	int namespace;
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
		Crj_ParseTable(L, namespace, "{=ss =su}",
		               "mode", &s_mode,
		               "flag", &s_flag);

		if (!strcmp(s_mode, "success"))
			*s_flag += 'w'+'a'+'x';
		else
			luaL_error(L, "intended failure");
		break;
	case Crj_EXT_UNLOADING:
		if (!strcmp(s_mode, "success"))
			*s_flag += 'w'+'a'+'n'+'e';
		else
			assert(FALSE);
		break;
	}

	return 0;
}




/* __END__ */
