/* Extension: __NAME__ - __DESCRIPTION__ ================================= {{{1
 *
 * __COPYRIGHT_NOTICE__
 * $Id$
 */

#include "cereja.h"








/* Misc. ============================================================== {{{1 */








/* Public Functions =================================================== {{{1 */

static int
foo_bar_get_baz(lua_State* L)
{
	return 0;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"get_baz", foo_bar_get_baz},
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
