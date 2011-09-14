/* UTF-8 wrapper API
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#define STRICT
#include "utf8api.h"








Utf8_PUBLIC(int)
Utf8_ToNative(LPTSTR dest, int dest_size, LPCSTR src, int src_size)
{
	int result;

	result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
	                             src, src_size, dest, dest_size);
	if (result == 0) {
		result = MultiByteToWideChar(CP_OEMCP, MB_ERR_INVALID_CHARS,
		                             src, src_size, dest, dest_size);
	}
	return result;
}


Utf8_PUBLIC(int)
Utf8_FromNative(LPSTR dest, int dest_size, LPCTSTR src, int src_size)
{
	return WideCharToMultiByte(CP_UTF8, 0, src, src_size, dest, dest_size,
	                           NULL, NULL);
}








/* Wrappers */

#define NUMBER_OF(array) (sizeof((array)) / sizeof((array)[0]))

#define DEFINE_NATIVE_STRING(v)				\
	LPTSTR v##T;						\
	TCHAR v##T_[v!=NULL ? strlen(v)+1 : 0];			\
	if (v != NULL) {					\
		Utf8_ToNative(v##T_, NUMBER_OF(v##T_), v, -1);	\
		v##T = v##T_;					\
	} else {						\
		v##T = NULL;					\
	}




Utf8_PUBLIC(HWND)
FindWindowU(LPCSTR lpClassName, LPCSTR lpWindowName)
{
	DEFINE_NATIVE_STRING(lpClassName);
	DEFINE_NATIVE_STRING(lpWindowName);

	return FindWindowT(lpClassNameT, lpWindowNameT);
}




/* FIXME: All string arguments must be given in native encoding,
 *        not converted automatically. */
Utf8_PUBLIC(DWORD)
FormatMessageU(DWORD dwFlags, LPCVOID lpSource,
               DWORD dwMessageId, DWORD dwLanguageId,
               LPSTR lpBuffer, DWORD nSize,
               va_list* Arguments)
{
	const int ALLOCATEP = dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER;
	DWORD result;
	DWORD cbBufferT = nSize;
	TCHAR lpBufferT_[cbBufferT];
	LPTSTR lpBufferT;
	DWORD cbBufferU;
	LPSTR lpBufferU;

	if (ALLOCATEP)
		lpBufferT = (LPTSTR)(void*)&lpBufferT;
	else
		lpBufferT = lpBufferT_;

	result = FormatMessageT(dwFlags, lpSource, dwMessageId, dwLanguageId,
	                        lpBufferT, cbBufferT, Arguments);
	if (result == 0)
		goto E_FormatMessageT;

	if (ALLOCATEP) {
		cbBufferU = Utf8_FromNative(NULL, 0, lpBufferT, -1);
		lpBufferU = LocalAlloc(LPTR, sizeof(CHAR) * cbBufferU);
		if (lpBufferU == NULL)
			goto E_LocalAlloc;
	} else {
		cbBufferU = nSize;
		lpBufferU = lpBuffer;
	}

	result = Utf8_FromNative(lpBufferU, cbBufferU, lpBufferT, -1);
	if (result == 0)
		goto E_Utf8_FromNative;

	if (ALLOCATEP) {
		*(LPSTR*)lpBuffer = lpBufferU;
		LocalFree(lpBufferT);
	}
	return result - 1;

E_Utf8_FromNative:
	if (ALLOCATEP)
		LocalFree(lpBufferU);
E_LocalAlloc:
	LocalFree(lpBufferT);
E_FormatMessageT:
	return 0;
}




Utf8_PUBLIC(DWORD)
GetModuleFileNameU(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	DWORD result;
	TCHAR lpFilenameT[nSize];

	result = GetModuleFileNameT(hModule, lpFilenameT, nSize);
	if (result == 0)
		return 0;

	lpFilenameT[NUMBER_OF(lpFilenameT) - 1] = TEXT('\0');
	return Utf8_FromNative(lpFilename, nSize, lpFilenameT, -1);
}




Utf8_PUBLIC(HMODULE)
LoadLibraryU(LPCSTR lpFileName)
{
	DEFINE_NATIVE_STRING(lpFileName);

	return LoadLibraryT(lpFileNameT);
}




Utf8_PUBLIC(int)
MessageBoxU(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	DEFINE_NATIVE_STRING(lpText);
	DEFINE_NATIVE_STRING(lpCaption);

	return MessageBoxT(hWnd, lpTextT, lpCaptionT, uType);
}




Utf8_PUBLIC(HRESULT)
SHGetFolderPathU(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags,
                 LPSTR pszPath)
{
	HRESULT result;
	TCHAR pszPathT[MAX_PATH];

	result = SHGetFolderPathT(hwndOwner,nFolder,hToken,dwFlags,pszPathT);
	if (SUCCEEDED(result)) {
		pszPathT[NUMBER_OF(pszPathT) - 1] = TEXT('\0');
		Utf8_FromNative(pszPath, MAX_PATH, pszPathT, -1);
	}

	return result;
}




Utf8_PUBLIC(HINSTANCE)
ShellExecuteU(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile,
              LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	DEFINE_NATIVE_STRING(lpOperation);
	DEFINE_NATIVE_STRING(lpFile);
	DEFINE_NATIVE_STRING(lpParameters);
	DEFINE_NATIVE_STRING(lpDirectory);

	return ShellExecuteT(hwnd, lpOperationT, lpFileT,
	                     lpParametersT, lpDirectoryT, nShowCmd);
}




Utf8_PUBLIC(BOOL)
PlaySoundU(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	DEFINE_NATIVE_STRING(pszSound);

	return PlaySoundT(pszSoundT, hmod, fdwSound);
}



Utf8_PUBLIC(FILE*)
fopenU(const char* filename, const char* mode)
{
	DEFINE_NATIVE_STRING(filename);
	DEFINE_NATIVE_STRING(mode);

	return fopenT(filenameT, modeT);
}




Utf8_PUBLIC(int)
Utf8_GetEnv(const char* varname, char* buf, int buf_size)
{
	DEFINE_NATIVE_STRING(varname);
	LPTSTR varvalue;

	varvalue = _tgetenv(varnameT);
	if (varvalue == NULL)
		return 0;

	return Utf8_FromNative(buf, buf_size, varvalue, -1);
}

/* __END__ */
