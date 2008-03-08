-- Module: ui.key - key-related API
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

module('ui.key', package.seeall)




cereja.load_extension("ui\\key", ui.key)




function pressedp(name)
  return _pressedp(string_to_vk(name))
end




do
  local vk_name_table = {}

  function vk_name_table_add(name, vk)
    vk_name_table[string.lower(name)] = vk
    return
  end

  function string_to_vk(name)
    local v = vk_name_table[string.lower(name)]
    if not v then
      error(string.format("Key name `%s' is not valid one", name))
    end
    return v
  end
end


do
  local modifier_name_table = {}

  function modifier_name_table_add(name, modifier)
    modifier_name_table[string.lower(name)] = modifier
    return
  end

  function string_to_modifier(name)
    local v = modifier_name_table[string.lower(name)]
    if not v then
      error(string.format("Modifier name `%s' is not valid one", name))
    end
    return v
  end
end


do
  local function split(s, pattern)
    local list = {}
    local i_part = 1
    while true do
      local i_begin, i_end = string.find(s, pattern, i_part)
      if not i_begin then break end

      table.insert(list, string.sub(s, i_part, i_begin-1))
      i_part = i_end+1
    end

    table.insert(list, string.sub(s, i_part, -1))
    return list
  end

  function string_to_vk_and_modifiers(s)
    local tokens = split(s, "-")
    local modifiers = 0
    local vk

    for i = 1, #tokens - 1 do
      modifiers = bor(modifiers, string_to_modifier(tokens[i]))
    end
    vk = string_to_vk(tokens[#tokens])

    return vk, modifiers
  end
end








-- Modifier names ======================================================== {{{1

MOD_ALT = 0x1
MOD_CONTROL = 0x2
MOD_SHIFT = 0x4
MOD_WIN = 0x8

modifier_name_table_add("A", MOD_ALT)
modifier_name_table_add("Alt", MOD_ALT)
modifier_name_table_add("M", MOD_ALT)
modifier_name_table_add("Meta", MOD_ALT)
modifier_name_table_add("C", MOD_CONTROL)
modifier_name_table_add("Ctrl", MOD_CONTROL)
modifier_name_table_add("Control", MOD_CONTROL)
modifier_name_table_add("S", MOD_SHIFT)
modifier_name_table_add("Shift", MOD_SHIFT)
modifier_name_table_add("W", MOD_WIN)
modifier_name_table_add("Win", MOD_WIN)




-- VK names ============================================================== {{{1

-- Constants ------------------------------------------------------------- {{{2

VK_LBUTTON = 0x01
VK_RBUTTON = 0x02
VK_CANCEL = 0x03
VK_MBUTTON = 0x04
VK_XBUTTON1 = 0x05
VK_XBUTTON2 = 0x06
-- 0x07    ; Undefined
VK_BACK = 0x08
VK_TAB = 0x09
-- 0x0A - 0x0B    ; Reserved
VK_CLEAR = 0x0C
VK_RETURN = 0x0D
-- 0x0E - 0x0F    ; Undefined
VK_SHIFT = 0x10
VK_CONTROL = 0x11
VK_MENU = 0x12
VK_PAUSE = 0x13
VK_CAPITAL = 0x14
VK_KANA = 0x15
VK_HANGUEL = 0x15
VK_HANGUL = 0x15
-- 0x16    ; Undefined
VK_JUNJA = 0x17
VK_FINAL = 0x18
VK_HANJA = 0x19
VK_KANJI = 0x19
-- 0x1A    ; Undefined
VK_ESCAPE = 0x1B
VK_CONVERT = 0x1C
VK_NONCONVERT = 0x1D
VK_ACCEPT = 0x1E
VK_MODECHANGE = 0x1F
VK_SPACE = 0x20
VK_PRIOR = 0x21
VK_NEXT = 0x22
VK_END = 0x23
VK_HOME = 0x24
VK_LEFT = 0x25
VK_UP = 0x26
VK_RIGHT = 0x27
VK_DOWN = 0x28
VK_SELECT = 0x29
VK_PRINT = 0x2A
VK_EXECUTE = 0x2B
VK_SNAPSHOT = 0x2C
VK_INSERT = 0x2D
VK_DELETE = 0x2E
VK_HELP = 0x2F
VK_0 = 0x30
VK_1 = 0x31
VK_2 = 0x32
VK_3 = 0x33
VK_4 = 0x34
VK_5 = 0x35
VK_6 = 0x36
VK_7 = 0x37
VK_8 = 0x38
VK_9 = 0x39
-- 0x3A - 0x40    ; Undefined
VK_A = 0x41
VK_B = 0x42
VK_C = 0x43
VK_D = 0x44
VK_E = 0x45
VK_F = 0x46
VK_G = 0x47
VK_H = 0x48
VK_I = 0x49
VK_J = 0x4A
VK_K = 0x4B
VK_L = 0x4C
VK_M = 0x4D
VK_N = 0x4E
VK_O = 0x4F
VK_P = 0x50
VK_Q = 0x51
VK_R = 0x52
VK_S = 0x53
VK_T = 0x54
VK_U = 0x55
VK_V = 0x56
VK_W = 0x57
VK_X = 0x58
VK_Y = 0x59
VK_Z = 0x5A
VK_LWIN = 0x5B
VK_RWIN = 0x5C
VK_APPS = 0x5D
-- 0x5E    ; Reserved
VK_SLEEP = 0x5F
VK_NUMPAD0 = 0x60
VK_NUMPAD1 = 0x61
VK_NUMPAD2 = 0x62
VK_NUMPAD3 = 0x63
VK_NUMPAD4 = 0x64
VK_NUMPAD5 = 0x65
VK_NUMPAD6 = 0x66
VK_NUMPAD7 = 0x67
VK_NUMPAD8 = 0x68
VK_NUMPAD9 = 0x69
VK_MULTIPLY = 0x6A
VK_ADD = 0x6B
VK_SEPARATOR = 0x6C
VK_SUBTRACT = 0x6D
VK_DECIMAL = 0x6E
VK_DIVIDE = 0x6F
VK_F1 = 0x70
VK_F2 = 0x71
VK_F3 = 0x72
VK_F4 = 0x73
VK_F5 = 0x74
VK_F6 = 0x75
VK_F7 = 0x76
VK_F8 = 0x77
VK_F9 = 0x78
VK_F10 = 0x79
VK_F11 = 0x7A
VK_F12 = 0x7B
VK_F13 = 0x7C
VK_F14 = 0x7D
VK_F15 = 0x7E
VK_F16 = 0x7F
VK_F17 = 0x80
VK_F18 = 0x81
VK_F19 = 0x82
VK_F20 = 0x83
VK_F21 = 0x84
VK_F22 = 0x85
VK_F23 = 0x86
VK_F24 = 0x87
-- 0x88 - 0x8F    ; Unassigned
VK_NUMLOCK = 0x90
VK_SCROLL = 0x91
-- 0x92 - 0x96    ; OEM specific
-- 0x97 - 0x9F    ; Unassigned
VK_LSHIFT = 0xA0
VK_RSHIFT = 0xA1
VK_LCONTROL = 0xA2
VK_RCONTROL = 0xA3
VK_LMENU = 0xA4
VK_RMENU = 0xA5
VK_BROWSER_BACK = 0xA6
VK_BROWSER_FORWARD = 0xA7
VK_BROWSER_REFRESH = 0xA8
VK_BROWSER_STOP = 0xA9
VK_BROWSER_SEARCH = 0xAA
VK_BROWSER_FAVORITES = 0xAB
VK_BROWSER_HOME = 0xAC
VK_VOLUME_MUTE = 0xAD
VK_VOLUME_DOWN = 0xAE
VK_VOLUME_UP = 0xAF
VK_MEDIA_NEXT_TRACK = 0xB0
VK_MEDIA_PREV_TRACK = 0xB1
VK_MEDIA_STOP = 0xB2
VK_MEDIA_PLAY_PAUSE = 0xB3
VK_LAUNCH_MAIL = 0xB4
VK_LAUNCH_MEDIA_SELECT = 0xB5
VK_LAUNCH_APP1 = 0xB6
VK_LAUNCH_APP2 = 0xB7
-- 0xB8 - 0xB9    ; Reserved
VK_OEM_1 = 0xBA
VK_OEM_PLUS = 0xBB
VK_OEM_COMMA = 0xBC
VK_OEM_MINUS = 0xBD
VK_OEM_PERIOD = 0xBE
VK_OEM_2 = 0xBF
VK_OEM_3 = 0xC0
-- 0xC1 - 0xD7    ; Reserved
-- 0xD8 - 0xDA    ; Unassigned
VK_OEM_4 = 0xDB
VK_OEM_5 = 0xDC
VK_OEM_6 = 0xDD
VK_OEM_7 = 0xDE
VK_OEM_8 = 0xDF
-- 0xE0    ; Reserved
-- 0xE1    ; OEM specific
VK_OEM_102 = 0xE2
-- 0xE3 - 0xE4    ; OEM specific
VK_PROCESSKEY = 0xE5
-- 0xE6    ; OEM specific
VK_PACKET = 0xE7
-- 0xE8    ; Unassigned
-- 0xE9 - 0xF5    ; OEM specific
VK_ATTN = 0xF6
VK_CRSEL = 0xF7
VK_EXSEL = 0xF8
VK_EREOF = 0xF9
VK_PLAY = 0xFA
VK_ZOOM = 0xFB
VK_NONAME = 0xFC
VK_PA1 = 0xFD
VK_OEM_CLEAR = 0xFE


-- Alias ----------------------------------------------------------------- {{{2

-- basic names (FOO for VK_FOO)
for k, v in pairs(ui.key) do
  local m = string.match(k, "^VK_([%u%d]+)$")
  if m then
    vk_name_table_add(m, ui.key[k])
  end
end


-- other useful names
vk_name_table_add("backspace", VK_BACK)
vk_name_table_add("enter", VK_RETURN)
vk_name_table_add("ctrl", VK_CONTROL)
vk_name_table_add("alt", VK_MENU)
vk_name_table_add("meta", VK_MENU)
vk_name_table_add("caps", VK_CAPITAL)
vk_name_table_add("capslock", VK_CAPITAL)
vk_name_table_add("esc", VK_ESCAPE)
vk_name_table_add("pageup", VK_PRIOR)
vk_name_table_add("pagedown", VK_NEXT)
vk_name_table_add("rolldown", VK_PRIOR)
vk_name_table_add("rollup", VK_NEXT)
vk_name_table_add("printscreen", VK_SNAPSHOT)
vk_name_table_add("ins", VK_INSERT)
vk_name_table_add("del", VK_DELETE)
vk_name_table_add("_0", VK_0)
vk_name_table_add("_1", VK_1)
vk_name_table_add("_2", VK_2)
vk_name_table_add("_3", VK_3)
vk_name_table_add("_4", VK_4)
vk_name_table_add("_5", VK_5)
vk_name_table_add("_6", VK_6)
vk_name_table_add("_7", VK_7)
vk_name_table_add("_8", VK_8)
vk_name_table_add("_9", VK_9)
vk_name_table_add("num0", VK_NUMPAD0)
vk_name_table_add("num1", VK_NUMPAD1)
vk_name_table_add("num2", VK_NUMPAD2)
vk_name_table_add("num3", VK_NUMPAD3)
vk_name_table_add("num4", VK_NUMPAD4)
vk_name_table_add("num5", VK_NUMPAD5)
vk_name_table_add("num6", VK_NUMPAD6)
vk_name_table_add("num7", VK_NUMPAD7)
vk_name_table_add("num8", VK_NUMPAD8)
vk_name_table_add("num9", VK_NUMPAD9)
vk_name_table_add("scrolllock", VK_SCROLL)

-- }}}




-- __END__
-- vim: et sts=2 sw=2 foldmethod=marker
