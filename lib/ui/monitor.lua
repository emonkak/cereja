-- Module: ui.monitor - monitor-related API
--
-- Copyright (C) 2007 kana <http://nicht.s8.xrea.com/>
-- $Id$

module('ui.monitor', package.seeall)
cereja.load_extension("ui\\monitor", ui.monitor)




function ui.monitor.get_count()
  return #(ui.monitor.find_all())
end




do
  local function find_by_number(n)
    return ui.monitor.find_all()[n]
  end

  local function get_number_of(hmonitor)
    local hmonitors = ui.monitor.find_all()

    for n, v in ipairs(hmonitors) do
      if hmonitor == v then
        return n
      end
    end

    return error(string.format('Argument %s is not a valid hmonitor',
                               tostring(hmonitor)))
  end

  function ui.monitor.find_next_monitor(delta, base_hmonitor)
    local base_hmonitor = base_hmonitor or ui.monitor.find_by_window()
    local n = get_number_of(base_hmonitor)
    return find_by_number(1 + ((n-1) + delta) % ui.monitor.get_count())
  end
end




-- __END__
