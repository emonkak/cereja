-- Test: Built-in API (Lua part)
--
-- Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
-- $Id$

package.path = '.\\lunit\\?.lua;' .. package.path
require('lunit')

local function printf(format, ...)
  print(string.format(format, ...))
end

local function assert_error(f, ...)
  local args = {...}
  lunit.assert_error(function () f(unpack(args)) end)
end

local function assert_pass(f, ...)
  local args = {...}
  lunit.assert_pass(function () f(unpack(args)) end)
end








-- Patch part =================================================================

lunit.wrap("os.pathexistsp", function ()
  lunit.assert_true(os.pathexistsp("."));
  lunit.assert_true(os.pathexistsp(".\\libcereja.lua"));
  lunit.assert_false(os.pathexistsp("No such file or directory"));
end)




lunit.wrap("package.findfile", function ()
  local function check(expected, basename, path)
    local result = package.findfile(basename, path)
    lunit.assert_equal(expected, result)
  end

  -- ordinary cases
  check('src\\api.c', 'api', 'src\\?.c;tests\\?.c');
  check('tests\\api.c', 'api', 'tests\\?.c;src\\?.c');

  -- not found case
  check(false, 'No such file or directory', 'tests\\?.c;src\\?.c');

  -- rare cases
  check('.\\src\\api.c', 'src\\api.c', '.\\?');  -- just 1 dir in path
  check(false, 'src\\api.c', '');  -- no dir in path
end)




lunit.wrap("bitwise operations", function ()
  -- bnot) 0...0001
  -- --------------
  --       1...1110
  lunit.assert_equal(-0x2, bnot(0x1))

  --      0011
  -- bor) 0101
  -- ---------
  --      0111
  lunit.assert_equal(0x7, bor(0x3, 0x5))

  --       0011
  -- band) 0101
  -- ---------
  --       0001
  lunit.assert_equal(0x1, band(0x3, 0x5))

  --       0011
  -- bxor) 0101
  -- ---------
  --       0110
  lunit.assert_equal(0x6, bxor(0x3, 0x5))

  --          0101
  -- blshift)    1
  -- -------------
  --          1010
  lunit.assert_equal(0xA, blshift(0x5, 0x1))

  --          1010
  -- brshift)    1
  -- -------------
  --          0101
  lunit.assert_equal(0x5, brshift(0xA, 0x1))
end)




lunit.wrap("range", function ()
  local t

  t = range(0, 3)
  lunit.assert_equal(3, #t)
  lunit.assert_equal(0, t[1])
  lunit.assert_equal(1, t[2])
  lunit.assert_equal(2, t[3])

  t = range(3, 7)
  lunit.assert_equal(4, #t)
  lunit.assert_equal(3, t[1])
  lunit.assert_equal(4, t[2])
  lunit.assert_equal(5, t[3])
  lunit.assert_equal(6, t[4])

  t = range(9, 7)
  lunit.assert_equal(0, #t)

  t = range(5)
  lunit.assert_equal(5, #t)
  lunit.assert_equal(0, t[1])
  lunit.assert_equal(1, t[2])
  lunit.assert_equal(2, t[3])
  lunit.assert_equal(3, t[4])
  lunit.assert_equal(4, t[5])

  t = range(-2)
  lunit.assert_equal(0, #t)
end)


lunit.wrap("mapx", function ()
  local t, u

  t = range(1, 3+1)
  lunit.assert_equal(3, #t)
  lunit.assert_equal(1, t[1])
  lunit.assert_equal(2, t[2])
  lunit.assert_equal(3, t[3])

  u = mapx(function (v) return v * v end, t)
  lunit.assert_equal(3, #u)
  lunit.assert_equal(1, u[1])
  lunit.assert_equal(4, u[2])
  lunit.assert_equal(9, u[3])
  lunit.assert_equal(t, u)
end)


lunit.wrap("sortx", function ()
  local t, u

  t = {5, -2, 3, -4, 1}
  u = sortx(t, function (v) return (v < 0 and -v) or v end)
  lunit.assert_equal(5, #u)
  lunit.assert_equal(1, u[1])
  lunit.assert_equal(-2, u[2])
  lunit.assert_equal(3, u[3])
  lunit.assert_equal(-4, u[4])
  lunit.assert_equal(5, u[5])
  lunit.assert_equal(t, u)
end)








-- cereja part ================================================================

lunit.wrap("exported names", function ()
  lunit.assert_table(cereja)

  local keys = {}
  for k, v in pairs(cereja) do
    table.insert(keys, k)
  end
  table.sort(keys)

  for i = 1, #keys do
    printf("%s\t%s", tostring(keys[i]), type(cereja[keys[i]]))
  end
end)




lunit.wrap("cereja.load_extension", function ()
  package.cpath = ".\\?.dll" .. ";" .. package.cpath

  -- non-existent extension
  assert_error(cereja.load_extension, "foo0_bar1_baz2");

  -- fail to load
  namespace = {mode="failure", flag=false}
  assert_error(cereja.load_extension, ".\\tests\\builtins_lua_dll", namespace)

  -- success
  local function ord(c) return string.byte(c) end
  namespace = {mode="success", flag=100}
  assert_pass(cereja.load_extension, ".\\tests\\builtins_lua_dll", namespace)
  lunit.assert_equal(100 + (ord('w')+ord('a')+ord('x')), namespace.flag)

  -- load again, but nothing happend
  assert_pass(cereja.load_extension, ".\\tests\\builtins_lua_dll", namespace)
  lunit.assert_equal(100 + (ord('w')+ord('a')+ord('x')), namespace.flag)
end)








lunit.run()

-- __END__
-- vim: et sts=2 sw=2
