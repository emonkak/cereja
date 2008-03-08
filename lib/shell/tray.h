/* shell.tray Interface
 *
 * Copyright (C) 2006-2007 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

/* Include this after ``cereja.h''. */




/* Data structure
 * ==============
 *
 * - The reference to the tray data will never be changed while cereja runs.
 *   The reference to icons in the tray may be changed while cereja runs,
 *   but will never be changed while subscribers are being called.
 *
 * - The reference to the tray data can be obtained via a Lua variable named
 *   ``shell.tray.data''.
 */

#ifdef UNICODE
typedef struct {
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	WCHAR szTip[128];
	DWORD dwState;
	DWORD dwStateMask;
	WCHAR szInfo[256];
	_ANONYMOUS_UNION union {
		UINT uTimeout;
		UINT uVersion;
	} DUMMYUNIONNAME;
	WCHAR szInfoTitle[64];
	DWORD dwInfoFlags;
	GUID guidItem;
} CrjNOTIFYICONDATA;  /* NOTIFYICONDATAW, version 6.0 */

typedef struct {
	/* DWORD cbSize; */
	HWND hWnd;
	UINT uID;
	/* UINT uFlags;  -- any member is valid. */
	UINT uCallbackMessage;
	HICON hIcon;
	WCHAR szTip[128];
	/* DWORD dwState; */
	/* DWORD dwStateMask; */
	WCHAR szInfo[256];
	_ANONYMOUS_UNION union {
		UINT uTimeout;
		/* UINT uVersion; */
	} DUMMYUNIONNAME;
	WCHAR szInfoTitle[64];
	DWORD dwInfoFlags;
	/* GUID guidItem; */
} CrjTrayIcon;
#else
#  error "Not supported"
#endif


typedef struct {
	int count;
	CrjTrayIcon** icons;
} CrjTrayData;




/* Notification
 * ============
 *
 * shell.tray provides API to notify changes in the tray.  This notification is
 * done via calling functions which are registered beforehand.  These functions
 * are called as ``subscribers'' and registering/unregistering a subscriber is
 * called as ``subscribing''/``unsubscribing''.
 *
 * - While subscribers are being called, the tray data will never be changed.
 *
 * - Notifications will be done when an icon is added, updated or removed.
 *
 * - Errors raised in subscribers are catched by shell.tray.
 *
 * - When a subscriber raise too many errors in short term, it will be
 *   automatically unsubscribed.
 *
 * - Subscribers can be implemented in C or Lua.
 *   C-subscribers are implemented as lua_CFunction
 *   and are treated as same as Lua-subscribers.
 *
 *
 * Subscriber prototype
 * --------------------
 *
 * subscriber(event, icon_index, flags)
 *
 * Parameters
 *
 *   event [number/DWORD]
 *     Indicates the event which is the reason of this notification.
 *     This value is equivalent to dwMessage of Shell_NotifyIcon.
 *
 *     NIM_ADD
 *       an icon has been added.
 *
 *     NIM_DELETE
 *       an icon has been removed.
 *
 *     NIM_MODIFY
 *       some data of an icon has been updated.
 *
 *   icon_index [number/int]
 *     The index of the icon which has been added or updated.
 *     This index is 1-origin.
 *
 *     FIXME (NIY): icon_index is nonsense on NIM_DELETE,
 *     so it is currently 0 -- invalid index. 
 *     It should be the index of the icon which will be removed.
 *
 *   flags [number/UINT]
 *     Indicates which of the members of CrjTrayIcon has been changed on this
 *     notification.  The meaning of flag values are equivalent to the one of
 *     uFlags of NOTIFYICONDATA.
 *
 *     Note that flags is nonsense on NIM_DELETE.
 *
 * Return values
 *
 *   nothing
 */




/* __END__ */
/* vim: foldmethod=marker
 */
