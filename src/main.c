/* cereja - an alternative shell for Windows
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include "cereja.h"
#include "version.h"
#include <getopt.h>








/* MISC. =============================================================== {{{ */


/* ETC. --------------------------------------------------------------- {{{2 */
	/* forward */
static void cleanup_cereja_part(lua_State*);


static BOOL
pcall(lua_State* L, lua_CFunction cfunc, const void* userdata)
{
	if (Crj_CPCall(L, cfunc, Crj_FORCE_CAST(void*, userdata)) != 0) {
		Crj_NoticeError(L, "core", lua_tostring(L, -1));
		return FALSE;
	}
	return TRUE;
}




static int
run_file(lua_State* L)
{
	const char* filename;
	char path[MAX_PATH * Utf8_NATIVE_RATIO];
	int  required_size;
	Crj_ParseArgs(L, "u", &filename);

	Crj_UserDirectory(L, path, Crj_NUMBER_OF(path));
	required_size = strlen(path) + 1 + strlen(filename) + 1;
	if (((int)Crj_NUMBER_OF(path)) < required_size) {
		luaL_error(L,
		  "insufficient buffer (required %d, given %d, copied `%s')",
		  required_size,
		  Crj_NUMBER_OF(path),
		  path);
	}
	strcat(path, "\\");
	strcat(path, filename);

	if (luaL_loadfile(L, path) || Crj_PCall(L, 0, 0))
		lua_error(L);
	return 0;
}


static int
run_string(lua_State* L)
{
	const char* s;
	Crj_ParseArgs(L, "u", &s);

	if (luaL_loadstring(L, s) || Crj_PCall(L, 0, 0))
		lua_error(L);
	return 0;
}




/* START-UP / CLEAN-UP : Lua part ------------------------------------- {{{2 */

static int
l_panic(lua_State* L)
{
	const char* message;

	message = lua_tostring(L, -1);
	if (message == NULL)
		message = "(internal error) Invalid error message)";
	Crj_Panic(message);
	/* never reached */
	return 0;
}

static lua_State*
startup_lua_part(void)
{
	lua_State* L;

	L = luaL_newstate();
	if (L == NULL) {
		Crj_NoticeError(NULL, "core",
		                "Lua library failed to initialize.");
		return NULL;
	}
	lua_atpanic(L, &l_panic);
	luaL_openlibs(L);
	_Crj_OpenBuiltins(L);
	return L;
}


static void
cleanup_lua_part(lua_State* L)
{
	_Crj_CloseBuiltins(L);
	lua_close(L);
}




/* START-UP / CLEAN-UP : cereja part ---------------------------------- {{{2 */

static HWND s_MainWindow = NULL;
static lua_State* s_L = NULL;

#define IDHK_EXIT 1
#define NONSENSE 0


static BOOL
confirm_userp(HWND hwnd, const char* caption, const char* message)
{
	return IDYES == MessageBox(hwnd, message, caption,
	                           (MB_YESNO | MB_DEFBUTTON2
	                            | MB_ICONINFORMATION
	                            | MB_TASKMODAL
	                            | MB_TOPMOST));
}


static LRESULT CALLBACK
main_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	default:
		break;

	case WM_CREATE:
		if (!RegisterHotKey(hwnd, IDHK_EXIT,
		                    MOD_ALT|MOD_CONTROL|MOD_SHIFT|MOD_WIN,
		                    VK_F1))
		{
			return -1;
		}
		break;

	case WM_DESTROY:
		UnregisterHotKey(hwnd, IDHK_EXIT);
		break;

	case WM_CLOSE:
		if (confirm_userp(hwnd, "Exit " Crj_APPNAME "?",
		                  "Are you sure you want to exit the shell?"))
		{
			PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;

	case WM_ENDSESSION:
		if (wp) {
			pcall(s_L, run_file, "logout.lua");
			cleanup_cereja_part(s_L);
			cleanup_lua_part(s_L);
		}
		return 0;

	case WM_HOTKEY:
		if (wp == IDHK_EXIT)
			SendMessage(hwnd, Crj_WM_EXIT, 0, 0);
		break;

	case WM_COPYDATA:
		SendMessage(hwnd, ((COPYDATASTRUCT*)lp)->dwData,
		            0, (LPARAM)(((COPYDATASTRUCT*)lp)->lpData));
		return TRUE;

	case Crj_WM_EXIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return NONSENSE;

	case Crj_WM_EXEC: {
		int top = lua_gettop(s_L);

		if (luaL_loadstring(s_L,(const char*)lp) || Crj_PCall(s_L,0,0))
			Crj_NoticeError(s_L, "core", lua_tostring(s_L, -1));

		lua_settop(s_L, top);
		} return NONSENSE;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}


static BOOL
is_there_another_cereja_instancep(void)
{
	return FindWindow(Crj_MAIN_WINDOW_CLASS,Crj_MAIN_WINDOW_TITLE) != NULL;
}


static void
startup_cereja_part(lua_State* L)
{
	WNDCLASS wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.style = 0;
	wc.lpfnWndProc = main_window_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT(Crj_MAIN_WINDOW_CLASS);
	if (!RegisterClass(&wc)) {
		lua_pushstring(L, "RegisterClass failed");
		goto E_RegisterClass;
	}

	s_L = L;
	s_MainWindow = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
	                              TEXT(Crj_MAIN_WINDOW_CLASS),
	                              TEXT(Crj_MAIN_WINDOW_TITLE),
	                              WS_POPUP | WS_VISIBLE,
	                              -13, -13,
	                              0, 0,
	                              NULL,
	                              NULL,
	                              GetModuleHandle(NULL),
	                              NULL);
	if (s_MainWindow == NULL) {
		lua_pushstring(L, "CreateWindowEx failed");
		goto E_CreateWindowEx;
	}
	return;
E_CreateWindowEx:
	UnregisterClass(TEXT(Crj_MAIN_WINDOW_CLASS), GetModuleHandle(NULL));
E_RegisterClass:
	lua_error(L);
}


static void
cleanup_cereja_part(lua_State* L __attribute__((unused)))
{
	DestroyWindow(s_MainWindow);
	s_MainWindow = NULL;

	UnregisterClass(TEXT(Crj_MAIN_WINDOW_CLASS), GetModuleHandle(NULL));
}




/* COMMAND-LINE OPTIONS ----------------------------------------------- {{{2 */

static const char SHORT_OPTIONS[] = ":e:E:f:ht:v";
const struct option LONG_OPTIONS[] = {
	{"exec", required_argument, NULL, 'e'},
	{"file", required_argument, NULL, 'f'},
	{"help", no_argument, NULL, 'h'},
	{"pre-exec", required_argument, NULL, 'E'},
	{"test", required_argument, NULL, 't'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};


#define HELP_MESSAGE "\
Usage: %s [OPTION]...\n\
\n\
Options:\n\
  -e, --exec=SCRIPT\n\
  \tRun SCRIPT by the instance which is already running.\n\
  -f, --file=FILE\n\
  \tEquivalent to ``--exec dofile(\"FILE\")''.\n\
  -E, --pre-exec=SCRIPT\n\
  \tRun SCRIPT before loading ``login.lua''.\n\
\n\
  -h, --help\n\
  \tShow this help message, then exit.\n\
  -v, --version\n\
  \tShow version information, then exit.\n\
\n\
Run as the shell if options other than --pre-exec are not given,\n\
otherwise exit."
static void
show_help_message(void)
{
	Crj_NoticeInfoF(NULL, "core", HELP_MESSAGE, Crj_APPNAME);
}


#define VERSION_MESSAGE "\
%s %s (%s, compiled %s %s)\n\
%s\n\
\n\
For latest information, see <%s>."
static void
show_version_message(void)
{
	Crj_NoticeInfoF(NULL, "core", VERSION_MESSAGE,
	                Crj_APPNAME, Crj_VERSION,
	                Crj_RELEASE_DATE, __DATE__, __TIME__,
	                Crj_COPYRIGHT,
	                Crj_WEBSITE_URI);
}


static void
send_script(const char* script)
{
	HWND hwnd;
	COPYDATASTRUCT cds;

	hwnd = FindWindow(Crj_MAIN_WINDOW_CLASS, Crj_MAIN_WINDOW_TITLE);
	if (hwnd == NULL) {
		Crj_NoticeErrorF(NULL, "core",
		  "There is no cereja instance to run the script:\n%s",
		  script);
		return;
	}

	cds.dwData = Crj_WM_EXEC;
	cds.cbData = (strlen(script) + 1) * sizeof(char);
	cds.lpData = Crj_FORCE_CAST(void*, script);
	SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cds);
}


static BOOL
process_command_line_options(int ac, char** av, const char** p_pre_script)
{
	int option;
	BOOL result;

	opterr = 0;
	result = FALSE;
	for (;;) {
		option = getopt_long(ac, av, SHORT_OPTIONS,LONG_OPTIONS, NULL);
		switch (option) {
		default:
		case '?':
			Crj_NoticeErrorF(NULL, "core",
			  "The given command-line option `%s' is not valid.",
			  av[optind - 1]);
			result = TRUE;
			goto L_END_OPTIONS;
		case ':':
			Crj_NoticeErrorF(NULL, "core",
			  "The command-line option `%s' requires an argument",
			  av[optind - 1]);
			result = TRUE;
			goto L_END_OPTIONS;
		case -1:
			goto L_END_OPTIONS;

		case 'e':
			send_script(optarg);
			result = TRUE;
			break;
		case 'E':
			*p_pre_script = optarg;
			break;
		case 'f': {
			char script[6+2+strlen(optarg)+2 + 1];

			sprintf(script, "dofile('%s')", optarg);
			send_script(script);
			}
			result = TRUE;
			break;
		case 'h':
			show_help_message();
			result = TRUE;
			goto L_END_OPTIONS;
		case 't': {
			lua_State* L;

			L = startup_lua_part();
			if (L != NULL) {
			  if (luaL_loadfile(L, optarg) || Crj_PCall(L, 0, 0))
			    puts(lua_tostring(L, -1));
			  cleanup_lua_part(L);
			}
			result = TRUE;
			} goto L_END_OPTIONS;
		case 'v':
			show_version_message();
			result = TRUE;
			goto L_END_OPTIONS;
		}
	}
L_END_OPTIONS:
	/* trailing non-option arguments are ignored. */
	return result;
}




/* MAIN-LOOP ---------------------------------------------------------- {{{2 */

static int
main_loop(lua_State* L __attribute__((unused)))
{
	BOOL result;
	MSG msg;

	while (TRUE) {
		result = GetMessage(&msg, NULL, 0, 0);
		if (result == 0)
			break;
		if (result == -1)
			break;

		DispatchMessage(&msg);
	}
	return 0;
}

/* }}} */








/* MAIN =============================================================== {{{1 */

static int
inner_main(int ac, char** av)
{
	lua_State* L;
	BOOL result;
	const char* pre_script = NULL;

	if (process_command_line_options(ac, av, &pre_script))
		return EXIT_SUCCESS;

	if (is_there_another_cereja_instancep()) {
		Crj_NoticeErrorF(NULL, "core", "%s is already running.",
		                 Crj_APPNAME);
		return EXIT_FAILURE;
	}

	L = startup_lua_part();
	if (L == NULL)
		return EXIT_FAILURE;
	startup_cereja_part(L);

	result = (((pre_script == NULL)
	           || pcall(L, run_string, pre_script))
	          && pcall(L, run_file, "login.lua")
	          && pcall(L, main_loop, NULL)
	          && pcall(L, run_file, "logout.lua"));

	cleanup_cereja_part(L);
	cleanup_lua_part(L);
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}


#ifdef UNICODE
int
main(int ac, char** av)
{
	const LPWSTR W_CMDLINE = GetCommandLineW();
	int u_buf_len;

	u_buf_len = Utf8_FromNative(NULL, 0, W_CMDLINE, -1);
	if (u_buf_len == (int)wcslen(W_CMDLINE) + 1) {  /* all args in ASCII */
		return inner_main(ac, av);
	} else {
		LPWSTR* w_av;
		int w_ac;

		w_av = CommandLineToArgvW(W_CMDLINE, &w_ac);
		if (w_av == NULL) {
			return EXIT_FAILURE;
		} else {
			char* u_av[w_ac];
			char u_buf[u_buf_len];
			int u_buf_rest = u_buf_len;
			char* u_p;
			int result;
			int i;

			for (i = 0, u_p = u_buf; i < w_ac; i++) {
				result = Utf8_FromNative(u_p, u_buf_rest,
				                         w_av[i], -1);
				u_av[i] = u_p;
				u_p += result;
				u_buf_rest -= result;
			}
			LocalFree(w_av);

			return inner_main(w_ac, u_av);
		}
	}
}
#else
#  error "Not supported"
#endif

/* __END__ */
/* vim: foldmethod=marker
 */
