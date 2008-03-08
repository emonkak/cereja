-- Built-in API (Lua part)
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$




local function errorf(arg1, arg2, ...)
  local level, format

  if type(arg1) == "number" then
    -- level, format, ...
    level = arg1
    format = arg2
  else
    -- format, ...
    level = 0
    format = arg1
  end

  error(string.format(format, ...), 2 + level)
end




do
  cereja._loaded_extensions = {}

  function cereja.load_extension(extname, namespace)
    if cereja._loaded_extensions[extname] then
      return
    end

    local extpath = package.findfile(extname, package.cpath)
    if not extpath then
      errorf(1, "Extension `%s' is not found", extname)
    end

    cereja._loaded_extensions[extname] = true  -- to avoid infinite recursion
    local success, hmodule_or_error_message = pcall(cereja._load_extension,
                                                    extpath, namespace)
    if not success then
      cereja._loaded_extensions[extname] = nil  -- to allow reloading again
      errorf(1, "Extension `%s' is failed to load, because:\n%s",
             extname, hmodule_or_error_message)
    end
    cereja._loaded_extensions[extname] = hmodule_or_error_message
  end
end




do
  function cereja.noticef(category, noticer, format, ...)
    return cereja.notice(category, noticer, string.format(format, ...))
  end

  function cereja.notice_debug(noticer, message)
    return cereja.notice(cereja.NOTICE_DEBUG, noticer, message)
  end
  function cereja.notice_info(noticer, message)
    return cereja.notice(cereja.NOTICE_INFO, noticer, message)
  end
  function cereja.notice_warning(noticer, message)
    return cereja.notice(cereja.NOTICE_WARNING, noticer, message)
  end
  function cereja.notice_error(noticer, message)
    return cereja.notice(cereja.NOTICE_ERROR, noticer, message)
  end

  function cereja.notice_debugf(noticer, f, ...)
    return cereja.notice(cereja.NOTICE_DEBUG, noticer, string.format(f, ...))
  end
  function cereja.notice_infof(noticer, f, ...)
    return cereja.notice(cereja.NOTICE_INFO, noticer, string.format(f, ...))
  end
  function cereja.notice_warningf(noticer, f, ...)
    return cereja.notice(cereja.NOTICE_WARNING, noticer, string.format(f, ...))
  end
  function cereja.notice_errorf(noticer, f, ...)
    return cereja.notice(cereja.NOTICE_ERROR, noticer, string.format(f, ...))
  end


  local notice_handler_list = {}

  function cereja.add_notice_handler(handler)
    table.insert(notice_handler_list, 1, handler)
    return
  end

  function cereja.remove_notice_handler(handler)
    for i, v in ipairs(notice_handler_list) do
      if handler == v then
        table.remove(notice_handler_list, i)
        break
      end
    end
    return
  end

  function cereja.call_notice_handlers(category, noticer, message)
    local noticedp = false
    for i, v in ipairs(notice_handler_list) do
      succeededp, result = pcall(v, category, noticer, message, noticedp)
      if succeededp then
        noticedp = result
      else
        cereja.remove_notice_handler(v)  -- to avoid infinite recursion.
        cereja.notice_errorf("core",
          "The following error is occured in user notice handler <%s>:"
            .."\n>>> %s"
            .."\nThe original message is:"
            .."\ncategory=%s"
            .."\nnoticer=%s"
            .."\nmessage=%s",
          tostring(v), result, tostring(category), noticer, message)
        return true
      end
    end
    return noticedp
  end
end








-- Additional API for the standard library.

do
  package.path = ''
  package.cpath = ''
  for _, base_dir in ipairs({cereja.user_directory,cereja.system_directory}) do
    for _, prefix in ipairs({'\\lib\\', '\\lib\\lua\\'}) do
      for _, suffix in ipairs({'', '\\__init__', '\\init'}) do
        package.path = (package.path .. ';'
                        .. base_dir .. prefix .. '?' .. suffix .. '.lua')
        package.cpath = (package.cpath .. ';'
                         .. base_dir .. prefix .. '?' .. suffix .. '.dll')
      end
    end
  end
  package.path = package.path:sub(2, -1)
  package.cpath = package.cpath:sub(2, -1)
end

function package.findfile(basename, path)
  for template in string.gmatch(path, '([^;]+);?') do
    local filepath = string.gsub(template, '%?', basename)
    if os.pathexistsp(filepath) then
      return filepath
    end
  end
  return false
end

-- ``require'' wrapper to allow reloading a module again.
do
  local require_orig = require

  function require(modname)
     local succeededp, result_or_error_message = pcall(require_orig, modname)
     if not succeededp then
       package.loaded[modname] = nil  -- the original one doesn't clear this.
       error(result_or_error_message)
     end
     return result_or_error_message
  end
end




function string.split(s, pattern)
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




function range(start, stop)
  if stop == nil then
    start, stop = 0, start
  end

  local t = {}
  local i = start
  while i < stop do
    t[i - start + 1] = i
    i = i + 1
  end
  return t
end

function mapx(f, tbl)
  for i, v in ipairs(tbl) do
    tbl[i] = f(v)
  end
  return tbl
end

function sortx(tbl, key)
  if key == nil then
    table.sort(tbl)
    return tbl
  end
  tbl = mapx(function (v) return {key(v), v} end, tbl)
  table.sort(tbl, function (a, b) return a[1] < b[1] end)
  tbl = mapx(function (v) return v[2] end, tbl)
  return tbl
end




-- __END__
-- vim: et sts=2 sw=2
