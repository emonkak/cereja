/* Extension: ui.sound - sound-related API  {{{1
 *
 * __COPYRIGHT_NOTICE__
 * $Id$
 */

#include "cereja.h"
#include <windows.h>
#include <mmsystem.h>
#include <math.h>



/* Misc.  {{{1 */

static BOOL
mixer_open(HMIXER* mixer)
{
	return mixerOpen(mixer, 0, 0, 0, MIXER_OBJECTF_HMIXER) == 0;
}


static BOOL
mixer_close(HMIXER mixer)
{
	return mixerClose(mixer) == 0;
}


static BOOL
mixer_get_cotrol(HMIXER mixer, MIXERCONTROL* mc, DWORD control_type)
{
	MIXERLINE ml;
	MIXERLINECONTROLS mlc;

	ZeroMemory(&ml, sizeof(ml));
	ml.cbStruct = sizeof(ml);
	ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

	if (mixerGetLineInfo((HMIXEROBJ)mixer, &ml,
			MIXER_GETLINEINFOF_COMPONENTTYPE | MIXER_OBJECTF_HMIXER) != 0)
		return FALSE;

	ZeroMemory(&mlc, sizeof(mlc));
	mlc.cbStruct = sizeof(mlc);
	mlc.dwLineID = ml.dwLineID;
	mlc.dwControlType = control_type;
	mlc.cControls = ml.cControls;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mlc.pamxctrl = mc;

	if (mixerGetLineControls((HMIXEROBJ)mixer, &mlc,
			MIXER_GETLINECONTROLSF_ONEBYTYPE | MIXER_OBJECTF_HMIXER) != 0)
		return FALSE;

	return TRUE;
}


static UINT
mixer_get_volume(HMIXER mixer, DWORD control_id)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mcdu;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdu, sizeof(mcdu));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.paDetails = &mcdu;

	mixerGetControlDetails((HMIXEROBJ)mixer, &mcd,
			MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);

	return mcdu.dwValue;
}


static BOOL
mixer_get_mute(HMIXER mixer, DWORD control_id)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_BOOLEAN mcdb;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdb, sizeof(mcdb));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = &mcdb;

	mixerGetControlDetails((HMIXEROBJ)mixer, &mcd,
			MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);

	return mcdb.fValue != 0;
}


static BOOL
mixer_set_volume(HMIXER mixer, DWORD control_id, UINT step)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_UNSIGNED mcdu;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdu, sizeof(mcdu));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.paDetails = &mcdu;
	mcdu.dwValue = step;

	return mixerSetControlDetails((HMIXEROBJ)mixer, &mcd,
			MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) == 0;
}


static BOOL
mixer_set_mute(HMIXER mixer, DWORD control_id, BOOL muted)
{
	MIXERCONTROLDETAILS mcd;
	MIXERCONTROLDETAILS_BOOLEAN mcdb;

	ZeroMemory(&mcd, sizeof(mcd));
	ZeroMemory(&mcdb, sizeof(mcdb));
	mcd.cbStruct = sizeof(mcd);
	mcd.dwControlID = control_id;
	mcd.cChannels = 1;
	mcd.cMultipleItems = 0;
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.paDetails = &mcdb;
	mcdb.fValue = muted;

	return mixerSetControlDetails((HMIXEROBJ)mixer, &mcd,
			MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE) == 0;
}



/* Public Functions  {{{1 */

static int
ui_sound_play_sound(lua_State* L)
{
	const char* file;

	Crj_ParseArgs(L, "s", &file);

	if (!PlaySound(file, NULL, SND_ASYNC | SND_FILENAME))
		return luaL_error(L, "PlaySound failed");

	return 0;
}


static int
ui_sound_get_volume(lua_State* L)
{
	HMIXER mixer;
	MIXERCONTROL mc;

	if (!mixer_open(&mixer))
		return luaL_error(L, "mixer_open failed");

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_VOLUME)) {
		mixer_close(mixer);
		return luaL_error(L, "mixer_get_control failed");
	}

	lua_pushinteger(L, mixer_get_volume(mixer, mc.dwControlID));
	lua_pushinteger(L, mc.Bounds.lMinimum);
	lua_pushinteger(L, mc.Bounds.lMaximum);
	mixer_close(mixer);

	return 3;
}


static int
ui_sound_get_mute(lua_State* L)
{
	HMIXER mixer;
	MIXERCONTROL mc;

	if (!mixer_open(&mixer))
		return luaL_error(L, "mixer_open failed");

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_MUTE)) {
		mixer_close(mixer);
		return luaL_error(L, "mixer_get_control failed");
	}

	lua_pushboolean(L, mixer_get_mute(mixer, mc.dwControlID));
	mixer_close(mixer);

	return 1;
}


static int
ui_sound_set_volume(lua_State* L)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	const char* order;
	int volume = 0;
	int order_len;

	Crj_ParseArgs(L, "s", &order);
	if (!mixer_open(&mixer))
		return luaL_error(L, "mixer_open failed");

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_VOLUME)) {
		mixer_close(mixer);
		return luaL_error(L, "mixer_get_control failed");
	}

	order_len = strlen(order);
	if (order_len > 0) {
		volume = atoi(order);

		if (*(order + order_len - 1) == '%')
			volume = (int)round(volume * (mc.Bounds.lMaximum - mc.Bounds.lMinimum) / 100.0) + mc.Bounds.lMinimum;

		if (*order == '-' || *order == '+')
			volume += mixer_get_volume(mixer, mc.dwControlID);

		volume = max(mc.Bounds.lMinimum, min(volume, mc.Bounds.lMaximum));
		mixer_set_volume(mixer, mc.dwControlID, volume);
	}

	mixer_close(mixer);

	lua_pushinteger(L, volume);
	lua_pushinteger(L, mc.Bounds.lMinimum);
	lua_pushinteger(L, mc.Bounds.lMaximum);
	return 3;
}


static int
ui_sound_set_mute(lua_State* L)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	BOOL is_mute;

	Crj_ParseArgs(L, "Q", &is_mute);
	if (!mixer_open(&mixer))
		return luaL_error(L, "mixer_open failed");

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_MUTE)) {
		mixer_close(mixer);
		return luaL_error(L, "mixer_get_control failed");
	}

	mixer_set_mute(mixer, mc.dwControlID, is_mute);
	mixer_close(mixer);

	lua_pushboolean(L, is_mute);
	return 1;
}

static int
ui_sound_toggle_mute(lua_State* L)
{
	HMIXER mixer;
	MIXERCONTROL mc;
	BOOL is_mute;

	if (!mixer_open(&mixer))
		return luaL_error(L, "mixer_open failed");

	ZeroMemory(&mc, sizeof(mc));

	if (!mixer_get_cotrol(mixer, &mc, MIXERCONTROL_CONTROLTYPE_MUTE)) {
		mixer_close(mixer);
		return luaL_error(L, "mixer_get_control failed");
	}

	is_mute = mixer_get_mute(mixer, mc.dwControlID);
	mixer_set_mute(mixer, mc.dwControlID, (!is_mute));
	mixer_close(mixer);

	lua_pushboolean(L, (!is_mute));
	return 1;
}



static const luaL_Reg PUBLIC_FUNCTIONS[] = {
	{"play_sound", ui_sound_play_sound},
	{"toggle_mute", ui_sound_toggle_mute},
	{"set_mute", ui_sound_set_mute},
	{"set_volume", ui_sound_set_volume},
	{"get_mute", ui_sound_get_mute},
	{"get_volume", ui_sound_get_volume},
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
