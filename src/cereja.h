/* The main header file for cereja.
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#ifndef _CRJ_CEREJA_H
#define _CRJ_CEREJA_H

#define WINVER 0x501
#define _WIN32_WINNT 0x501
#define _WIN32_IE 0x0600
#define UNICODE
#define _UNICODE
#define STRICT
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>

#include "utf8api.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"








/* cereja C API
 * ============
 */

#define Crj_APPNAME "cereja"

#define Crj_MAIN_WINDOW_CLASS "cereja"
#define Crj_MAIN_WINDOW_TITLE "cereja"

	/* WPARAM; LPARAM */
#define Crj_WM_EXIT (WM_USER + 1)  /* unused; unused */
#define Crj_WM_EXEC (WM_USER + 2)  /* unused; script (const char*) */




#define Crj_PUBLIC(type) __declspec(dllexport) type

	/* Cast expr to type, to avoid -Wcast-qual in obvious cases. */
	/* BUGS: assumes that sizeof(type) <= sizeof(unsigned long). */
#define Crj_FORCE_CAST(type, expr) ((type)(unsigned long)(expr))

#define Crj_NUMBER_OF(array) (sizeof((array)) / sizeof((array)[0]))




/* for internal use; don't call this. */
Crj_PUBLIC(void) _Crj_OpenBuiltins(lua_State* L);
Crj_PUBLIC(void) _Crj_CloseBuiltins(lua_State* L);




Crj_PUBLIC(void*) Crj_Malloc(size_t size);
Crj_PUBLIC(void*) Crj_Realloc(void* old, size_t size);
#define Crj_MALLOC(size) Crj_Malloc(size)
#define Crj_REALLOC(old,size) Crj_Realloc(old,size)
#define Crj_FREE(p) free(p)




/* Notice to user
 * --------------
 */

Crj_PUBLIC(void) Crj_Panic(const char* message);
/* for critical situations. */


Crj_PUBLIC(void) Crj_Notice(lua_State* L,
                            int category,
                            const char* noticer,
                            const char* message);
/* Notice a message to user.
 *
 * Arguments:
 *
 *   L
 *     lua_State.
 *     This may be NULL while the system of cereja is not fully ready.
 *
 *   category
 *     Category of the message.
 *
 *   noticer
 *     Name of the noticer (e.g. module name).
 *
 *   message
 *     Message to be noticed.
 */

Crj_PUBLIC(void) Crj_NoticeF(lua_State* L,
                             int category,
                             const char* noticer,
                             const char* format,
                             ...);
/* printf-like version of Crj_Notice */

/* Category values for Crj_Notice. */
enum {
	Crj_NOTICE_DEBUG,
	Crj_NOTICE_INFO,
	Crj_NOTICE_WARNING,
	Crj_NOTICE_ERROR,
};


/* Shorthands for Crj_Notice with a specific category. */
#define Crj_NoticeDebug(L, s, m)  Crj_Notice(L, Crj_NOTICE_DEBUG, s, m)
#define Crj_NoticeInfo(L, s, m)  Crj_Notice(L, Crj_NOTICE_INFO, s, m)
#define Crj_NoticeWarning(L, s, m)  Crj_Notice(L, Crj_NOTICE_WARNING, s, m)
#define Crj_NoticeError(L, s, m)  Crj_Notice(L, Crj_NOTICE_ERROR, s, m)

/* Shorthands for Crj_NoticeF with a specific category. */
#define Crj_NoticeDebugF(L, s, f, ...) \
        Crj_NoticeF(L, Crj_NOTICE_DEBUG, s, f, __VA_ARGS__)
#define Crj_NoticeInfoF(L, s, f, ...) \
        Crj_NoticeF(L, Crj_NOTICE_INFO, s, f, __VA_ARGS__)
#define Crj_NoticeWarningF(L, s, f, ...) \
        Crj_NoticeF(L, Crj_NOTICE_WARNING, s, f, __VA_ARGS__)
#define Crj_NoticeErrorF(L, s, f, ...) \
        Crj_NoticeF(L, Crj_NOTICE_ERROR, s, f, __VA_ARGS__)




/* Error handling & traceback
 * --------------------------
 */

Crj_PUBLIC(void) Crj_Call(lua_State* L, int nargs, int nresults);
Crj_PUBLIC(int) Crj_PCall(lua_State* L, int nargs, int nresults);
Crj_PUBLIC(int) Crj_CPCall(lua_State* L, lua_CFunction func, void* ud);
/* Like lua_call, lua_pcall and lua_cpcall, but with Crj_PushTraceback. */


Crj_PUBLIC(void) Crj_PushWindowsError(lua_State* L, DWORD error_code);
/* Push a string representing the meaning of error_code. */


Crj_PUBLIC(void) Crj_PushTraceback(lua_State* L, int level);
/* Push a string representing a traceback of the call stack.
 * `level' specifies at which level to start the traceback.
 * Level 0 is the current running function,
 * whereas level n+1 is the function that has called level n.
 */
Crj_PUBLIC(int) Crj_PushTracebackHandler(lua_State* L);
/* Like Crj_PushTraceback, for an error handler of lua_pcall. */




/* Misc.
 * -----
 */

Crj_PUBLIC(void) Crj_SystemDirectory(lua_State* L, char* buf, int size);
Crj_PUBLIC(void) Crj_UserDirectory(lua_State* L, char* buf, int size);
/* Copy the system-directory or the user-directory to buf. */


Crj_PUBLIC(void) Crj_ParseArgs(lua_State* L, const char* format, ...);  /*{{{*/
/* Parse the parameters of a lua_CFunction,
 * and assign them to the given C variables.
 * The content of the stack must be only the parameters,
 * so the index N refers the Nth parameter.
 *
 * Return values:
 *   [success]
 *     nothing
 *
 *   [failure]
 *     nothing; throw a Lua error.
 *
 * Format:
 *
 *   A format string consists of zero or more format items.
 *   The Nth format item corresponds the Nth parameter.
 *   Spaces between format items are ignored for readability.
 *
 *   To describe each format item, use the following template:
 *     "format item" (Lua type to be matched) [C type(s) to be assigned]
 *
 *   "n" (number) [lua_Number]
 *     Convert a Lua number to a lua_Number.
 *
 *   "b" (number) [char]
 *   "h" (number) [short]
 *   "i" (number) [int]
 *   "l" (number) [long]
 *   "B" (number) [unsigned char]
 *   "H" (number) [unsigned short]
 *   "I" (number) [unsigned int]
 *   "L" (number) [unsigned long]
 *     Like "n", but convert to an integer of the specified type.
 *
 *   "s" (string) [const char*]
 *     Convert a Lua string to a C pointer to a string.
 *
 *   "s#" (string) [const char*, size_t]
 *     Like "s", but also assign the string length.
 *
 *   "z" (string or nil or false) [const char*]
 *   "z#" (string or nil or false) [const char*, size_t]
 *     Like "s" and "s#", but the corresponding argument may be nil or false,
 *     in which case the given C variables are assigned to NULL and 0.
 *
 *   "Q" (boolean) [BOOL = int]
 *     Convert a Lua boolean to a C boolean.
 *
 *   "u" (light userdata) [void*]
 *   "U" (full userdata) [void*]
 *     Convert a userdata to a C pointer.
 *
 *   "O" (any Lua value) [int (stack index)]
 *     Convert any Lua value to its stack index.
 *
 *   "O/<type>" (any Lua value) [int (stack index)]
 *     Like "O", but check whether the type of a value is <type>.
 *     <type> must be one of the followings:
 *
 *     N  nil
 *     n  number
 *     s  string
 *     f  function
 *     Q  boolean
 *     u  light userdata
 *     U  full userdata
 *     t  table
 *     T  thread
 *
 *   "{<entry>...}" (table) [...]
 *     Convert values in the table to C values
 *     specified by zero or more <entry>s.
 *     <entry> must be one of the followings:
 *
 *     "<value>"
 *       Look for the value corresponding to the key N,
 *       then convert it as specified by <value>.
 *
 *       The key N is an integer greater than or equal to 1.
 *       N is 1 for the 1st <value>, 2 for the 2nd <value>, and so forth.
 *
 *       <value> is one of the format items other than "O" and "O/<type>".
 *       This may be also "{...}".
 *
 *     "=<key><value>"
 *       Like "<value>, but the key is specified by <key>.
 *
 *       <key> takes a C value, convert it to a Lua value,
 *       then use it as the key to look for the value to be converted.
 *       <key> is one of the followings:
 *
 *         "s"  (string)  [const char*]
 *         "n"  (number)  [lua_Number]
 *         "i"  (number)  [int]
 *         "l"  (number)  [long]
 *
 *     "?<key><value>"
 *       Like "=<key><value>", but the corresponding value may not be found.
 *       If it is not found, this entry will be skipped and
 *       the corresponding C variables will not be touched.
 *       So the C variables should be initialized to their default values.
 *
 *   "|"
 *     This character indicates that the remaining format items are optional.
 *     If parameters for the optional items are omitted,
 *     the corresponding C variables will not be touched.
 *     So the C variables should be initialized to their default values,
 */

Crj_PUBLIC(void) Crj_ParseTable(lua_State* L, int table_index,
                                const char* format, ...);
/* Like Crj_ParseArgs, but parse the table indexed by `table_index'.
 * `format' is same as the one of Crj_ParseArgs,
 * but it must be surrounded with `{' and `}'.
 */

/*}}}*/

Crj_PUBLIC(void) Crj_BuildValues(lua_State*L, const char*format, ...);  /*{{{*/
/* Build values according to format, and push them to the Lua stack.
 *
 * Return values:
 *   [success]
 *     nothing
 *
 *   [failure]
 *     nothing; throw a Lua error.
 *
 * Format:
 *
 *   A format string consists of zer or more format items.
 *   The Nth format item corresponds the Nth value to be pushed on the stack.
 *   Spaces between format items are ignored for readability.
 *
 *   To describe each format item, use the following template:
 *     "format item" (Lua type to be built) [C type(s) to be passed]
 *
 *   "n" (number) [lua_Number]
 *     Convert a lua_Number to a Lua number.
 *
 *   "b" (number) [char]
 *   "h" (number) [short]
 *   "i" (number) [int]
 *   "l" (number) [long]
 *   "B" (number) [unsigned char]
 *   "H" (number) [unsigned short]
 *   "I" (number) [unsigned int]
 *   "L" (number) [unsigned long]
 *     Like "n", but convert an integer of the specified type.
 *
 *     BUGS: "b", "B", "h" and "H" are same as "i".
 *     Becase the corresponding types are promoted to int
 *     when passed through `...'.
 *
 *   "s" (string) [const char*]
 *     Convert a C pointer to a string to a Lua string.
 *     The string must be NUL-terminated.
 *
 *   "s#" (string) [const char*, size_t]
 *     Like "s", but take the string length.
 *     The string may not be NUL-terminated.
 *
 *   "Q" (boolean) [BOOL = int]
 *     Convert a C boolean to a Lua boolean.
 *
 *   "u" (light userdata) [void*]
 *     Convert a C pointer to a light userdata.
 *
 *   "F" (function) [lua_CFunction]
 *     Convert a lua_CFunction to a function.
 *
 *   "O" (any Lua value) [int (stack index)]
 *     Convert a stack index to a Lua value.
 *
 *   "{<entry>...}" (table) [...]
 *     Build a table and set values to it as specified by <entry>s.
 *
 *     <entry> is one of the followings:
 *
 *     "<value>"
 *       Build a value specified by <value>,
 *       then set it to the table with the key N.
 *
 *       The key N is an integer greater than or equal to 1.
 *       N is 1 for the 1st <value>, 2 for the 2nd <value>, and so forth.
 *
 *       <value> is one of the format items.
 *
 *     "=<key><value>"
 *       Build two values specified by <key> and <value>,
 *       then set the latter value to the table
 *       with the former value as the key.
 *
*       <key> and <value> are one of the format items,
 */  /*}}}*/




/* Extension
 * ---------
 */

typedef enum {
	Crj_EXT_LOADING = 1,
	Crj_EXT_UNLOADING = 2
} CrjExtEvent;

typedef int Crj_ExtMainF(lua_State* L);
/* The entry point lua_CFunction of an extension.
 * This function is called when an event is occured.
 *
 * Parameters:
 *   All parameters are passed as single table with string keys.
 *
 *   Template:
 *     "key" (value) [event]
 *
 *   "event" (number) [all event]
 *     The identifier of an occured event.
 *
 *   "hmodule" (light userdata) [all event]
 *     HMODULE of the extension.
 *
 *   "namespace" (table) [Crj_EXT_LOAD]
 *     The namespace of the caller module.
 *
 * Return values:
 *   noting
 */




#endif  /* _CRJ_CEREJA_H */
/* vim: foldmethod=marker
 */
