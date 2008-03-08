/* Test: UTF-8 wrapper API
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */
/* Code range (hex) |  Scalar value (binary)      |  UTF-8 (binary)
 * ===================================================================
 * 000000-00007F    |  0zzzzzzz                   |  0zzzzzzz
 * -----------------+-----------------------------+-------------------
 * 000080-0007FF    |  00000yyy yyzzzzzz          |  110yyyyy 10zzzzzz
 * -----------------+-----------------------------+-------------------
 * 000800-00FFFF    |  xxxxyyyy yyzzzzzz          |  1110xxxx 10yyyyyy
 *                  |                             |  10zzzzzz
 * -----------------+-----------------------------+-------------------
 * 010000-10FFFF    |  000wwwxx xxxxyyyy yyzzzzzz |  11110www 10xxxxxx
 *                  |                             |  10yyyyyy 10zzzzzz
 *
 * Code range (hex) |  Scalar value (binary)      |  UTF-16 (binary)
 * ===================================================================
 * 000000-00FFFF    |  yyyyyyyy zzzzzzzz          |  yyyyyyyy zzzzzzzz
 * -----------------+-----------------------------+-------------------
 * 010000-10FFFF    |  0000xxxx yyyyyyyy zzzzzzzz |  110110xx xxyyyyyy
 *                  |  (subtracted by 010000)     |  110111yy zzzzzzzz
 *
 * NOTE: Windows uses UTF-16.
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include "utf8api.h"

#define NUMBER_OF(array) (sizeof((array)) / sizeof((array)[0]))








TEST_PROC(Utf8_ToNative)
{
	CHAR bs[16];
	WCHAR ws[16];
	int result;

	/* 000000-00007F */
		/* 00007F = 00000000 00000000 01111111 */
	strcpy(bs, "\x7F");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0x007F, ws[0]);
	TEST_ASSERT_INT(L'\0', ws[1]);

	/* 000080-0007FF */
		/* 000080 = 00000000 00000000 10000000 */
	strcpy(bs, "\xC2\x80");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0x0080, ws[0]);
	TEST_ASSERT_INT(L'\0', ws[1]);
		/* 0007FF = 00000000 00000111 11111111 */
	strcpy(bs, "\xDF\xBF");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0x07FF, ws[0]);
	TEST_ASSERT_INT(L'\0', ws[1]);

	/* 000800-00FFFF */
		/* 000800 = 00000000 00001000 00000000 */
	strcpy(bs, "\xE0\xA0\x80");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0x0800, ws[0]);
	TEST_ASSERT_INT(L'\0', ws[1]);
		/* 00FFFF = 00000000 11111111 11111111 */
	strcpy(bs, "\xEF\xBF\xBF");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0xFFFF, ws[0]);
	TEST_ASSERT_INT(L'\0', ws[1]);

	/* 010000-10FFFF */
		/* 010000 = 00000001 00000000 00000000 */
	strcpy(bs, "\xF0\x90\x80\x80");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(2+1, result);
	TEST_ASSERT_INT(0xD800, ws[0]);
	TEST_ASSERT_INT(0xDC00, ws[1]);
	TEST_ASSERT_INT(L'\0', ws[2]);
		/* 10FFFF = 00010000 11111111 11111111 */
	strcpy(bs, "\xF4\x8F\xBF\xBF");
	result = Utf8_ToNative(ws, NUMBER_OF(ws), bs, -1);
	TEST_ASSERT_INT(2+1, result);
	TEST_ASSERT_INT(0xDBFF, ws[0]);
	TEST_ASSERT_INT(0xDFFF, ws[1]);
	TEST_ASSERT_INT(L'\0', ws[2]);
}




TEST_PROC(Utf8_FromNative)
{
	UCHAR bs[16];
	WCHAR ws[16];
	int result;

	/* 000000-00007F */
		/* 00007F = 00000000 00000000 01111111 */
	wcscpy(ws, L"\x007F");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(1+1, result);
	TEST_ASSERT_INT(0x7F, bs[0]);
	TEST_ASSERT_INT('\0', bs[1]);

	/* 000080-0007FF */
		/* 000080 = 00000000 00000000 10000000 */
	wcscpy(ws, L"\x0080");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(2+1, result);
	TEST_ASSERT_INT(0xC2, bs[0]);
	TEST_ASSERT_INT(0x80, bs[1]);
	TEST_ASSERT_INT('\0', bs[2]);
		/* 0007FF = 00000000 00000111 11111111 */
	wcscpy(ws, L"\x07FF");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(2+1, result);
	TEST_ASSERT_INT(0xDF, bs[0]);
	TEST_ASSERT_INT(0xBF, bs[1]);
	TEST_ASSERT_INT('\0', bs[2]);

	/* 000800-00FFFF */
		/* 000800 = 00000000 00001000 00000000 */
	wcscpy(ws, L"\x0800");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(3+1, result);
	TEST_ASSERT_INT(0xE0, bs[0]);
	TEST_ASSERT_INT(0xA0, bs[1]);
	TEST_ASSERT_INT(0x80, bs[2]);
	TEST_ASSERT_INT('\0', bs[3]);
		/* 00FFFF = 00000000 11111111 11111111 */
	wcscpy(ws, L"\xFFFF");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(3+1, result);
	TEST_ASSERT_INT(0xEF, bs[0]);
	TEST_ASSERT_INT(0xBF, bs[1]);
	TEST_ASSERT_INT(0xBF, bs[2]);
	TEST_ASSERT_INT('\0', bs[3]);

	/* 010000-10FFFF */
		/* 010000 = 00000001 00000000 00000000 */
	wcscpy(ws, L"\xD800\xDC00");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(4+1, result);
	TEST_ASSERT_INT(0xF0, bs[0]);
	TEST_ASSERT_INT(0x90, bs[1]);
	TEST_ASSERT_INT(0x80, bs[2]);
	TEST_ASSERT_INT(0x80, bs[3]);
	TEST_ASSERT_INT('\0', bs[4]);
		/* 10FFFF = 00010000 11111111 11111111 */
	wcscpy(ws, L"\xDBFF\xDFFF");
	result = Utf8_FromNative((LPSTR)bs, NUMBER_OF(bs), ws, -1);
	TEST_ASSERT_INT(4+1, result);
	TEST_ASSERT_INT(0xF4, bs[0]);
	TEST_ASSERT_INT(0x8F, bs[1]);
	TEST_ASSERT_INT(0xBF, bs[2]);
	TEST_ASSERT_INT(0xBF, bs[3]);
	TEST_ASSERT_INT('\0', bs[4]);
}




TEST_PROC(FormatMessageU)
{
	DWORD result;
	char buf[10];
	char* bufp;

	result = FormatMessageU(FORMAT_MESSAGE_FROM_STRING,
	                        TEXT("BWV 784"), 0, 0,
	                        buf, NUMBER_OF(buf), NULL);
	TEST_ASSERT_INT(7, result);
	TEST_ASSERT_STR("BWV 784", buf);

	result = FormatMessageU(FORMAT_MESSAGE_FROM_STRING,
	                        TEXT("\x0800"), 0, 0,
	                        buf, NUMBER_OF(buf), NULL);
	TEST_ASSERT_INT(3, result);
	TEST_ASSERT_STR("\xE0\xA0\x80", buf);

	result = FormatMessageU(FORMAT_MESSAGE_FROM_STRING
	                          | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	                        TEXT("\x0800"), 0, 0,
	                        (LPSTR)&bufp, 0, NULL);
	TEST_ASSERT_INT(3, result);
	TEST_ASSERT_STR("\xE0\xA0\x80", bufp);
	LocalFree(bufp);
}




TEST_PROC(Utf8_GetEnv)
{
	char buf[3*4 + 1];
	int buf_size;

	/* When the variable exists. */
	_wputenv(L"\x65e5\x672c\x8a9e" L"\x003d" L"\x306b\x307b\x3093\x3054");

	buf_size = Utf8_GetEnv("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e", NULL, 0);
	TEST_ASSERT_INT(3*4 + 1, buf_size);

	buf_size = Utf8_GetEnv("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e",
	                       buf, NUMBER_OF(buf));
	TEST_ASSERT_INT(3*4 + 1, buf_size);
	TEST_ASSERT_STR("\xe3\x81\xab\xe3\x81\xbb\xe3\x82\x93\xe3\x81\x94",
	                buf);

	/* When the variable does not exist. */
	_wputenv(L"\x65e5\x672c\x8a9e" L"\x003d" L"");

	buf_size = Utf8_GetEnv("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e", NULL, 0);
	TEST_ASSERT_INT(0, buf_size);
}

TEST_PROC(Utf8_GetEnvA)
{
	const char* value;

	/* When the variable exists. */
	_wputenv(L"\x65e5\x672c\x8a9e" L"\x003d" L"\x306b\x307b\x3093\x3054");

	value = Utf8_GetEnvA("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");
	TEST_ASSERT_PTR_NOT_NULL(value);
	TEST_ASSERT_STR("\xe3\x81\xab\xe3\x81\xbb\xe3\x82\x93\xe3\x81\x94",
	                value);

	/* When the variable does not exist. */
	_wputenv(L"\x65e5\x672c\x8a9e" L"\x003d" L"");

	value = Utf8_GetEnvA("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");
	TEST_ASSERT_PTR_NULL(value);
}




/* __END__ */
/* vim: foldmethod=marker
 */
