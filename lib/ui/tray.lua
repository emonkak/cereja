-- Module: ui.tray - user interface for tray
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

module('ui.tray', package.seeall)
require('shell.tray')
require('ui.key')

cereja.load_extension("ui\\tray", ui.tray)
shell.tray.subscribe('ui.tray', _subscriber)




function activate()
  set_visibility(true)
end

function hotkey_add(key, command)
  local vk, modifiers = ui.key.string_to_vk_and_modifiers(key)
  return _hotkey_add(vk, modifiers, command)
end




-- default key-bindings.

hotkey_add("Esc", ui.tray.CMD_HIDE)

hotkey_add("Down", ui.tray.CMD_CURSOR_NEXT)
hotkey_add("N", ui.tray.CMD_CURSOR_NEXT)
hotkey_add("J", ui.tray.CMD_CURSOR_NEXT)

hotkey_add("Up", ui.tray.CMD_CURSOR_PREV)
hotkey_add("P", ui.tray.CMD_CURSOR_PREV)
hotkey_add("K", ui.tray.CMD_CURSOR_PREV)

hotkey_add("Home", ui.tray.CMD_CURSOR_FIRST)
hotkey_add("End", ui.tray.CMD_CURSOR_LAST)

hotkey_add("L", ui.tray.CMD_SEND_LEFT_CLICK)
hotkey_add("Shift-L", ui.tray.CMD_SEND_LEFT_DOUBLE_CLICK)
hotkey_add("R", ui.tray.CMD_SEND_RIGHT_CLICK)
hotkey_add("Shift-R", ui.tray.CMD_SEND_RIGHT_DOUBLE_CLICK)
hotkey_add("M", ui.tray.CMD_SEND_MIDDLE_CLICK)
hotkey_add("Shift-M", ui.tray.CMD_SEND_MIDDLE_DOUBLE_CLICK)

hotkey_add("Shift-Down", ui.tray.CMD_ICON_MOVE_NEXT)
hotkey_add("Shift-N", ui.tray.CMD_ICON_MOVE_NEXT)
hotkey_add("Shift-J", ui.tray.CMD_ICON_MOVE_NEXT)
hotkey_add("Shift-Up", ui.tray.CMD_ICON_MOVE_PREV)
hotkey_add("Shift-P", ui.tray.CMD_ICON_MOVE_PREV)
hotkey_add("Shift-K", ui.tray.CMD_ICON_MOVE_PREV)

-- __END__
