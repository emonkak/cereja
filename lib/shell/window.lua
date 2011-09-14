-- Module:  shell.window - shell window procedure

module('shell.window', package.seeall)
cereja.load_extension("shell\\window", shell.window)


HSHELL_ACTIVATESHELLWINDOW = 3
HSHELL_ENDTASK = 10
HSHELL_GETMINRECT = 5
HSHELL_LANGUAGE = 8
HSHELL_REDRAW = 6
HSHELL_TASKMAN = 7
HSHELL_WINDOWACTIVATED = 4
HSHELL_WINDOWCREATED = 1
HSHELL_WINDOWDESTROYED = 2
HSHELL_ACCESSIBILITYSTATE = 11
HSHELL_APPCOMMAND = 12
HSHELL_RUDEAPPACTIVATED = 32772
HSHELL_FLASH = 32774
HSHELL_WINDOWREPLACED = 13
HSHELL_WINDOWREPLACING = 14


WM_POWERBROADCAST = 536

PBT_APMQUERYSUSPEND = 0
PBT_APMQUERYSTANDBY = 1
PBT_APMQUERYSUSPENDFAILED = 2
PBT_APMQUERYSTANDBYFAILED = 3
PBT_APMSUSPEND = 4
PBT_APMSTANDBY = 5
PBT_APMRESUMECRITICAL = 6
PBT_APMRESUMESUSPEND = 7
PBT_APMRESUMESTANDBY = 8
PBT_APMBATTERYLOW = 9
PBT_APMPOWERSTATUSCHANGE = 10
PBT_APMOEMEVENT = 11
PBT_APMRESUMEAUTOMATIC = 18
PBT_POWERSETTINGCHANGE = 32787


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

  function publish(msg, event, hwnd)
    local i = 1
    while i <= #_subscribers do
      local v = _subscribers[i]
      local succeedp, message = pcall(v.procedure, msg, event, hwnd)
      if not succeedp then
        cereja.notice_errorf('shell', 'The following error occured in subscriber <%s>:\n%s', tostring(v.procedure), message)
        local t = os.time()
        if t - v.error_time < AUTO_UNSUBSCRIBING_TIME then
          v.error_count = v.error_count + 1
          if AUTO_UNSUBSCRIBING_COUNT <= v.error_count then
            unsubscribe(v.procedure)
            cereja.notice_errorf('shell', 'Subscriber <%s> seems to be buggy because it raised too many errors in short term, so that it has been automatically unsubscribed.', tostring(v.procedure))
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



-- __END__
