-- Configuration sample: config.lua
-- $Id$

-- Hotkey for reload_config should be registered at first
-- to be able to reload this file even if this file is failed to load.
ui.hotkey.reset()
ui.hotkey.register("Alt-Ctrl-Shift-Win-R", reload_config)


-- tray
ui.hotkey.register("Win-T", ui.tray.activate)


-- window
ui.hotkey.register("Alt-Shift-M", ui.window.minimize)
ui.hotkey.register("Alt-Shift-X", ui.window.maximize)
ui.hotkey.register("Alt-Shift-R", ui.window.restore)
ui.hotkey.register("Alt-Shift-V", ui.window.maximize_vertically)
ui.hotkey.register("Alt-Shift-H", ui.window.maximize_horizontally)
ui.hotkey.register("Alt-Shift-A", ui.window.set_always_on_top)
ui.hotkey.register("Alt-Shift-Left", ui.window.snap_to_left)
ui.hotkey.register("Alt-Shift-Up", ui.window.snap_to_top)
ui.hotkey.register("Alt-Shift-Right", ui.window.snap_to_right)
ui.hotkey.register("Alt-Shift-Down", ui.window.snap_to_bottom)
ui.hotkey.register("Alt-Shift-Space", function () ui.window.set_alpha(204) end)


-- misc.
ui.hotkey.register("Win-F12", shell.system_shutdown)
ui.hotkey.register("Win-F11", shell.system_reboot)
ui.hotkey.register("Win-F10", shell.system_logoff)
ui.hotkey.register("Win-F9", shell.system_hibernate)
ui.hotkey.register("Win-F8", shell.system_suspend)
ui.hotkey.register("Win-F7", shell.system_lock)


-- application launcher
function explorer(dir)
  return shell.execute(nil, "explorer", "/E, " .. dir)
end
ui.hotkey.register("Win-Ctrl-E", function () shell.execute("explorer") end)
ui.hotkey.register("Win-Ctrl-I", function () shell.execute("iexplore") end)
ui.hotkey.register("Win-Ctrl-U",
                   function () explorer(cereja.user_directory) end)
ui.hotkey.register("Win-Ctrl-S",
                   function () explorer(cereja.system_directory) end)




-- Reset my subscribers to avoid doubly adding subscribers on reloading.
shell.tray.unsubscribe('user')


-- Sort icons in the tray automatically.
shell.tray.subscribe('user', function (event, icon_index, flags)
  if event == shell.tray.NIM_ADD then
    shell.tray.sort_icons()
  end
  return
end)
shell.tray.sort_icons()


-- Pseudo balloon tooltips by Snarl.
shell.tray.subscribe('user', function (event, icon_index, flags)
  -- Is this an event to show a balloon tooltip?
  if event == shell.tray.NIM_MODIFY
     and band(flags, shell.tray.NIF_INFO) ~= 0
  then
    title = shell.tray.get_icon_data(icon_index, 'szInfoTitle')
    text = shell.tray.get_icon_data(icon_index, 'szInfo')
    if 0 < #text then
      app.snarl.show_message(title, text, 10)
    else
      -- Nothing to do.
      -- Because #text == 0 means a request to hide a balloon tooltip.
    end
  end
  return
end)




-- __END__
-- vim: et sts=2 sw=2
