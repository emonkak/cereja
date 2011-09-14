/* UTF-8 wrapper API
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */
/* NOTE: Windows uses UTF-16 as its native encoding. */

#ifndef _Utf8_UTF8API_H
#define _Utf8_UTF8API_H

#if (!defined(UNICODE)) || (!defined(_UNICODE))
#  error "Not supported"
#endif
#ifndef _WIN32_IE
#  define _WIN32_IE 0x0500  /* SHGetFolderPath */
#endif

#include <windows.h>
#include <shellapi.h>  /* ShellExecute */
#include <shlobj.h>  /* SHGetFolderPath */
#include <tchar.h>
#include <mmsystem.h>

#include <stdio.h>
#include <malloc.h>




#define Utf8_PUBLIC(type) __declspec(dllexport) type


/* The maximum ratio of A to B,
 * where A is the length of any character encoded by UTF-16 in WCHAR,
 * and B is the length of the same character encoded by UTF-8 in CHAR. */
#define Utf8_NATIVE_RATIO 3


Utf8_PUBLIC(int) Utf8_ToNative(LPTSTR dest, int dest_size,
                               LPCSTR src, int src_size);
Utf8_PUBLIC(int) Utf8_FromNative(LPSTR dest, int dest_size,
                                 LPCTSTR src, int src_size);




/* Wrappers
 * ========
 *
 * - Naming convensions:
 *
 *   FooBar     mapped to FooBarU.
 *
 *   FooBarU    UTF-8 wrapper for FooBarT.
 *
 *   FooBarT    mapped to FooBarW or FooBarA.
 *
 * - Wrappers for some APIs are not provided, because:
 *
 *   CONSTANT   These APIs usually take literal strings.
 *
 *   STRUCTURE  These APIs take a structure in which the parameters are packed.
 *
 *   RARE       These APIs are rarely used.
 */

/* CreateProcess [RARE] */
/* CreateWindowEx [CONSTANT] */
Utf8_PUBLIC(HWND) FindWindowU(LPCSTR, LPCSTR);
Utf8_PUBLIC(DWORD) FormatMessageU(DWORD, LPCVOID, DWORD, DWORD, LPSTR, DWORD,
                                  va_list*);
Utf8_PUBLIC(DWORD) GetModuleFileNameU(HMODULE, LPSTR, DWORD);
/* GetModuleHandle [RARE] */
Utf8_PUBLIC(HMODULE) LoadLibraryU(LPCSTR);
Utf8_PUBLIC(int) MessageBoxU(HWND, LPCSTR, LPCSTR, UINT);
/* RegisterClass/WNDCLASS/UnregisterClass [CONSTANT/STRUCTURE] */
Utf8_PUBLIC(HRESULT) SHGetFolderPathU(HWND, int, HANDLE, DWORD, LPSTR);
Utf8_PUBLIC(HINSTANCE) ShellExecuteU(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR,INT);

Utf8_PUBLIC(BOOL) PlaySoundU(LPCSTR, HMODULE, DWORD);

Utf8_PUBLIC(FILE*) fopenU(const char*, const char*);
#define fopenW _wfopen

#include "utf8api.h.gen"  /* auto-generated API mappings */




Utf8_PUBLIC(int) Utf8_GetEnv(const char* varname, char* buf, int buf_size);

/* getenv(3)-like wrapper for Utf8_GetEnv, implemented with alloca(3). */
#define Utf8_GetEnvA(_varname)                                          \
        __extension__ ({                                                \
                const char* varname = (_varname);                       \
                int required_buf_size = Utf8_GetEnv(varname, NULL, 0);  \
                char* buf;                                              \
                                                                        \
                if (0 < required_buf_size) {                            \
                        buf = alloca(required_buf_size);                \
                        Utf8_GetEnv(varname, buf, required_buf_size);   \
                } else {                                                \
                        buf = NULL;                                     \
                }                                                       \
                                                                        \
                buf;                                                    \
        })




#endif  /* _Utf8_UTF8API_H */
