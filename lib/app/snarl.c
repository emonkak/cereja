/* Extension: app.snarl - Snarl Interface ================================ {{{1
 *
 * Copyright (C) 2008 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"




typedef enum {
	SNARL_SHOW = 1,
	SNARL_HIDE = 2,
	SNARL_UPDATE = 3,
	SNARL_IS_VISIBLE = 4,
	SNARL_GET_VERSION = 5,
	SNARL_REGISTER_CONFIG_WINDOW = 6,
	SNARL_REVOKE_CONFIG_WINDOW = 7,
	SNARL_REGISTER_ALERT = 8,
	SNARL_REVOKE_ALERT = 9,
	SNARL_REGISTER_CONFIG_WINDOW_2 = 10,
	SNARL_EX_SHOW = 32
} SNARL_COMMANDS;

#define _SNARLSTRUCT_BASIC_MEMBERS	\
	SNARL_COMMANDS Cmd;		\
	int Id;				\
	int Timeout;			\
	int LngData2;			\
	char Title[1024];		\
	char Text[1024];		\
	char Icon[1024]

typedef struct {
	_SNARLSTRUCT_BASIC_MEMBERS;
} SNARLSTRUCT;

typedef struct {
	_SNARLSTRUCT_BASIC_MEMBERS;
	char Class[1024];
	char Extra[1024];
	char Extra2[1024];
	int Reserved1;
	int Reserved2;
} SNARLSTRUCTEX;








/* Misc. Functions ==================================================== {{{1 */

#define STRCPY(dest, src)                               \
        (strncpy(dest, src, Crj_NUMBER_OF(dest)),       \
         (dest[Crj_NUMBER_OF(dest) - 1] = '\0'))




static HWND
get_snarl_window(void)
{
	return FindWindow(NULL, "Snarl");
}




static DWORD
send_snarl_message(SNARLSTRUCT* data)
{
	HWND hwnd = get_snarl_window();
	DWORD result = 0;
	COPYDATASTRUCT cds;

	if (IsWindow(hwnd)) {
		cds.dwData = 2;
		cds.cbData = sizeof(*data);
		cds.lpData = data;
		SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cds,
		                   SMTO_ABORTIFHUNG, 100, &result);
	}
	return result;
}








/* Public Functions =================================================== {{{1 */

static int
ui_get_window(lua_State* L)
{
	HWND hwnd;

	hwnd = get_snarl_window();
	lua_pushlightuserdata(L, (IsWindow(hwnd) ? hwnd : NULL));
	return 1;
}




static int
ui_show_message(lua_State* L)
{
	SNARLSTRUCT data;
	const char* title;
	const char* text;
	long timeout = 0;
	const char* icon_path = "";
	HWND hwnd_to_reply = NULL;
	UINT reply_message = WM_NULL;

	Crj_ParseArgs(L, "s s | i s u i",
	              &title, &text,
	              &timeout, &icon_path, &hwnd_to_reply, &reply_message);

	ZeroMemory(&data, sizeof(data));
	data.Cmd = SNARL_SHOW;
	STRCPY(data.Title, title);
	STRCPY(data.Text, text);
	data.Timeout = timeout;
	data.LngData2 = (int)hwnd_to_reply;
	data.Id = reply_message;
	STRCPY(data.Icon, icon_path);

	lua_pushinteger(L, send_snarl_message(&data));
	return 1;
}




static int
ui_hide_message(lua_State* L)
{
	SNARLSTRUCT data;
	int snarl_message_id;

	Crj_ParseArgs(L, "i", &snarl_message_id);

	ZeroMemory(&data, sizeof(data));
	data.Cmd = SNARL_HIDE;
	data.Id = snarl_message_id;

	lua_pushboolean(L, send_snarl_message(&data));
	return 1;
}




static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"get_window", ui_get_window},
	{"show_message", ui_show_message},
	{"hide_message", ui_hide_message},
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
