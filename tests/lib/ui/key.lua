-- Test: Module ui.key
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

-- BUGS [NOT_TESTED]: ui.key.pressedp

package.path = '.\\lunit\\?.lua;' .. package.path
require('lunit')

require('ui.key')
for k, v in pairs(ui.key) do
  if string.match(k, "^[^_]") then
    _G[k] = v
  end
end








lunit.wrap("string_to_vk and vk_name_table_add", function ()
  lunit.assert_equal(VK_TAB, string_to_vk("tab"))
  lunit.assert_equal(VK_TAB, string_to_vk("tAb"))
  lunit.assert_equal(VK_TAB, string_to_vk("TAB"))

  lunit.assert_error(function () string_to_vk("tabby") end)
  vk_name_table_add("tAbBy", VK_TAB)
  lunit.assert_pass(function () string_to_vk("tabby") end)
  lunit.assert_equal(VK_TAB, string_to_vk("tabby"))
end)


lunit.wrap("string_to_modifier and modifier_name_table_add", function ()
  lunit.assert_equal(MOD_CONTROL, string_to_modifier("control"))
  lunit.assert_equal(MOD_CONTROL, string_to_modifier("cOnTrOl"))
  lunit.assert_equal(MOD_CONTROL, string_to_modifier("CONTROL"))

  lunit.assert_error(function () string_to_modifier("controlly") end)
  modifier_name_table_add("ContRolLy", MOD_CONTROL)
  lunit.assert_pass(function () string_to_modifier("conTRolly") end)
  lunit.assert_equal(MOD_CONTROL, string_to_modifier("controlly"))
end)


lunit.wrap("string_to_vk_and_modifiers", function ()
  vk, modifiers = string_to_vk_and_modifiers("Ctrl-Alt-Delete")
  lunit.assert_equal(VK_DELETE, vk)
  lunit.assert_equal(bor(MOD_CONTROL, MOD_ALT), modifiers)

  lunit.assert_error(function () string_to_vk_and_modifiers("-C-A-Delete") end)
  lunit.assert_error(function () string_to_vk_and_modifiers("C--A-Delete") end)
  lunit.assert_error(function () string_to_vk_and_modifiers("C-A-") end)
end)








lunit.run()

-- __END__
-- vim: et sts=2 sw=2
