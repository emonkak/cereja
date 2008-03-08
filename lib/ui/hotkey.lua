-- Module: ui.hotkey - hotkey feature (Lua part)
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

module('ui.hotkey', package.seeall)
require('ui.key')




cereja.load_extension("ui\\hotkey", ui.hotkey)




function register(key_combination, action)
  local vk, modifiers = ui.key.string_to_vk_and_modifiers(key_combination)
  _register(vk, modifiers, action)
  return
end




-- __END__
-- vim: et sts=2 sw=2 foldmethod=marker
