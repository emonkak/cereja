-- Module: ui.window - window-related API
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

module('ui.window', package.seeall)
cereja.load_extension("ui\\window", ui.window)

require('ui.monitor')




local function get_target_window(hwnd)
  return ((hwnd ~= NULL) and hwnd) or ui.window.get_foreground_window()
end




activate = raise


do
  local function convert(tx, dx, wx, ww, mx, mw, vx, vw)
    local bx, bw
    local coordinate = string.sub(tx, 1, 1)
    local unit = string.sub(tx, 2)
    if string.match(coordinate, '%u') then
      bx, bw = vx, vw
    else
      bx, bw = mx, mw
    end
    if unit == '%' then
      dx = math.floor(bw * dx / 100)
    else  -- if unit == 'px' then
      -- nothing to do.
    end

    coordinate = string.lower(coordinate)
    if coordinate == 'a' then
      -- ``<cond> and <then> or <else>'' -- same as C's ``<c> ? <t> : <e>''.
      return bx + (0 <= dx and 0 or bw - ww + 1) + dx
    elseif coordinate == 'c' then
      return bx + math.floor(bw/2) - math.floor(ww/2) + dx
    else -- if coordinate == 'r' then
      return wx + dx
    end
  end

  function move(tx, ty, dx, dy, hwnd)
    hwnd = get_target_window(hwnd)
    local hmonitor = ui.monitor.find_by_window(hwnd)

    local wx, wy, ww, wh = get_placement(hwnd)
    local mx, my, mw, mh = unpack(ui.monitor.get_info(hmonitor).placement)
    local vx, vy, vw, vh = unpack(ui.monitor.get_vscreen_placement())

    local x = convert(tx, dx, wx, ww, mx, mw, vx, vw)
    local y = convert(ty, dy, wy, wh, my, mh, vy, vh)

    return _move(x, y, hwnd);
  end
end


function snap_to_left(...) return move("a", "r", 0, 0, ...) end
function snap_to_top(...) return move("r", "a", 0, 0, ...) end
function snap_to_right(...) return move("a", "r", -1, 0, ...) end
function snap_to_bottom(...) return move("r", "a", 0, -1, ...) end
function snap_to_center(...) return move("c", "c", 0, 0, ...) end


do
  local PREFIX = 'cereja.ui.window:'
  local function getprop(hwnd, key)
    return get_property(hwnd, PREFIX..key)
  end
  local function setprop(hwnd, key, value)
    return set_property(hwnd, PREFIX..key, value)
  end

  function maximize_vertically(hwnd)
    hwnd = get_target_window(hwnd)

    local zoomedp = getprop(hwnd, 'vmaximize.zoomedp')
    if zoomedp then
      setprop(hwnd, 'vmaximize.zoomedp', false)
      local x, _, width, _ = get_placement(hwnd)
      local _, y, _, height = unpack(getprop(hwnd, 'vmaximize.placement'))
      move('A', 'A', x, y, hwnd)
      resize(width, height, hwnd)
    else
      setprop(hwnd, 'vmaximize.zoomedp', true)
      local x, y, width, height = get_placement(hwnd)
      setprop(hwnd, 'vmaximize.placement', {x, y, width, height})
      local monitor_info = ui.monitor.get_info(ui.monitor.find_by_window(hwnd))
      move('A', 'A', x, monitor_info.placement[2], hwnd)
      resize(width, monitor_info.placement[4], hwnd)
    end
  end

  function maximize_horizontally(hwnd)
    hwnd = get_target_window(hwnd)

    local zoomedp = getprop(hwnd, 'hmaximize.zoomedp')
    if zoomedp then
      setprop(hwnd, 'hmaximize.zoomedp', false)
      local _, y, _, height = get_placement(hwnd)
      local x, _, width, _ = unpack(getprop(hwnd, 'hmaximize.placement'))
      move('A', 'A', x, y, hwnd)
      resize(width, height, hwnd)
    else
      setprop(hwnd, 'hmaximize.zoomedp', true)
      local x, y, width, height = get_placement(hwnd)
      setprop(hwnd, 'hmaximize.placement', {x, y, width, height})
      local monitor_info = ui.monitor.get_info(ui.monitor.find_by_window(hwnd))
      move('A', 'A', monitor_info.placement[1], y, hwnd)
      resize(monitor_info.placement[3], height, hwnd)
    end
  end
end




-- FIXME: consider the workarea of dest_hmonitor.
function move_to_monitor(dest_hmonitor, hwnd)
  hwnd = get_target_window(hwnd)
  local src_hmonitor = ui.monitor.find_by_window(hwnd)

  local wl, wt, ww, wh = ui.window.get_placement(hwnd)
  local wr, wb = wl + ww, wt + wh
  local sl, st, sw, sh = unpack(ui.monitor.get_info(src_hmonitor).placement)
  local sr, sb = sl + sw, st + sh
  local dl, dt, dw, dh = unpack(ui.monitor.get_info(dest_hmonitor).placement)
  local dr, db = dl + dw, dt + dh

  wl = wl + (dl - sl)
  wt = wt + (dt - st)
  wr = wr + (dl - sl)
  wb = wb + (dt - st)

  if dr < wr then
    wl = wl - (wr - dr)
    wr = wr - (wr - dr)
  end
  if wl < dl then
    wl = wl + (dl - wl)
    wr = wr + (dl - wl)
  end
  if db < wb then
    wt = wt - (wb - db)
    wb = wb - (wb - db)
  end
  if wt < dt then
    wt = wt + (dt - wt)
    wb = wb + (dt - wt)
  end

  return ui.window.move('A', 'A', wl, wt, hwnd)
end


function move_to_next_monitor(delta, hwnd)
  hwnd = get_target_window(hwnd)
  local base_hmonitor = ui.monitor.find_by_window(hwnd)
  local dest_hmonitor = ui.monitor.find_next_monitor(delta, base_hmonitor)
  return ui.window.move_to_monitor(dest_hmonitor, hwnd)
end


function move_visibly(hwnd)
  hwnd = get_target_window(hwnd)
  return ui.window.move_to_monitor(ui.monitor.find_by_window(hwnd), hwnd)
end




function get_application_list()
  local list = {}
  ui.window.enumerate(function (hwnd)
    local style = ui.window.get_style(hwnd)
    local exstyle = ui.window.get_exstyle(hwnd)
    if (band(style, ui.window.WS_VISIBLE) == 0) then return true end
    if (band(style, ui.window.WS_DISABLED) ~= 0) then return true end
    if (band(exstyle, ui.window.WS_EX_TOOLWINDOW) ~= 0) then return true end
    table.insert(list, hwnd)
    return true
  end)
  return list
end

function show_application_list()
  local s = ''
  for _, hwnd in ipairs(ui.window.get_application_list()) do
    s = s .. string.format("\n\nClass=``%s''\nTitle=``%s''",
                           ui.window.get_class_name(hwnd),
                           ui.window.get_title_name(hwnd))
  end
  cereja.notice_info('ui.window', s)
end




-- get_minimized_metrics/set_minimized_metrics: `arrange' flags
ARW_BOTTOMLEFT = 0x0000
ARW_BOTTOMRIGHT = 0x0001
ARW_TOPLEFT = 0x0002
ARW_TOPRIGHT = 0x0003

ARW_LEFT = 0x0000
ARW_RIGHT = 0x0000
ARW_UP = 0x0004
ARW_DOWN = 0x0004
ARW_HIDE = 0x0008




-- Window Properties
--
-- NOTE: This API is not a wrapper of Windows API, because:
--
--       A. Lua GC doesn't know anything about window properties.
--          So a value of a property may be collected in some cases.
--
--       B. Settable objects are very limited and some objects (e.g. tables)
--          are not directly accessible from C.
--
-- BUGS: Management of hwnd_table is not perfect.
--       It should use shell hook to check destroying windows.
do
  -- hwnd_table = {entry*}
  -- entry = {hwnd, properties}
  -- properties = {[key]=value, ...}
  local hwnd_table = {}

  local function search_properties(hwnd)
    local i = 1
    while i <= #hwnd_table do
      if not validp(hwnd_table[i][1]) then
        table.remove(hwnd_table, i)
      elseif hwnd == hwnd_table[i][1] then
        return hwnd_table[i][2]
      else
        i = i + 1
      end
    end
    return nil
  end


  function get_property(hwnd, key)
    local properties = search_properties(hwnd)
    if not properties then
      return nil
    end
    return properties[key]  -- is nil if key does not exist.
  end


  function set_property(hwnd, key, new_value)
    if not ui.window.validp(hwnd) then
      return nil
    end

    local properties = search_properties(hwnd)
    if not properties then
      properties = {}
      table.insert(hwnd_table, {hwnd, properties})
    end

    local old_value = properties[key]  -- is nil if key does not exist.
    properties[key] = new_value
    return old_value
  end


  function remove_property(hwnd, key)
    local properties = search_properties(hwnd)
    if properties then
      properties[key] = nil  -- assigning nil to remove the entry.
    end
    return
  end
end




-- Misc. Constants

WS_BORDER = 0x800000
WS_CAPTION = 0xc00000
WS_CHILD = 0x40000000
WS_CHILDWINDOW = 0x40000000
WS_CLIPCHILDREN = 0x2000000
WS_CLIPSIBLINGS = 0x4000000
WS_DISABLED = 0x8000000
WS_DLGFRAME = 0x400000
WS_GROUP = 0x20000
WS_HSCROLL = 0x100000
WS_ICONIC = 0x20000000
WS_MAXIMIZE = 0x1000000
WS_MAXIMIZEBOX = 0x10000
WS_MINIMIZE = 0x20000000
WS_MINIMIZEBOX = 0x20000
WS_OVERLAPPED = 0
WS_OVERLAPPEDWINDOW = 0xcf0000
WS_POPUP = 0x80000000
WS_POPUPWINDOW = 0x80880000
WS_SIZEBOX = 0x40000
WS_SYSMENU = 0x80000
WS_TABSTOP = 0x10000
WS_THICKFRAME = 0x40000
WS_TILED = 0
WS_TILEDWINDOW = 0xcf0000
WS_VISIBLE = 0x10000000
WS_VSCROLL = 0x200000

WS_EX_ACCEPTFILES = 16
WS_EX_APPWINDOW = 0x40000
WS_EX_CLIENTEDGE = 512
WS_EX_COMPOSITED = 0x2000000
WS_EX_CONTEXTHELP = 0x400
WS_EX_CONTROLPARENT = 0x10000
WS_EX_DLGMODALFRAME = 1
WS_EX_LAYERED = 0x80000
WS_EX_LAYOUTRTL = 0x400000
WS_EX_LEFT = 0
WS_EX_LEFTSCROLLBAR = 0x4000
WS_EX_LTRREADING = 0
WS_EX_MDICHILD = 64
WS_EX_NOACTIVATE = 0x8000000
WS_EX_NOINHERITLAYOUT = 0x100000
WS_EX_NOPARENTNOTIFY = 4
WS_EX_OVERLAPPEDWINDOW = 0x300
WS_EX_PALETTEWINDOW = 0x188
WS_EX_RIGHT = 0x1000
WS_EX_RIGHTSCROLLBAR = 0
WS_EX_RTLREADING = 0x2000
WS_EX_STATICEDGE = 0x20000
WS_EX_TOOLWINDOW = 128
WS_EX_TOPMOST = 8
WS_EX_TRANSPARENT = 32
WS_EX_WINDOWEDGE = 256




-- __END__
-- vim: et sts=2 sw=2 foldmethod=marker
