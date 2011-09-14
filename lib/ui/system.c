/* Extension: ui.system - system-related API  {{{1
 *
 * __COPYRIGHT_NOTICE__
 * $Id$
 */

#include "cereja.h"
#include <windows.h>
#include <iphlpapi.h>
#include <process.h>



/* Misc.  {{{1 */

unsigned __stdcall lua_call_thread(void* p)
{
	lua_call(p, 0, 0);

	_endthread();
	return 0;
}



/* Public Functions  {{{1 */

static int
ui_system_call(lua_State* L)
{
	int func;
	
	Crj_ParseArgs(L, "O/f", &func);
	lua_State *NL = lua_newthread(L);
	lua_pushvalue(L, func);
	lua_xmove(L, NL, 1);
	lua_pop(L, 1);

	_beginthreadex(NULL, 0, lua_call_thread, NL, 0, NULL);
	return 0;
}


static int
ui_system_sleep(lua_State* L)
{
	DWORD ms;

	Crj_ParseArgs(L, "L", &ms);
	Sleep(ms);

	return 0;
}


static int
ui_system_read_ini_value(lua_State* L)
{
	const char* section;
	const char* key;
	const char* path;
	char buffer[4096];

	Crj_ParseArgs(L, "s s s", &section, &key, &path);

	GetPrivateProfileStringA(section, key, NULL, buffer, sizeof(buffer), path);

	lua_pushstring(L, buffer);
	return 1;
}


static int
ui_system_write_ini_value(lua_State* L)
{
	const char* section;
	const char* key;
	const char* value;
	const char* path;

	Crj_ParseArgs(L, "s s s s", &section, &key, &value, &path);

	if (WritePrivateProfileStringA(section, key, value, path) == 0)
		return luaL_error(L, "WritePrivateProfileString failed");

	return 0;
}


static int
ui_system_get_network_adapter(lua_State* L)
{
	PIP_ADAPTER_INFO pai;
	ULONG len = 0;
	UINT i = 0;

	GetAdaptersInfo(NULL, &len);
	pai = (PIP_ADAPTER_INFO)malloc(len);

	if (GetAdaptersInfo(pai, &len) != NO_ERROR)
		return luaL_error(L, "GetAdaptersInfo failed");

	lua_newtable(L);

	while (pai) {
		i++;
		lua_createtable(L, 0, 7);
		lua_pushstring(L, pai->AdapterName);
		lua_setfield(L, -2, "name");
		lua_pushstring(L, pai->Description);
		lua_setfield(L, -2, "description");

		UINT j;
		char* mac = (char*)malloc(pai->AddressLength * 3);

		for(j = 0; j < pai->AddressLength; j++)
			sprintf(mac + j * 3,
					j == pai->AddressLength - 1 ? "%02x" : "%02x:", pai->Address[j]);
		lua_pushstring(L, mac);
		lua_setfield(L, -2, "mac");
		free(mac);

		lua_pushstring(L, pai->IpAddressList.IpAddress.String);
		lua_setfield(L, -2, "ip");

		lua_pushstring(L, pai->IpAddressList.IpMask.String);
		lua_setfield(L, -2, "mask");

		lua_pushstring(L, pai->GatewayList.IpAddress.String);
		lua_setfield(L, -2, "gateway");

		lua_pushstring(L, pai->DhcpServer.IpAddress.String);
		lua_setfield(L, -2, "dhcp");

		lua_rawseti(L, -2, i);

		pai = pai->Next;
	}

	free(pai);

	return 1;
}


static int
ui_system_get_network_param(lua_State* L)
{
	PFIXED_INFO pfi;
	ULONG len = 0;
	int i = 1;

	GetNetworkParams(NULL, &len);
	pfi = (PFIXED_INFO)malloc(len);

	if (GetNetworkParams(pfi, &len) != NO_ERROR)
		return luaL_error(L, "GetNetworkParams failed");

	PIP_ADDR_STRING pias = pfi->DnsServerList.Next;

	lua_createtable(L, 1, 2);
	lua_pushstring(L, pfi->HostName);
	lua_setfield(L, -2, "host");
	lua_pushstring(L, pfi->DomainName);
	lua_setfield(L, -2, "domain");
	lua_newtable(L);
	lua_pushstring(L, pfi->DnsServerList.IpAddress.String);
	lua_rawseti(L, -2, i);

	while (pias) {
		i++;
		lua_pushstring(L, pias->IpAddress.String);
		lua_rawseti(L, -2, i);
		pias = pias->Next;
	}
	lua_setfield(L, -2, "dns");

	free(pfi);

	return 1;
}


static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"call", ui_system_call},
	{"sleep", ui_system_sleep},
	{"get_network_adapter", ui_system_get_network_adapter},
	{"get_network_param", ui_system_get_network_param},
	{"read_ini_value", ui_system_read_ini_value},
	{"write_ini_value", ui_system_write_ini_value},
	{NULL, NULL}
};



/* Main  {{{1 */

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

/* __END__ {{{1 */
/* vim: foldmethod=marker
*/
