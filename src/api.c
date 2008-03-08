/* cereja C API
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"
#include <assert.h>
#include <ctype.h>








/* for error messages in Crj_ParseArgs/Crj_BuildValues */
#define SANITIZE_CHAR(c) (isprint((c)) ? (c) : '#')

#define STRSKIP(s,skipchars) ((s) + strspn((s), (skipchars)))
#define WHITESPACES "\f\n \r\t"
#define SUCCESS TRUE
#define FAILURE FALSE

	/* BUGS: This value is for Lua 5.1.
	 * This may be different for each Lua version.  */
#define FIRST_PSEUDO_INDEX LUA_REGISTRYINDEX
#define ABSOLUTE_INDEX(L, index)                                \
        (((FIRST_PSEUDO_INDEX < (index)) && ((index) < 0))      \
         ? (lua_gettop(L) + (index) + 1)                        \
         : (index))








/* Crj_ParseArgs -- {{{ */

	/* forward */
static int parse_table(lua_State* L, const char** p_format, va_list* p_va,
                       int table_index);


static BOOL
check_type(lua_State* L, int value_index, int expected_type)
{
	if (lua_type(L, value_index) != expected_type) {
		lua_pushfstring(L, "type mismatch (expected %s, but got %s)",
		                lua_typename(L, expected_type),
		                luaL_typename(L, value_index));
		return FAILURE;
	}

	return SUCCESS;
}
#define CHECK_TYPE(L, v, e)                                     \
        {                                                       \
                if (check_type((L), (v), (e)) != SUCCESS)       \
                        return FAILURE;                         \
        }
#define CONVERT_NUMBER(L, va, value_index, TYPE)                \
        {                                                       \
                TYPE* number;                                   \
                                                                \
                CHECK_TYPE((L), (value_index), LUA_TNUMBER);    \
                number = va_arg((va), TYPE*);                   \
                *number = lua_tonumber((L), (value_index));     \
        }


static int
parse_item(lua_State* L, const char** p_format, va_list* p_va,
           int value_index, BOOL in_tablep, BOOL* p_optionalp)
{
	const char* format = *p_format;

L_RETRY:
	format = STRSKIP(format, WHITESPACES);
	switch (*format) {
	default:
		lua_pushfstring(L, "format item `%c(%d)' is not valid",
		                SANITIZE_CHAR(*format), *format);
		return FAILURE;
	case 'n': CONVERT_NUMBER(L, *p_va, value_index, lua_Number); break;
	case 'b': CONVERT_NUMBER(L, *p_va, value_index, char); break;
	case 'h': CONVERT_NUMBER(L, *p_va, value_index, short); break;
	case 'i': CONVERT_NUMBER(L, *p_va, value_index, int); break;
	case 'l': CONVERT_NUMBER(L, *p_va, value_index, long); break;
	case 'B': CONVERT_NUMBER(L, *p_va, value_index, unsigned char); break;
	case 'H': CONVERT_NUMBER(L, *p_va, value_index, unsigned short); break;
	case 'I': CONVERT_NUMBER(L, *p_va, value_index, unsigned int); break;
	case 'L': CONVERT_NUMBER(L, *p_va, value_index, unsigned long); break;
	case 's':
		CHECK_TYPE(L, value_index, LUA_TSTRING);
		/* FALLTHRU */
	case 'z': {
		const char** string;
		size_t* length = NULL;

		string = va_arg(*p_va, const char**);
		if (*(format+1) == '#') {
			length = va_arg(*p_va, size_t*);
			format++;
		}

		if (lua_isstring(L, value_index)) {
			*string = lua_tolstring(L, value_index, length);
		} else if (!lua_toboolean(L, value_index)) {  /*nil-or-false?*/
			*string = NULL;
			if (length != NULL)
				*length = 0;
		} else {
			lua_pushfstring(L,
			  "type mismatch (expected %s/nil/false, but got %s)",
			  lua_typename(L, LUA_TSTRING),
			  luaL_typename(L, value_index));
			return FAILURE;
		}
		} break;
	case 'Q': {
		BOOL* p_boolean;

		p_boolean = va_arg(*p_va, BOOL*);
		*p_boolean = lua_toboolean(L, value_index);
		} break;
	case 'u': /* FALLTHRU */
	case 'U': {
		void** userdata;

		CHECK_TYPE(L, value_index,
		           ((*format == 'U') ? LUA_TUSERDATA
		                             : LUA_TLIGHTUSERDATA));
		userdata = va_arg(*p_va, void**);
		*userdata = lua_touserdata(L, value_index);
		} break;
	case 'O': {
		int* index;

		if (in_tablep) {
			lua_pushstring(L,
			  "format item `O' is not available in `{...}");
			return FAILURE;
		}

		if (*(format+1) == '/') {
			int type;

			switch (*(format+2)) {
			default:
				lua_pushfstring(L,
				  "type `%c(%d)' for `O/<type>' is not valid",
				  SANITIZE_CHAR(*(format+2)), *(format+2));
				return FAILURE;
			case 'N': type = LUA_TNIL; break;
			case 'n': type = LUA_TNUMBER; break;
			case 's': type = LUA_TSTRING; break;
			case 'f': type = LUA_TFUNCTION; break;
			case 'Q': type = LUA_TBOOLEAN; break;
			case 'u': type = LUA_TLIGHTUSERDATA; break;
			case 'U': type = LUA_TUSERDATA; break;
			case 't': type = LUA_TTABLE; break;
			case 'T': type = LUA_TTHREAD; break;
			}
			CHECK_TYPE(L, value_index, type);
			format += 2;
		}

		index = va_arg(*p_va, int*);
		*index = value_index;
		} break;
	case '{':
		CHECK_TYPE(L, value_index, LUA_TTABLE);
		format++;
		if (parse_table(L, &format, p_va, value_index) != SUCCESS)
			return FAILURE;
		break;
	case '|':
		if (in_tablep) {
			lua_pushstring(L,
			  "optional argument `|' is not available in `{...}");
			return FAILURE;
		}
		*p_optionalp = TRUE;
		format++;
		goto L_RETRY;
	}

	*p_format = format + 1;
	return SUCCESS;
}


/* BUGS: This must be updated when parse_item/parse_table_item is changed. */
static void
skip_item(const char** p_format, va_list* p_va)
{
	const char* f = *p_format;

	switch (*f) {
	case 'O':  /* "O" is not available in "{...}", so treat as invalid. */
	case '|':  /* same as "O" */
		/* FALLTHRU */
	default:
		/* nop; invalid char will be checked as the next item. */
		return;
	case 'n': (void)va_arg(*p_va, lua_Number*); break;
	case 'b': (void)va_arg(*p_va, char*); break;
	case 'h': (void)va_arg(*p_va, short*); break;
	case 'i': (void)va_arg(*p_va, int*); break;
	case 'l': (void)va_arg(*p_va, long*); break;
	case 'B': (void)va_arg(*p_va, unsigned char*); break;
	case 'H': (void)va_arg(*p_va, unsigned short*); break;
	case 'I': (void)va_arg(*p_va, unsigned int*); break;
	case 'L': (void)va_arg(*p_va, unsigned long*); break;
	case 's': /* FALLTHRU */
	case 'z':
		  (void)va_arg(*p_va, const char**);
		  if (*(f+1) == '#') {
			  (void)va_arg(*p_va, size_t*);
			  f++;
		  }
		  break;
	case 'Q': (void)va_arg(*p_va, BOOL*); break;
	case 'u': /* FALLTHRU */
	case 'U': (void)va_arg(*p_va, void**); break;
	case '{': {
		int level;

		level = 1;
		while (0 < level) {
			f++;
			switch (*f) {
			default:
				break;
			case '{':
				level++;
				break;
			case '}':
				level--;
				break;
			case '\0':
				return;
			}
		}
		} break;
	}

	*p_format = f + 1;
}

static int
parse_table_item(lua_State* L, const char** p_format, va_list* p_va,
                 int table_index, int* p_order)
{
	const char* format = *p_format;

	format = STRSKIP(format, WHITESPACES);
	switch (*format) {
	default:  /* "<value>" */
		lua_pushinteger(L, *p_order);
		lua_gettable(L, table_index);
		if (parse_item(L, &format, p_va, -1, TRUE, NULL) != SUCCESS) {
			lua_pushfstring(L, " (for key %d, near `%s')",
			                *p_order, format);
			lua_concat(L, 2);
			return FAILURE;
		}
		lua_pop(L, 1);
		(*p_order)++;
		break;
	case '\0':
		lua_pushstring(L, "format item `{...}' is not terminated");
		return FAILURE;
	case '}':
		break;
	case '?': /* FALLTHRU */
	case '=': {  /* "=<key><value>" or "?<key><value>" */
		int key_index;
		int value_requiredp = (*format == '=');
		format++;
		switch (*format) {
		default:
			lua_pushfstring(L, "key `%c(%d)' is not valid",
			                SANITIZE_CHAR(*format),
			                *format);
			return FAILURE;
		case 's': lua_pushstring(L, va_arg(*p_va, const char*)); break;
		case 'n': lua_pushnumber(L, va_arg(*p_va, lua_Number)); break;
		case 'i': lua_pushnumber(L, va_arg(*p_va, int)); break;
		case 'l': lua_pushnumber(L, va_arg(*p_va, long)); break;
		}
		key_index = lua_gettop(L);

		lua_pushvalue(L, -1);
		lua_gettable(L, table_index);
		format++;
		if (!lua_isnil(L, -1)) {  /* found value for key? */
			if (parse_item(L, &format, p_va, -1, TRUE, NULL)
			    != SUCCESS)
			{
				lua_pushfstring(L, " (for key `%s')",
						lua_tostring(L, key_index));
				lua_concat(L, 2);
				return FAILURE;
			}
		} else {
			if (value_requiredp) {  /* '=' */
				lua_pushfstring(L,
				  "value for key `%s' is not found",
				  lua_tostring(L, key_index));
				return FAILURE;
			} else {  /* '?' */
				skip_item(&format, p_va);
			}
		}
		lua_pop(L, 2);
		} break;
	}

	*p_format = format;
	return SUCCESS;
}

static int
parse_table(lua_State* L, const char** p_format, va_list*p_va, int table_index)
{
	int order;
	int result;
		/* `table_index' must be an absolute index (positive integer).
		 * Because value pointed by a relative index is not consistent:
		 * it will be changed as values are pushed on the stack.  */
	table_index = ABSOLUTE_INDEX(L, table_index);

	order = 1;
	for (;;) {
		result = parse_table_item(L, p_format, p_va,
		                          table_index, &order);
		if (result != SUCCESS)
			return FAILURE;
		if (**p_format == '}')
			break;
	}
	return SUCCESS;
}


Crj_PUBLIC(void)
Crj_ParseArgs(lua_State* L, const char* original_format, ...)
{
	va_list va;
	const char* format;
	int args_count;
	int arg_index;
	BOOL optionalp = FALSE;
	int result = SUCCESS;

	va_start(va, original_format);
	format = original_format;
	args_count = lua_gettop(L);
	for (arg_index = 1; arg_index <= args_count; arg_index++) {
		result = parse_item(L, &format, &va, arg_index,
		                    FALSE, &optionalp);
		if (result != SUCCESS)
			break;
	}
	va_end(va);

	if (result != SUCCESS) {
		if (*format != '\0') {
			/* error message pushed by parse_item */
			lua_pushfstring(L, "bad argument #%d:", arg_index);
			lua_insert(L, -2);
			lua_pushfstring(L," (in format `%s')",original_format);

			/* bad argument #N:<error message> (in format `...') */
			lua_concat(L, 3);
			lua_error(L);
		} else {
			luaL_error(L, "extra %d argument(s) (in format `%s')",
			           args_count - arg_index + 1,
			           original_format);
		}
	}

	format = STRSKIP(format, WHITESPACES);
	if ((*format != '\0')                   /* there are remaining items */
	    && !(optionalp || (*format == '|'))) /* & no optional arguments. */
	{
		luaL_error(L, "lacked argument(s) for `%s' (in format `%s')",
		           format, original_format);
	}
}


Crj_PUBLIC(void)
Crj_ParseTable(lua_State* L, int table_index, const char* original_format, ...)
{
	const char* format = original_format;
	va_list va;
	int result;

	va_start(va, original_format);
	result = parse_item(L, &format, &va, table_index, FALSE, NULL);
	va_end(va);

	if (result != SUCCESS) {
		/* error message pushed by parse_item */
		lua_pushfstring(L, " (in format `%s')", original_format);

		/* <error message> (in format `...') */
		lua_concat(L, 2);
		lua_error(L);
	}

	format = STRSKIP(format, WHITESPACES);
	if (*format != '\0') {
		luaL_error(L, "extra format items (in format `%s')",
		           original_format);
	}
}

/* }}} */




/* Crj_BuildValues -- {{{ */

enum {
	B_SUCCESS,
	B_END_OF_FORMAT,
	B_ERROR,
	B_TABLE_FIELD_PREFIX,
	B_END_OF_TABLE
};

static int
build_a_value(lua_State* L, const char** _format, va_list* va,
              int in_tablep, int values_count)
{
	const char* format = *_format;

		/* skip free spaces for readability */
	format += strspn(format, " \f\n\r\t");

	switch (*format) {
	default:
		lua_pushfstring(L, "bad format item `%c(%d)'",
		                SANITIZE_CHAR(*format), *format);
		return B_ERROR;
	case '\0':
		return B_END_OF_FORMAT;
	case 'n': lua_pushnumber(L, va_arg(*va, lua_Number)); break;
	case 'b': case 'B': case 'h': case 'H':
		  /* FALLTHRU: char, unsigned char, short and unsigned short
		   * are promoted to int when passed through `...'.
		   */
	case 'i': lua_pushnumber(L, va_arg(*va, int)); break;
	case 'I': lua_pushnumber(L, va_arg(*va, unsigned int)); break;
	case 'l': lua_pushnumber(L, va_arg(*va, long)); break;
	case 'L': lua_pushnumber(L, va_arg(*va, unsigned long)); break;
	case 's': {
		const char* string;
		size_t length;

		string = va_arg(*va, const char*);
		if (*(format+1) == '#') {
			length = va_arg(*va, size_t);
			format += 1;
		} else {
			length = strlen(string);
		}
		lua_pushlstring(L, string, length);
		} break;
	case 'Q':
		lua_pushboolean(L, va_arg(*va, BOOL));
		break;
	case 'u':
		lua_pushlightuserdata(L, va_arg(*va, void*));
		break;
	case 'F':
		lua_pushcfunction(L, va_arg(*va, lua_CFunction));
		break;
	case 'O': {
		int index;

		index = va_arg(*va, int);
		if ((FIRST_PSEUDO_INDEX < index) && (index < 0))
			index -= values_count;
		lua_pushvalue(L, index);
		} break;
	case '=':
		if (!in_tablep) {
			lua_pushfstring(L, "bad format item `%c(%d)'",
			                SANITIZE_CHAR(*format), *format);
			return B_ERROR;
		}
		*_format = format + 1;
		return B_TABLE_FIELD_PREFIX;
	case '}':
		if (!in_tablep) {
			lua_pushfstring(L, "bad format item `%c(%d)'",
			                SANITIZE_CHAR(*format), *format);
			return B_ERROR;
		}
		*_format = format + 1;
		return B_END_OF_TABLE;
	case '{': {
		int order;
		int result;

		lua_newtable(L);
		order = 1;

		format++;
		for (;;) {
		  result = build_a_value(L, &format, va, TRUE, values_count);
		  switch (result) {
		  default:
		    assert(FALSE);
		  case B_SUCCESS:  /* <value> */
		    lua_pushinteger(L, order);
		    lua_insert(L, -2);  /* swap -1 and -2. */
		    lua_settable(L, -3);
		    order++;
		    break;
		  case B_TABLE_FIELD_PREFIX:  /* =<key><value> */
		    if ((build_a_value(L, &format, va, TRUE, values_count)
		         == B_SUCCESS)
		        && (build_a_value(L, &format, va,TRUE, values_count+1)
		            == B_SUCCESS)) {
		      lua_settable(L, -3);
		    } else {
		      lua_pushstring(L,
		        "bad format: unmatched `=<key><value>' in `{...}'"
		      );
		      return B_ERROR;
		    }
		    break;
		  case B_END_OF_TABLE:
		    goto L_TABLE;
		  case B_END_OF_FORMAT:
		    lua_pushstring(L, "bad format: `{...}' is not terminated");
		    return B_ERROR;
		  }
		}
	L_TABLE:;
		}
		if (*format == '\0') {
			*_format = format;
			return B_END_OF_FORMAT;
		}
		break;
	}

	*_format = format + 1;
	return B_SUCCESS;
}

Crj_PUBLIC(void)
Crj_BuildValues(lua_State* L, const char* _format, ...)
{
	va_list va;
	int result;
	const char* format = _format;
	int values_count;

	va_start(va, _format);
	values_count = 0;
	for (;;) {
		result = build_a_value(L, &format, &va, FALSE, values_count);
		if (result != B_SUCCESS)
			break;
		values_count++;
	}
	va_end(va);

	if (result != B_END_OF_FORMAT) {
		/* error message pushed by build_a_value */
		lua_error(L);
	}
}  /* }}} */




Crj_PUBLIC(void)
Crj_SystemDirectory(lua_State* L, char* buf, int size)
{
	int length;
	char* last_dir_sep;

	/* GetModuleFileName returns the string length without the last NUL. */
	length = GetModuleFileName(NULL, buf, size);
	if (length == 0)
		luaL_error(L, "system-directory cannot be obtained");
	if (size <= length+1) {
		buf[size - 1] = '\0';
		luaL_error(L, "insufficient buffer (given %d, copied `%s')",
		           size, buf);
	}

	last_dir_sep = strrchr(buf, '\\');
	if (last_dir_sep == NULL)
		luaL_error(L, "system-directory is somewhat strange (`%s')",
		           buf);
	*last_dir_sep = '\0';
	return;
}

Crj_PUBLIC(void)
Crj_UserDirectory(lua_State* L, char* buf, int size)
{
	const char* base_dir;
	const char* postfix;
	char base_dir_buf[MAX_PATH * Utf8_NATIVE_RATIO];

	if ((base_dir = Utf8_GetEnvA("CEREJA_USERDIR")) != NULL) {
		postfix = "";
	} else if ((base_dir = Utf8_GetEnvA("HOME")) != NULL) {
		postfix = "\\.cereja";
	} else {
		HRESULT hr;
		hr = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,
		                     SHGFP_TYPE_CURRENT, base_dir_buf);
		if (!SUCCEEDED(hr))
			luaL_error(L, "user-directory cannot be determined");
		postfix = "\\cereja";
		base_dir = base_dir_buf;
	}

	if (!(((int)(strlen(base_dir) + strlen(postfix) + 1)) <= size)) {
		luaL_error(L, "insufficient buffer (required %d, given %d)",
		           strlen(base_dir) + strlen(postfix) + 1,
		           size);
	}
	strcpy(buf, base_dir);
	strcat(buf, postfix);
}




static BOOL
default_notice_handler(int category,
                       const char* noticer,
                       const char* message,
                       BOOL noticedp)
{
	if (!noticedp) {
		char title[80*2];
		UINT icon;

		_snprintf(title, Crj_NUMBER_OF(title), "%s - %s",
		          noticer, Crj_APPNAME);
		title[Crj_NUMBER_OF(title) - 1] = '\0';
		switch (category) {
		default:  /* FALLTHRU */
		case Crj_NOTICE_DEBUG: icon = 0; break;
		case Crj_NOTICE_INFO: icon = MB_ICONINFORMATION; break;
		case Crj_NOTICE_WARNING: icon = MB_ICONWARNING; break;
		case Crj_NOTICE_ERROR: icon = MB_ICONERROR; break;
		}

		MessageBox(NULL, message, title,
		           MB_OK | MB_TOPMOST | MB_APPLMODAL | icon);
	}
	return TRUE;
}


Crj_PUBLIC(void)
Crj_Notice(lua_State* L, int category, const char* noticer, const char*message)
{
	BOOL noticedp = FALSE;

	if (L != NULL) {
		lua_getglobal(L, "cereja");
		lua_getfield(L, -1, "call_notice_handlers");

		if (lua_isfunction(L, -1)) {
			lua_pushinteger(L, category);
			lua_pushstring(L, noticer);
			lua_pushstring(L, message);
			lua_call(L, 3, 1);
			noticedp = lua_toboolean(L, -1);
		}

		lua_pop(L, 2);
	}
	default_notice_handler(category, noticer, message, noticedp);
}

Crj_PUBLIC(void)
Crj_NoticeF(lua_State* L, int category, const char* noticer,
            const char* format, ...)
{
	va_list va;
	char message[80*25];  /* FIXME: fixed buffer for message */

	va_start(va, format);
	_vsnprintf(message, Crj_NUMBER_OF(message), format, va);
	message[Crj_NUMBER_OF(message) - 1] = '\0';
	va_end(va);

	Crj_Notice(L, category, noticer, message);
}




Crj_PUBLIC(void)
Crj_PushWindowsError(lua_State* L, DWORD error_code)
{
	char* buf;
	DWORD length;

	if (error_code == 0)
		error_code = GetLastError();

	length = FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER
	                        | FORMAT_MESSAGE_FROM_SYSTEM),
	                       NULL,
	                       error_code,
	                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			       (LPSTR)&buf,
	                       0,
	                       NULL);
	if (length == 0) {
		lua_pushfstring(L, "Windows error %d", error_code);
	} else {
		/* chomp trailing CR/LF */
		while ((0 < length) && iscntrl(buf[length-1]))
			buf[--length] = '\0';

		lua_pushfstring(L, "Windows error %d `%s'", error_code, buf);

		LocalFree(buf);
	}
	return;
}




Crj_PUBLIC(void)
Crj_PushTraceback(lua_State* L, int start_level)
{
	int level;
	int top;
	lua_Debug ar;

	top = lua_gettop(L);
	lua_pushstring(L, "Traceback:");
	for (level = start_level; lua_getstack(L, level, &ar) == 1; level++) {
		lua_getinfo(L, "Snl", &ar);
		lua_pushstring(L, "\n\t");
		lua_pushfstring(L, "%s:", ar.short_src);
		if (0 < ar.currentline)
			lua_pushfstring(L, "%d:", ar.currentline);
		lua_pushstring(L, " ");
		if (ar.namewhat[0] != '\0') {
			lua_pushfstring(L, "in function `%s'", ar.name);
		} else if (ar.what[0] == 'm') {
			lua_pushstring(L, "in main chunk");
		} else if (ar.what[0] == 'C') {
			lua_pushstring(L, "?");
		} else if (ar.what[0] == 't') {
			lua_pushstring(L, "?");
		} else {
			lua_pushfstring(L, "in function <%s:%d>",
			                ar.short_src, ar.linedefined);
		}
	}
	lua_concat(L, lua_gettop(L) - top);
	return;
}

Crj_PUBLIC(int)
Crj_PushTracebackHandler(lua_State* L)
{
	/* error message pushed by lua_pcall */
	lua_pushstring(L, "\n\n");
	Crj_PushTraceback(L, 1);
	lua_concat(L, 3);

	return 1;
}


Crj_PUBLIC(void)
Crj_Call(lua_State* L, int nargs, int nresults)
{
	if (Crj_PCall(L, nargs, nresults) != 0)
		lua_error(L);
}

Crj_PUBLIC(int)
Crj_PCall(lua_State* L, int nargs, int nresults)
{
	/*          ,-------.       ,-------.
	 *          |   :   |       |   :   |
	 * -(1+N+1) | func  |       |handler|
	 * -(1+N  ) | arg 1 |       | func  |
	 *     :    |   :   |  ==>  | arg 1 |
	 * -(1+1  ) | arg N |       |   :   |
	 * -(1+0  ) |handler|       | arg N |
	 *          `-------'       `-------'
	 */
	int result;
	int handler_index;

	lua_pushcfunction(L, Crj_PushTracebackHandler);
	lua_insert(L, -(1+nargs+1));
	handler_index = ABSOLUTE_INDEX(L, -(1+nargs+1));

	result = lua_pcall(L, nargs, nresults, handler_index);
	lua_remove(L, handler_index);
	return result;
}

Crj_PUBLIC(int)
Crj_CPCall(lua_State* L, lua_CFunction func, void* ud)
{
	lua_pushcfunction(L, func);
	lua_pushlightuserdata(L, ud);
	return Crj_PCall(L, 1, 0);
}




Crj_PUBLIC(void)
Crj_Panic(const char* message)
{
	MessageBox(NULL, message, Crj_APPNAME,
	           MB_OK | MB_TOPMOST | MB_APPLMODAL | MB_ICONERROR);

	exit(EXIT_FAILURE);
}




Crj_PUBLIC(void*)
Crj_Malloc(size_t size)
{
	void* p;

	p = malloc(size);
	if (p == NULL)
		Crj_Panic("Out of memory");

	return p;
}


Crj_PUBLIC(void*)
Crj_Realloc(void* old, size_t size)
{
	void* p;

	p = realloc(old, size);
	if (p == NULL)
		Crj_Panic("Out of memory");

	return p;
}




/* __END__ */
/* vim: foldmethod=marker
 */
