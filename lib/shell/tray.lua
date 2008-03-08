-- Module: shell.tray - tray service
--
-- Copyright (C) 2006-2008 kana <http://nicht.s8.xrea.com/>
-- $Id$

require('shell')
module('shell.tray', package.seeall)




do
  local _subscribers = {}


  function subscribe(group_name, procedure)
    if type(group_name) ~= 'string' then
      error(string.format('group_name: Expected string, but given %s',
                          type(group_name)))
    end
    if type(procedure) ~= 'function' then
      error(string.format('procedure: Expected function, but given %s',
                          type(procedure)))
    end

    table.insert(_subscribers,
                 {group_name=group_name, procedure=procedure,
                  error_time=0, error_count=0})
    return
  end

  function unsubscribe(specifier)
    local n = 0
    for i, v in ipairs(_subscribers) do
      if specifier == v.group_name or specifier == v.procedure then
        table.remove(_subscribers, i)
        n = n + 1
      end
    end
    return n
  end


  AUTO_UNSUBSCRIBING_TIME = 60  -- sec.
  AUTO_UNSUBSCRIBING_COUNT = 3

  function publish(event, icon_index, flags)
    local i = 1
    while i <= #_subscribers do
      local v = _subscribers[i]
      local succeedp, message = pcall(v.procedure, event, icon_index, flags)
      if not succeedp then
        cereja.notice_errorf('shell.tray', 'The following error occured in subscriber <%s>:\n%s', tostring(v.procedure), message)
        local t = os.time()
        if t - v.error_time < AUTO_UNSUBSCRIBING_TIME then
          v.error_count = v.error_count + 1
          if AUTO_UNSUBSCRIBING_COUNT <= v.error_count then
            unsubscribe(v.procedure)
            cereja.notice_errorf('shell.tray', 'Subscriber <%s> seems to be buggy because it raised too many errors in short term, so that it has been automatically unsubscribed.', tostring(v.procedure))
            i = i - 1
          end
        else
          v.error_time = t
          v.error_count = 1
        end
      end
      i = i + 1
    end
  end
end




function move_icon_first(index)
  return move_icon(index, 1)
end

function move_icon_last(index)
  return move_icon(index, -1)
end

function shift_icons(diff)
  local from, to
  if diff < 0 then
    from = 1
    to = -1
  else
    from = -1
    to = 1
  end

  for i = 1, math.abs(diff) do
    move_icon(from, to)
  end
  return
end

local function default_sort_icons_key(index)
  return string.upper(shell.tray.get_icon_data(index, 'szTip'))
end
function sort_icons(key)
  key = key or default_sort_icons_key
  _reorder_icons(sortx(range(1, get_icon_count()+1), key))
end




-- must load here, because subscriber API will be used in this extension.
cereja.load_extension("shell\\tray", shell.tray)

-- __END__
-- vim: et sts=2 sw=2
