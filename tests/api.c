/* Test: cereja C API
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"

#include <setjmp.h>




static jmp_buf s_cont;
#define TRY if (setjmp(s_cont) == 0)
#define CATCH else
#define RAISE longjmp(s_cont, !0);

#define PROTECT(flag,code)      \
        TRY {                   \
                {code;}         \
                (flag) = 1;     \
        } CATCH {               \
                (flag) = 0;     \
        }

static int
panic(lua_State* L)
{
	RAISE;
	/* never reached */
	return 0;
}

static lua_State*
new_lua_state(void)
{
	lua_State* L;

	L = luaL_newstate();
	if (L != NULL)
		lua_atpanic(L, panic);
	return L;
}

#define NEW_LUA_STATE(L) \
	lua_State* L = new_lua_state(); \
	TEST_ASSERT_PTR_NOT_NULL(L);








TEST_PROC(Crj_NUMBER_OF)
{
	short h[1];
	int i[2];
	long l[3];
	char c[4];
	char* s[5];
	float f[6];
	double d[7];
	char d2[9][8];
	char d3[12][11][10];

	TEST_ASSERT_INT(1, Crj_NUMBER_OF(h));
	TEST_ASSERT_INT(2, Crj_NUMBER_OF(i));
	TEST_ASSERT_INT(3, Crj_NUMBER_OF(l));
	TEST_ASSERT_INT(4, Crj_NUMBER_OF(c));
	TEST_ASSERT_INT(5, Crj_NUMBER_OF(s));
	TEST_ASSERT_INT(6, Crj_NUMBER_OF(f));
	TEST_ASSERT_INT(7, Crj_NUMBER_OF(d));
	TEST_ASSERT_INT(9, Crj_NUMBER_OF(d2));
	TEST_ASSERT_INT(12, Crj_NUMBER_OF(d3));
}




/* TEST_PROC(Crj_Panic)  BUGS: untested. */




/* Crj_ParseArgs/Crj_ParseTable {{{ */

TEST_PROC(Crj_ParseArgs_NullFormat)
{
	int flag;
	NEW_LUA_STATE(L);

	PROTECT(flag, {Crj_ParseArgs(L, "");});
	TEST_ASSERT(flag);
	lua_settop(L, 0);

	PROTECT(flag, {Crj_ParseArgs(L, "\f \n \r \t");});
	TEST_ASSERT(flag);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseArgs_AllFormatItemsButTable)
{
	int flag;
	lua_Number n;
	char sb;
	short sh;
	int si;
	long sl;
	unsigned char ub;
	unsigned short uh;
	unsigned int ui;
	unsigned long ul;
	const char* s1;
	const char* s2;
	size_t s2l;
	const char* z1;
	const char* z2;
	size_t z2l;
	const char* z3;
	const char* z4;
	size_t z4l;
	BOOL b;
	void* lu;
	void* fu;
	void* fu_data;
	int o;
	int ot;
	NEW_LUA_STATE(L);

	n = 0;
	sb = 0; sh = 0; si = 0; sl = 0;
	ub = 0; uh = 0; ui = 0; ul = 0;
	s1 = NULL;
	s2 = NULL;
	s2l = 0;
	z1 = NULL;
	z2 = NULL;
	z2l = 0;
	z3 = NULL;
	z4 = NULL;
	z4l = 0;
	b = FALSE;
	lu = NULL;
	fu = NULL;
	o = 0;
	ot = 0;
	lua_pushnumber(L, 31415);
	lua_pushinteger(L, 'b');
	lua_pushinteger(L, 'h');
	lua_pushinteger(L, 'i');
	lua_pushinteger(L, 'l');
	lua_pushinteger(L, 'B');
	lua_pushinteger(L, 'H');
	lua_pushinteger(L, 'I');
	lua_pushinteger(L, 'L');
	lua_pushstring(L, "92653");
	lua_pushlstring(L, "58979\0NUL", 5+1+3);
	lua_pushstring(L, "92653");
	lua_pushlstring(L, "58979\0NUL", 5+1+3);
	lua_pushnil(L);
	lua_pushboolean(L, FALSE);
	lua_pushinteger(L, 0);
	lua_pushlightuserdata(L, &lu);
	fu_data = lua_newuserdata(L, sizeof(32));
	lua_pushlightuserdata(L, &o);
	lua_pushlightuserdata(L, &ot);
	PROTECT(flag, {Crj_ParseArgs(L, "nbhilBHIL ss# zz#zz# Q u U O O/u",
	                                &n,
	                                &sb, &sh, &si, &sl,
	                                &ub, &uh, &ui, &ul,
	                                &s1, &s2, &s2l,
	                                &z1, &z2, &z2l, &z3, &z4, &z4l,
	                                &b, &lu, &fu, &o, &ot);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(31415, n);
	TEST_ASSERT_INT('b', sb);
	TEST_ASSERT_INT('h', sh);
	TEST_ASSERT_INT('i', si);
	TEST_ASSERT_INT('l', sl);
	TEST_ASSERT_INT('B', ub);
	TEST_ASSERT_INT('H', uh);
	TEST_ASSERT_INT('I', ui);
	TEST_ASSERT_INT('L', ul);
	TEST_ASSERT_STR("92653", s1);
	TEST_ASSERT_INT(5+1+3, s2l);
	TEST_ASSERT(0 == memcmp("58979\0NUL", s2, sizeof(char) * (5+1+3 + 1)));
	TEST_ASSERT_STR("92653", z1);
	TEST_ASSERT_INT(5+1+3, z2l);
	TEST_ASSERT(0 == memcmp("58979\0NUL", z2, sizeof(char) * (5+1+3 + 1)));
	TEST_ASSERT_PTR_NULL(z3);
	TEST_ASSERT_PTR_NULL(z4);
	TEST_ASSERT_INT(0, z4l);
	TEST_ASSERT_INT(TRUE, b);
	TEST_ASSERT(lu == &lu);
	TEST_ASSERT(fu == fu_data);
	TEST_ASSERT_INT(19, o);
	TEST_ASSERT_INT(20, ot);
	TEST_ASSERT_INT(LUA_TLIGHTUSERDATA, lua_type(L, ot));
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseArgs_OptionalArgs_SomeOptionalArgs)
{
	int flag;
	void* u1;
	void* u2;
	void* u3;
	void* u4;
	NEW_LUA_STATE(L);

	/* not omitted */
	u1 = u2 = u3 = u4 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	lua_pushlightuserdata(L, &u3);
	lua_pushlightuserdata(L, &u4);
	PROTECT(flag, {Crj_ParseArgs(L, "u u | u u", &u1, &u2, &u3, &u4);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	TEST_ASSERT_PTR(u3, u3);
	TEST_ASSERT_PTR(u4, u4);
	lua_settop(L, 0);

	/* 1 omitted parameter */
	u1 = u2 = u3 = u4 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	lua_pushlightuserdata(L, &u3);
	/* lua_pushlightuserdata(L, &u4); */
	PROTECT(flag, {Crj_ParseArgs(L, "u u | u u", &u1, &u2, &u3, &u4);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	TEST_ASSERT_PTR(u3, u3);
	TEST_ASSERT_PTR(NULL, u4);
	lua_settop(L, 0);

	/* 2 omitted parameters */
	u1 = u2 = u3 = u4 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	/* lua_pushlightuserdata(L, &u3); */
	/* lua_pushlightuserdata(L, &u4); */
	PROTECT(flag, {Crj_ParseArgs(L, "u u | u u", &u1, &u2, &u3, &u4);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	TEST_ASSERT_PTR(NULL, u3);
	TEST_ASSERT_PTR(NULL, u4);
	lua_settop(L, 0);

	/* too few parameters */
	u1 = u2 = u3 = u4 = NULL;
	lua_pushlightuserdata(L, &u1);
	/* lua_pushlightuserdata(L, &u2); */
	/* lua_pushlightuserdata(L, &u3); */
	/* lua_pushlightuserdata(L, &u4); */
	PROTECT(flag, {Crj_ParseArgs(L, "u u | u u", &u1, &u2, &u3, &u4);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(NULL, u2);
	TEST_ASSERT_PTR(NULL, u3);
	TEST_ASSERT_PTR(NULL, u4);
	lua_settop(L, 0);

	/* too many parameters */
	u1 = u2 = u3 = u4 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	lua_pushlightuserdata(L, &u3);
	lua_pushlightuserdata(L, &u4);
	lua_pushlightuserdata(L, NULL);
	PROTECT(flag, {Crj_ParseArgs(L, "u u | u u", &u1, &u2, &u3, &u4);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	TEST_ASSERT_PTR(u3, u3);
	TEST_ASSERT_PTR(u4, u4);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseArgs_OptionalArgs_AllOptionalArgs)
{
	int flag;
	void* u1;
	void* u2;
	NEW_LUA_STATE(L);

	/* not omitted */
	u1 = u2 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	PROTECT(flag, {Crj_ParseArgs(L, "| u u", &u1, &u2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	lua_settop(L, 0);

	/* 1 omitted parameter */
	u1 = u2 = NULL;
	lua_pushlightuserdata(L, &u1);
	/* lua_pushlightuserdata(L, &u2); */
	PROTECT(flag, {Crj_ParseArgs(L, "| u u", &u1, &u2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(NULL, u2);
	lua_settop(L, 0);

	/* no parameter */
	u1 = u2 = NULL;
	/* lua_pushlightuserdata(L, &u1); */
	/* lua_pushlightuserdata(L, &u2); */
	PROTECT(flag, {Crj_ParseArgs(L, "| u u", &u1, &u2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(NULL, u1);
	TEST_ASSERT_PTR(NULL, u2);
	lua_settop(L, 0);

	/* too many parameters */
	u1 = u2 = NULL;
	lua_pushlightuserdata(L, &u1);
	lua_pushlightuserdata(L, &u2);
	lua_pushlightuserdata(L, NULL);
	PROTECT(flag, {Crj_ParseArgs(L, "| u u", &u1, &u2);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(u2, u2);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseArgs_OptionalArgs_MiscCases)
{
	int flag;
	void* u1;
	void* u2;
	NEW_LUA_STATE(L);

	/* no arguments, no parameters */
	PROTECT(flag, {Crj_ParseArgs(L, "|");});
	TEST_ASSERT(flag);
	lua_settop(L, 0);

	/* `|' appears more than once -- extra `|'s are just ignored */
	u1 = u2 = NULL;
	lua_pushlightuserdata(L, &u1);
	/* lua_pushlightuserdata(L, &u2); */
	PROTECT(flag, {Crj_ParseArgs(L, "u|||||||||u", &u1, &u2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_PTR(u1, u1);
	TEST_ASSERT_PTR(NULL, u2);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseArgs_ErrorCases)
{
	int flag;
	lua_Number n;
	const char* s;
	size_t l;
	void* lu;
	int o;
	NEW_LUA_STATE(L);

	/* invalid format: s# must not be separated. */
	s = NULL;
	l = 3238;
	lua_pushstring(L, "3238");
	PROTECT(flag, {Crj_ParseArgs(L, "s #", &s, &l);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_STR("3238", s);
	TEST_ASSERT_INT(3238, l);
	lua_settop(L, 0);

	/* invalid format: # */
	s = NULL;
	l = 3238;
	lua_pushstring(L, "3238");
	PROTECT(flag, {Crj_ParseArgs(L, "# s #", &s, &l);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR_NULL(s);
	TEST_ASSERT_INT(3238, l);
	lua_settop(L, 0);

	/* type mismatch -- FIXME: not all patterns are tested */
	lua_pushlightuserdata(L, L);
	PROTECT(flag, {Crj_ParseArgs(L, "U", &lu);});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* type mismatch for O/x -- FIXME: not all patterns are tested */
	lua_pushlightuserdata(L, L);
	PROTECT(flag, {Crj_ParseArgs(L, "O/U", &o);});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* too few arguments */
	lua_pushnumber(L, 1);
	lua_pushnumber(L, 2);
	PROTECT(flag, {Crj_ParseArgs(L, "n n n", &n, &n, &n);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_INT(2, n);
	lua_settop(L, 0);

	/* too many arguments */
	lua_pushnumber(L, 1);
	lua_pushnumber(L, 2);
	lua_pushnumber(L, 3);
	PROTECT(flag, {Crj_ParseArgs(L, "n", &n);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_INT(1, n);
	lua_settop(L, 0);

	/* unmatched "{...}" */
	PROTECT(flag, {Crj_ParseArgs(L, "}");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* invalid type for "z" and "z#" */
	s = (const char*)&s;
	l = 35289;
	lua_pushboolean(L, TRUE);
	PROTECT(flag, {Crj_ParseArgs(L, "z#", &s, &l);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR(&s, s);
	TEST_ASSERT_INT(35289, l);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_OrderedValues)
{
	int flag;
	int i1;
	int i2;
	int i3;
	const char* s;
	NEW_LUA_STATE(L);

	i1 = 123;
	i2 = 456;
	i3 = 789;
	lua_newtable(L);
	lua_pushinteger(L, 1);
	lua_pushinteger(L, 314);
	lua_settable(L, 1);
	lua_pushinteger(L, 2);
	lua_pushinteger(L, 159);
	lua_settable(L, 1);
	lua_pushinteger(L, 3);
	lua_pushinteger(L, 265);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, 1, "{i i i}", &i1, &i2, &i3);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_INT(159, i2);
	TEST_ASSERT_INT(265, i3);
	lua_settop(L, 0);

	i1 = 123;
	i2 = 456;
	s = NULL;
	lua_newtable(L);
	lua_pushinteger(L, 1);
	lua_pushinteger(L, 314);
	lua_settable(L, -3);
	lua_pushinteger(L, 2);
	lua_pushstring(L, "159");
	lua_settable(L, -3);
	lua_pushinteger(L, 3);
	lua_pushinteger(L, 265);
	lua_settable(L, -3);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{isi}", &i1, &s, &i2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_STR("159", s);
	TEST_ASSERT_INT(265, i2);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_KeyedValues)
{
	int flag;
	int i1;
	int i2;
	int i3;
	int i4;
	NEW_LUA_STATE(L);

	i1 = 101;
	i2 = 102;
	i3 = 103;
	i4 = 104;
	lua_newtable(L);
	lua_pushstring(L, "foo");
	lua_pushinteger(L, 314);
	lua_settable(L, 1);
	lua_pushinteger(L, 321);
	lua_pushinteger(L, 159);
	lua_settable(L, 1);
	lua_pushinteger(L, 654);
	lua_pushinteger(L, 265);
	lua_settable(L, 1);
	lua_pushinteger(L, 987);
	lua_pushinteger(L, 358);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, 1, "{=si =ni =ii =li}",
	                              "foo", &i1,
	                              (lua_Number)321, &i2,
	                              654, &i3,
	                              987L, &i4);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_INT(159, i2);
	TEST_ASSERT_INT(265, i3);
	TEST_ASSERT_INT(358, i4);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_OptionalKeyedValues)
{
	int flag;
	int i1;
	int i2;
	int i3;
	int i4;
	NEW_LUA_STATE(L);

	i1 = 101;
	i2 = 102;
	i3 = 103;
	i4 = 104;
	lua_newtable(L);
	lua_pushstring(L, "foo");
	lua_pushinteger(L, 314);
	lua_settable(L, 1);
	lua_pushinteger(L, 321);
	lua_pushinteger(L, 159);
	lua_settable(L, 1);
	lua_pushinteger(L, 654);
	lua_pushinteger(L, 265);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, 1, "{=si ?ni =ii ?li}",
	                              "foo", &i1,
	                              (lua_Number)321, &i2,
	                              654, &i3,
	                              987L, &i4);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_INT(159, i2);
	TEST_ASSERT_INT(265, i3);
	TEST_ASSERT_INT(104, i4);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_MixedValues)
{
	int flag;
	const char* s1;
	const char* s2;
	const char* s3;
	int i1;
	int i2;
	NEW_LUA_STATE(L);

	s1 = NULL;
	s2 = NULL;
	s3 = NULL;
	i1 = 101;
	i2 = 202;
	lua_newtable(L);
	lua_pushinteger(L, 1);
	lua_pushstring(L, "brighter");
	lua_settable(L, 1);
	lua_pushstring(L, "bar");
	lua_pushinteger(L, 314);
	lua_settable(L, 1);
	lua_pushinteger(L, 2);
	lua_pushstring(L, "dawn");
	lua_settable(L, 1);
	lua_pushinteger(L, 3);
	lua_pushstring(L, "blue");
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, 1, "{s =si s ?li s}",
	                              &s1,
	                              "bar", &i1,
	                              &s2,
	                              654L, &i2,
	                              &s3);});
	TEST_ASSERT(flag);
	TEST_ASSERT_STR("brighter", s1);
	TEST_ASSERT_STR("dawn", s2);
	TEST_ASSERT_STR("blue", s3);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_INT(202, i2);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_Nested)
{
	int flag;
	int i1;
	int i2;
	NEW_LUA_STATE(L);

	i1 = 101;
	i2 = 202;
	lua_newtable(L);
	{
		lua_pushinteger(L, 1);
		lua_newtable(L);
		{
			lua_pushinteger(L, 1);
			lua_pushinteger(L, 314);
			lua_settable(L, -3);
		}
		lua_settable(L, 1);
	}
	PROTECT(flag, {Crj_ParseTable(L, 1, "{{i} ?i{i}}",
	                              &i1,
	                              634, &i2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(314, i1);
	TEST_ASSERT_INT(202, i2);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_ParseTable_ErrorCases)
{
	int flag;
	const char* s;
	NEW_LUA_STATE(L);

	/* not surrounded with `{' and `}' */
	lua_newtable(L);
	PROTECT(flag, {Crj_ParseTable(L, 1, "i {}");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* extra format item */
	lua_newtable(L);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{} i");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* unterminated "{...} " */
	lua_newtable(L);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* invalid <key> */
	lua_newtable(L);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{=@i}");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	/* type mismatch (ordered) */
	s = NULL;
	lua_newtable(L);
	lua_pushinteger(L, 1);
	lua_pushinteger(L, 111);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{s}", &s);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR_NULL(s);
	lua_settop(L, 0);

	/* type mismatch (keyed) */
	s = NULL;
	lua_newtable(L);
	lua_pushinteger(L, 222);
	lua_pushinteger(L, 111);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{=is}", 222, &s);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_PTR_NULL(s);
	lua_settop(L, 0);

	/* unterminated "{...}" (nested) */
	lua_newtable(L);
	lua_pushinteger(L, 1);
	lua_newtable(L);
	lua_settable(L, 1);
	PROTECT(flag, {Crj_ParseTable(L, -1, "{{}");});
	TEST_ASSERT(!flag);
	lua_settop(L, 0);

	lua_close(L);
}

/* }}} */




TEST_PROC(Crj_BuildValues_AllItemsButTable)
{
	int flag;
	int before;
	int after;
	NEW_LUA_STATE(L);

	lua_pushnumber(L, 314);
	lua_pushnumber(L, 159);
	lua_pushnumber(L, 265);
	before = lua_gettop(L);
	PROTECT(flag,
	        {Crj_BuildValues(L, "nbhilBHIL s s# Q u F O O",
	                         (lua_Number)979,
	                         'b', 'h', 'i', 'l', 'B', 'H', 'I', 'L',
	                         "323", "84626", 3,
	                         TRUE, &flag, panic, 3, -2);});
	after = lua_gettop(L);
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(3+16, lua_gettop(L));
	TEST_ASSERT_INT(16, after - before);
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 1));
	TEST_ASSERT_INT(314, lua_tonumber(L, 1));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 2));
	TEST_ASSERT_INT(159, lua_tonumber(L, 2));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 3));
	TEST_ASSERT_INT(265, lua_tonumber(L, 3));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -16));
	TEST_ASSERT_INT(979, lua_tonumber(L, -16));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -15));
	TEST_ASSERT_INT('b', lua_tonumber(L, -15));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -14));
	TEST_ASSERT_INT('h', lua_tonumber(L, -14));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -13));
	TEST_ASSERT_INT('i', lua_tonumber(L, -13));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -12));
	TEST_ASSERT_INT('l', lua_tonumber(L, -12));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -11));
	TEST_ASSERT_INT('B', lua_tonumber(L, -11));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -10));
	TEST_ASSERT_INT('H', lua_tonumber(L, -10));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -9));
	TEST_ASSERT_INT('I', lua_tonumber(L, -9));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -8));
	TEST_ASSERT_INT('L', lua_tonumber(L, -8));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, -7));
	TEST_ASSERT_STR("323", lua_tostring(L, -7));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, -6));
	TEST_ASSERT_STR("846", lua_tostring(L, -6));
	TEST_ASSERT_INT(LUA_TBOOLEAN, lua_type(L, -5));
	TEST_ASSERT_INT(TRUE, lua_toboolean(L, -5));
	TEST_ASSERT_INT(LUA_TLIGHTUSERDATA, lua_type(L, -4));
	TEST_ASSERT(&flag == lua_touserdata(L, -4));
	TEST_ASSERT(lua_iscfunction(L, -3));
	TEST_ASSERT(panic == lua_tocfunction(L, -3));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -2));
	TEST_ASSERT_INT(265, lua_tonumber(L, -2));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
	TEST_ASSERT_INT(159, lua_tonumber(L, -1));
	lua_settop(L, 0);

	lua_close(L);
}

TEST_PROC(Crj_BuildValues_TableWithoutNesting)
{
	int flag;
	int before;
	int after;
	NEW_LUA_STATE(L);

	lua_pushnumber(L, 314);
	lua_pushnumber(L, 159);
	lua_pushnumber(L, 265);
	lua_pushnumber(L, 358);
	before = lua_gettop(L);
	PROTECT(flag,
	        {Crj_BuildValues(L, "i {O =iO i =Oi O} i",
	                         979,
	                           1,
	                           323, 2,
	                           846,
	                           3, 264,
	                           4,
	                         338);});
	after = lua_gettop(L);
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(4+3, lua_gettop(L));
	TEST_ASSERT_INT(3, after - before);
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 1));
	TEST_ASSERT_INT(314, lua_tonumber(L, 1));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 2));
	TEST_ASSERT_INT(159, lua_tonumber(L, 2));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 3));
	TEST_ASSERT_INT(265, lua_tonumber(L, 3));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 4));
	TEST_ASSERT_INT(358, lua_tonumber(L, 4));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -3));
	TEST_ASSERT_INT(979, lua_tonumber(L, -3));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
	TEST_ASSERT_INT(338, lua_tonumber(L, -1));
	TEST_ASSERT_INT(LUA_TTABLE, lua_type(L, -2));
	{
		lua_pushvalue(L, -2);  /* copy the table to the top */

		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(314, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 323);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(159, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(846, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 265);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(264, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(358, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 4);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNIL, lua_type(L, -1));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);

	before = lua_gettop(L);
	PROTECT(flag, {Crj_BuildValues(L, "{}");});
	after = lua_gettop(L);
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(0+1, lua_gettop(L));
	TEST_ASSERT_INT(1, after - before);
	TEST_ASSERT(lua_istable(L, 1));
	lua_settop(L, 0);

	lua_close(L);
}

TEST_PROC(Crj_BuildValues_TableWithNesting)
{
	int flag;
	int before;
	int after;
	NEW_LUA_STATE(L);

	lua_pushnumber(L, 314);
	lua_pushnumber(L, 159);
	before = lua_gettop(L);
	PROTECT(flag,              /* 1 2a bA  c  3 */
	        {Crj_BuildValues(L, "{i {O {i} O} i}",
	                         265, 1, 358, 2, 979);});
	after = lua_gettop(L);
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(2+1, lua_gettop(L));
	TEST_ASSERT_INT(1, after - before);
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 1));
	TEST_ASSERT_INT(314, lua_tonumber(L, 1));
	TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, 2));
	TEST_ASSERT_INT(159, lua_tonumber(L, 2));
	TEST_ASSERT_INT(LUA_TTABLE, lua_type(L, -1));
	{
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(265, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TTABLE, lua_type(L, -1));
		{
			lua_pushinteger(L, 1);
			lua_gettable(L, -2);
			TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
			TEST_ASSERT_INT(314, lua_tonumber(L, -1));
			lua_pop(L, 1);

			lua_pushinteger(L, 2);
			lua_gettable(L, -2);
			TEST_ASSERT_INT(LUA_TTABLE, lua_type(L, -1));
			{
				lua_pushinteger(L, 1);
				lua_gettable(L, -2);
				TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
				TEST_ASSERT_INT(358, lua_tonumber(L, -1));
				lua_pop(L, 1);

				lua_pushinteger(L, 2);
				lua_gettable(L, -2);
				TEST_ASSERT_INT(LUA_TNIL, lua_type(L, -1));
				lua_pop(L, 1);
			}
			lua_pop(L, 1);

			lua_pushinteger(L, 3);
			lua_gettable(L, -2);
			TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
			TEST_ASSERT_INT(159, lua_tonumber(L, -1));
			lua_pop(L, 1);

			lua_pushinteger(L, 4);
			lua_gettable(L, -2);
			TEST_ASSERT_INT(LUA_TNIL, lua_type(L, -1));
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		lua_pushinteger(L, 3);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNUMBER, lua_type(L, -1));
		TEST_ASSERT_INT(979, lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushinteger(L, 4);
		lua_gettable(L, -2);
		TEST_ASSERT_INT(LUA_TNIL, lua_type(L, -1));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);

	lua_close(L);
}




TEST_PROC(Crj_SystemDirectory)
{
	char buf[MAX_PATH * Utf8_NATIVE_RATIO];
	int flag;
	NEW_LUA_STATE(L);

	/* success */
	PROTECT(flag, {Crj_SystemDirectory(L, buf, Crj_NUMBER_OF(buf));});
	TEST_ASSERT(flag);

	/* insufficient buffer: 3+5 = `X:\' + `x.exe' (without last nul) */
	PROTECT(flag, {Crj_SystemDirectory(L, buf, 3+5);});
	TEST_ASSERT(!flag);

	lua_close(L);
}


TEST_PROC(Crj_UserDirectory)
{
	char buf[MAX_PATH * Utf8_NATIVE_RATIO];
	int flag;
	NEW_LUA_STATE(L);

	/* success */
	PROTECT(flag, {Crj_UserDirectory(L, buf, Crj_NUMBER_OF(buf));});
	TEST_ASSERT(flag);

	/* insufficient buffer: 1+7 = `\' + `\cereja' (without last nul) */
	PROTECT(flag, {Crj_UserDirectory(L, buf, 1+7);});
	TEST_ASSERT(!flag);

	lua_close(L);
}




TEST_PROC(Crj_PushWindowsError)
{
	const char* s;
	int flag;
	NEW_LUA_STATE(L);

	PROTECT(flag, {Crj_PushWindowsError(L, ERROR_INVALID_FUNCTION);});
	TEST_ASSERT(flag);

	s = lua_tostring(L, -1);
	TEST_ASSERT_PTR_NOT_NULL(s);
	TEST_ASSERT_INT(0, strncmp(s, "Windows error ", 14));

	lua_close(L);
}




/* BUGS: the content of its results are not checked. */
TEST_PROC(Crj_PushTraceback)
{
	int top;
	int flag;
	NEW_LUA_STATE(L);

	top = lua_gettop(L);
	PROTECT(flag, {Crj_PushTraceback(L, 0);});
	TEST_ASSERT(flag);

	TEST_ASSERT_INT(1, lua_gettop(L) - top);
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, -1));

	lua_close(L);
}

/* BUGS: the content of its results are not checked. */
TEST_PROC(Crj_PushTracebackHandler)
{
	int top;
	int flag;
	NEW_LUA_STATE(L);

	top = lua_gettop(L);
	lua_pushcfunction(L, Crj_PushTracebackHandler);
	lua_pushstring(L, "Error message");
	PROTECT(flag, {lua_call(L, 1, 1);});
	TEST_ASSERT(flag);

	TEST_ASSERT_INT(1, lua_gettop(L) - top);
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, -1));

	lua_close(L);
}




/* Crj_PCall, Crj_Call, Crj_CPCall {{{ */
/* BUGS: The content of traceback is not checked. */

static int
pcall_success(lua_State* L)
{
	int n;
	int i;
	Crj_ParseArgs(L, "i", &n);

	for (i = 0; i < n; i++)
		lua_pushinteger(L, i);
	return n;
}

TEST_PROC(Crj_PCall_Success)
{
	int result;
	NEW_LUA_STATE(L);

	/* nresults 0 (actual 3) */
	Crj_BuildValues(L, "F i", pcall_success, 3);
	result = Crj_PCall(L, 1, 0);
	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT(0, lua_gettop(L));
	lua_settop(L, 0);

	/* nresults 1 (actual 3) */
	Crj_BuildValues(L, "F i", pcall_success, 3);
	result = Crj_PCall(L, 1, 1);
	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(0, lua_tointeger(L, 1));
	lua_settop(L, 0);

	/* nresults LUA_MULTRET (actual 3) */
	Crj_BuildValues(L, "F i", pcall_success, 3);
	result = Crj_PCall(L, 1, LUA_MULTRET);
	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT(3, lua_gettop(L));
	TEST_ASSERT_INT(0, lua_tointeger(L, 1));
	TEST_ASSERT_INT(1, lua_tointeger(L, 2));
	TEST_ASSERT_INT(2, lua_tointeger(L, 3));
	lua_settop(L, 0);

	lua_close(L);
}


static int
pcall_failure(lua_State* L)
{
	int n;
	int i;
	Crj_ParseArgs(L, "i", &n);

	for (i = 0; i < n; i++)
		lua_pushinteger(L, i);
	return luaL_error(L, "intended failure %d", n);
}

TEST_PROC(Crj_PCall_Failure)
{
	int result;
	NEW_LUA_STATE(L);

	/* nresults 0 (actual 3) */
	Crj_BuildValues(L, "F i", pcall_failure, 3);
	result = Crj_PCall(L, 1, 0);
	TEST_ASSERT_INT(LUA_ERRRUN, result);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, 1));
	TEST_ASSERT(strstr(lua_tostring(L, 1), "intended failure 3") != NULL);
	lua_settop(L, 0);

	/* nresults 1 (actual 5) */
	Crj_BuildValues(L, "F i", pcall_failure, 5);
	result = Crj_PCall(L, 1, 1);
	TEST_ASSERT_INT(LUA_ERRRUN, result);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, 1));
	TEST_ASSERT(strstr(lua_tostring(L, 1), "intended failure 5") != NULL);
	lua_settop(L, 0);

	/* nresults LUA_MULTRET (actual 7) */
	Crj_BuildValues(L, "F i", pcall_failure, 7);
	result = Crj_PCall(L, 1, LUA_MULTRET);
	TEST_ASSERT_INT(LUA_ERRRUN, result);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, 1));
	TEST_ASSERT(strstr(lua_tostring(L, 1), "intended failure 7") != NULL);
	lua_settop(L, 0);

	lua_close(L);
}


TEST_PROC(Crj_Call)
{
	int flag;
	NEW_LUA_STATE(L);

	/* nresults 2 (actual 3) : success */
	Crj_BuildValues(L, "F i", pcall_success, 3);
	PROTECT(flag, {Crj_Call(L, 1, 2);});
	TEST_ASSERT(flag);
	TEST_ASSERT_INT(2, lua_gettop(L));
	TEST_ASSERT_INT(0, lua_tointeger(L, 1));
	TEST_ASSERT_INT(1, lua_tointeger(L, 2));
	lua_settop(L, 0);

	/* nresults 2 (actual 4) : failure */
	Crj_BuildValues(L, "F i", pcall_failure, 4);
	PROTECT(flag, {Crj_Call(L, 1, 2);});
	TEST_ASSERT(!flag);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, 1));
	TEST_ASSERT(strstr(lua_tostring(L, 1), "intended failure 4") != NULL);
	lua_settop(L, 0);

	lua_close(L);
}


static int
cpcall_func(lua_State* L)
{
	void* u;
	Crj_ParseArgs(L, "u", &u);

	if (u == NULL)
		luaL_error(L, "NULL!");
	lua_pushlightuserdata(L, u);
	return 1;
}

TEST_PROC(Crj_CPCall)
{
	int result;
	NEW_LUA_STATE(L);

	result = Crj_CPCall(L, cpcall_func, &result);
	TEST_ASSERT_INT(0, result);
	TEST_ASSERT_INT(0, lua_gettop(L));
	lua_settop(L, 0);

	result = Crj_CPCall(L, cpcall_func, NULL);
	TEST_ASSERT_INT(LUA_ERRRUN, result);
	TEST_ASSERT_INT(1, lua_gettop(L));
	TEST_ASSERT_INT(LUA_TSTRING, lua_type(L, 1));
	TEST_ASSERT(strstr(lua_tostring(L, 1), "NULL!") != NULL);
	lua_settop(L, 0);

	lua_close(L);
}

/* }}} */




/* __END__ */
/* vim: foldmethod=marker
 */
