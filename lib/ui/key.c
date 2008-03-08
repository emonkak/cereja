/* Extension: ui.key - key-related API =================================== {{{1
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"








/* Public Functions =================================================== {{{1 */

static int
ui_key__pressedp(lua_State* L)
{
	int vk;
	Crj_ParseArgs(L, "i", &vk);

	lua_pushboolean(L,
	  GetAsyncKeyState(vk) & (1 << (sizeof(SHORT)*CHAR_BIT - 1)));
	return 1;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"_pressedp", ui_key__pressedp},
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
