
--[[--------------------------------------------------------------------------

    This file is part of lunit 0.3 (alpha).

    For Details about lunit look at: http://www.nessie.de/mroth/lunit/

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2004 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person   
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,   
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY      
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,      
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--]]--------------------------------------------------------------------------




require "lunit"

lunit.import "all"


----------------------------------------------
-- Tests basics. We must assume that errors --
-- thrown by test functions are detected.   --
-- We use the stdlib error() function to    --
-- signal errors instead of assert_fail()   --
----------------------------------------------

tc = TestCase("Basic Tests")

function tc.test_assert_fail()
  local ok, errmsg
  
  ok, errmsg = pcall(function() assert_fail() end)
  if ok then
    error("assert_fail() doesn't fail!")
  end
  
  ok, errmsg = pcall(function() assert_fail("A message") end) 
  if ok then
    error("assert_fail(\"A message\") doesn't fail!")
  end
end

function tc.test_assert_error()
  local ok, errmsg
  
  ok, errmsg = pcall(function() assert_error(function() error("Error!") end) end)
  if not ok then
    error("assert_error( <error> ) doesn't work!")
  end
  
  ok, errmsg = pcall(function() assert_error("A message", function() error("Error") end) end)
  if not ok then
    error("assert_error(\"A message\", <error>) doesn't work!")
  end
  
  ok, errmsg = pcall(function() assert_error(function() end) end)
  if ok then
    error("assert_error( <no error> ) doesn't fail!")
  end
  
  ok, errmsg = pcall(function() assert_error("A Message", function() end) end)
  if ok then
    error("assert_error(\"A message\", <no error>) doesn't fail!")
  end
end

function tc.test_assert_pass()
  local ok, errmsg
  
  ok, errmsg = pcall(function() assert_pass(function() error("Error!") end) end)
  if ok then
    error("assert_pass( <error> ) doesn't fail!")
  end
  
  ok, errmsg = pcall(function() assert_pass("A message", function() error("Error") end) end)
  if ok then
    error("assert_pass(\"A message\", <error>) doesn't fail!")
  end
  
  ok, errmsg = pcall(function() assert_pass(function() end) end)
  if not ok then
    error("assert_pass( <no error> ) doesn't work!")
  end
  
  ok, errmsg = pcall(function() assert_pass("A Message", function() end) end)
  if not ok then
    error("assert_pass(\"A message\", <no error>) doesn't work!")
  end
end

function tc.test_assert_true()
  assert_pass("assert_true(true) doesn't work!", function() assert_true(true) end)
  assert_pass("assert_true(true, \"A message\" doesn't work!", function() assert_true(true, "A Message") end)
  assert_error("assert_true(false) doesn't fail!", function() assert_true(false) end)
  assert_error("assert_true(false, \"A message\" doesn't fail!", function() assert_true(false, "A Message") end)
end

function tc.test_assert_false()
  assert_pass("assert_false(false) doesn't work!", function() assert_false(false) end)
  assert_pass("assert_false(false, \"A message\" doesn't work!", function() assert_false(false, "A Message") end)
  assert_error("assert_false(true) doesn't fail!", function() assert_false(true) end)
  assert_error("assert_false(true, \"A message\" doesn't fail!", function() assert_false(true, "A Message") end)
end

function tc.test_assert()
  assert_pass("assert(true) doesn't work!", function() assert(true) end)
  assert_pass("assert(12345) doesn't work!", function() assert(12345) end)
  assert_pass("assert(\"A string\") doesn't work!", function() assert("A string") end)
  assert_pass("assert( {} ) doesn't work!", function() assert( {} ) end)
  assert_error("assert_(false) doesn't fail!", function() assert(false) end)
  assert_error("assert_(nil) doesn't fail!", function() assert(nil) end)
end

function tc.test_assert_equal()
  
  assert_pass("assert_equal(\"A String\", \"A String\") doesn't work!", function()
    local a_string = assert_equal("A String", "A String")
    assert_true("A String" == a_string)
  end)
  
  assert_pass("assert_equal(\"A String\", \"A String\", \"A message\") doesn't work!", function()
    local a_string = assert_equal("A String", "A String", "A message")
    assert_true("A String" == a_string)
  end)
  
  assert_pass("assert_equal(12345, 12345) doesn't work!", function()
    local a_number = assert_equal(12345, 12345)
    assert_true(12345 == a_number)
  end)
  
  assert_pass("assert_equal(12345, 12345, \"A message\") doesn't work!", function()
    local a_number = assert_equal(12345, 12345, "A message")
    assert_true(12345 == a_number)
  end)
  
  assert_pass("assert_equal(nil, nil) doesn't work!", function()
    local a_nil = assert_equal(nil, nil)
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_equal(12345, 12345, \"A message\") doesn't work!", function()
    local a_nil = assert_equal(nil, nil, "A message")
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_equal(false, false) doesn't work!", function()
    local a_false = assert_equal(false, false)
    assert_true(false == a_false)
  end)
  
  assert_pass("assert_equal(false, false, \"A message\") doesn't work!", function()
    local a_false = assert_equal(false, false, "A message")
    assert_true(false == a_false)
  end)
  
  assert_pass("assert_equal(true, true) doesn't work!", function()
    local a_true = assert_equal(true, true)
    assert_true(true == a_true)
  end)
  
  assert_pass("assert_equal(true, true, \"A message\") doesn't work!", function()
    local a_true = assert_equal(true, true, "A message")
    assert_true(true == a_true)
  end)
  
  assert_error("assert_equal(\"A String\", \"Another String\") doesn't fail!", function()
    assert_equal("A String", "Another String")
  end)
  
  assert_error("assert_equal(\"A String\", \"Another String\", \"A message\") doesn't fail!", function()
    assert_equal("A String", "Another String", "A message")
  end)
  
  assert_error("assert_equal(123, 456) doesn't fail!", function()
    assert_equal(123, 456)
  end)
  
  assert_error("assert_equal(123, 456) \"A message\") doesn't fail!", function()
    assert_equal(123, 456, "A message")
  end)
  
  assert_error("assert_equal(true, false) doesn't fail!", function()
    assert_equal(true, false)
  end)
  
  assert_error("assert_equal(true, false) \"A message\") doesn't fail!", function()
    assert_equal(true, false, "A message")
  end)
  
  assert_error("assert_equal(true, nil) doesn't fail!", function()
    assert_equal(true, nil)
  end)
  
  assert_error("assert_equal(true, nil) \"A message\") doesn't fail!", function()
    assert_equal(true, nil, "A message")
  end)
  
  assert_error("assert_equal(false, true) doesn't fail!", function()
    assert_equal(false, true)
  end)
  
  assert_error("assert_equal(false, true, \"A message\") doesn't fail!", function()
    assert_equal(false, true, "A message")
  end)
  
  assert_error("assert_equal(false, nil) doesn't fail!", function()
    assert_equal(false, nil)
  end)
  
  assert_error("assert_equal(false, nil) \"A message\") doesn't fail!", function()
    assert_equal(false, nil, "A message")
  end)
  
  assert_error("assert_equal(nil, true) doesn't fail!", function()
    assert_equal(nil, true)
  end)
  
  assert_error("assert_equal(nil, true) \"A message\") doesn't fail!", function()
    assert_equal(nil, true, "A message")
  end)
  
  assert_error("assert_equal(nil, false) doesn't fail!", function()
    assert_equal(nil, false)
  end)
  
  assert_error("assert_equal(nil, false) \"A message\") doesn't fail!", function()
    assert_equal(nil, false, "A message")
  end)
  
end


function tc.test_assert_not_equal()
  
  assert_pass("assert_not_equal(\"A String\", \"Another String\") doesn't work!", function()
    local a_string = assert_not_equal("A String", "Another String")
    assert_true("Another String" == a_string)
  end)
  
  assert_pass("assert_not_equal(\"A String\", \"Another String\", \"A message\") doesn't work!", function()
    local a_string = assert_not_equal("A String", "Another String", "A message")
    assert_true("Another String" == a_string)
  end)
  
  assert_pass("assert_not_equal(123, 456) doesn't work!", function()
    local a_number = assert_not_equal(123, 456)
    assert_true(456 == a_number)
  end)
  
  assert_pass("assert_not_equal(123, 456, \"A message\") doesn't work!", function()
    local a_number = assert_not_equal(123, 456, "A message")
    assert_true(456 == a_number)
  end)
  
  assert_pass("assert_not_equal(true, false) doesn't work!", function()
    local a_false = assert_not_equal(true, false)
    assert_true(false == a_false)
  end)
  
  assert_pass("assert_not_equal(true, false) \"A message\") doesn't work!", function()
    local a_false = assert_not_equal(true, false, "A message")
    assert_true(false == a_false)
  end)
  
  assert_pass("assert_not_equal(true, nil) doesn't work!", function()
    local a_nil = assert_not_equal(true, nil)
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_not_equal(true, nil) \"A message\") doesn't work!", function()
    local a_nil = assert_not_equal(true, nil, "A message")
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_not_equal(false, true) doesn't work!", function()
    local a_true = assert_not_equal(false, true)
    assert_true(true == a_true)
  end)
  
  assert_pass("assert_not_equal(false, true, \"A message\") doesn't work!", function()
    local a_true = assert_not_equal(false, true, "A message")
    assert_true(true == a_true)
  end)
  
  assert_pass("assert_not_equal(false, nil) doesn't work!", function()
    local a_nil = assert_not_equal(false, nil)
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_not_equal(false, nil) \"A message\") doesn't work!", function()
    local a_nil = assert_not_equal(false, nil, "A message")
    assert_true(nil == a_nil)
  end)
  
  assert_pass("assert_not_equal(nil, true) doesn't work!", function()
    local a_true = assert_not_equal(nil, true)
    assert_true(true == a_true)
  end)
  
  assert_pass("assert_not_equal(nil, true) \"A message\") doesn't work!", function()
    local a_true = assert_not_equal(nil, true, "A message")
    assert_true(true == a_true)
  end)
  
  assert_pass("assert_not_equal(nil, false) doesn't work!", function()
    local a_false = assert_not_equal(nil, false)
    assert_true(false == a_false)
  end)
  
  assert_pass("assert_not_equal(nil, false) \"A message\") doesn't work!", function()
    local a_false = assert_not_equal(nil, false, "A message")
    assert_true(false == a_false)
  end)
  
  assert_error("assert_not_equal(\"A String\", \"A String\") doesn't work!", function()
    assert_not_equal("A String", "A String")
  end)
  
  assert_error("assert_not_equal(\"A String\", \"A String\", \"A message\") doesn't fail!", function()
    assert_not_equal("A String", "A String", "A message")
  end)
  
  assert_error("assert_not_equal(12345, 12345) doesn't fail!", function()
    assert_not_equal(12345, 12345)
  end)
  
  assert_error("assert_not_equal(12345, 12345, \"A message\") doesn't fail!", function()
    assert_not_equal(12345, 12345, "A message")
  end)
  
  assert_error("assert_not_equal(nil, nil) doesn't fail!", function()
    assert_not_equal(nil, nil)
  end)
  
  assert_error("assert_not_equal(nil, nil, \"A message\") doesn't fail!", function()
    assert_not_equal(nil, nil, "A message")
  end)
  
  assert_error("assert_not_equal(false, false) doesn't fail!", function()
    assert_not_equal(false, false)
  end)
  
  assert_error("assert_not_equal(false, false, \"A message\") doesn't fail!", function()
    assert_not_equal(false, false, "A message")
  end)
  
  assert_error("assert_not_equal(true, true) doesn't fail!", function()
    assert_not_equal(true, true)
  end)
  
  assert_error("assert_not_equal(true, true, \"A message\") doesn't fail!", function()
    assert_not_equal(true, true, "A message")
  end)
  
end




local a_number = 123
local a_string = "A string"
local a_table = { }
local a_function = function() end
local a_thread = coroutine.create(function() end)

  

------------------------------
-- Test type test functions --
------------------------------

typetest = TestCase("is_type() Tests")
  
function typetest.test_is_nil()
  assert_true( is_nil(nil) )
  assert_false( is_nil(true) )
  assert_false( is_nil(false) )
  assert_false( is_nil(a_number) )
  assert_false( is_nil(a_string) )
  assert_false( is_nil(a_table) )
  assert_false( is_nil(a_function) )
  assert_false( is_nil(a_thread) )
end

function typetest.test_is_boolean()
  assert_true( is_boolean(false) )
  assert_true( is_boolean(true) )
  assert_false( is_boolean(nil) )
  assert_false( is_boolean(a_number) )
  assert_false( is_boolean(a_string) )
  assert_false( is_boolean(a_table) )
  assert_false( is_boolean(a_function) )
  assert_false( is_boolean(a_thread) )
end

function typetest.test_is_number()
  assert_true( is_number(a_number) )
  assert_false( is_number(nil) )
  assert_false( is_number(true) )
  assert_false( is_number(false) )
  assert_false( is_number(a_string) )
  assert_false( is_number(a_table) )
  assert_false( is_number(a_function) )
  assert_false( is_number(a_thread) )
end

function typetest.test_is_string()
  assert_true( is_string(a_string) )
  assert_false( is_string(nil) )
  assert_false( is_string(true) )
  assert_false( is_string(false) )
  assert_false( is_string(a_number) )
  assert_false( is_string(a_table) )
  assert_false( is_string(a_function) )
  assert_false( is_string(a_thread) )
end

function typetest.test_is_table()
  assert_true( is_table(a_table) )
  assert_false( is_table(nil) )
  assert_false( is_table(true) )
  assert_false( is_table(false) )
  assert_false( is_table(a_number) )
  assert_false( is_table(a_string) )
  assert_false( is_table(a_function) )
  assert_false( is_table(a_thread) )
end

function typetest.test_is_function()
  assert_true( is_function(a_function) )
  assert_false( is_function(nil) )
  assert_false( is_function(true) )
  assert_false( is_function(false) )
  assert_false( is_function(a_number) )
  assert_false( is_function(a_string) )
  assert_false( is_function(a_table) )
  assert_false( is_function(a_thread) )
end

function typetest.test_is_thread()
  assert_true( is_thread(a_thread) )
  assert_false( is_thread(nil) )
  assert_false( is_thread(true) )
  assert_false( is_thread(false) )
  assert_false( is_thread(a_number) )
  assert_false( is_thread(a_string) )
  assert_false( is_thread(a_table) )
  assert_false( is_thread(a_function) )
end




-------------------------------------
-- Tests assert_not_type functions --
-------------------------------------

nottypeassert = TestCase("assert_not_type() Tests")

function nottypeassert.test_assert_not_nil()
  assert_not_nil( true )
  assert_not_nil( false )
  assert_not_nil( a_number )
  assert_not_nil( a_string )
  assert_not_nil( a_table )
  assert_not_nil( a_function )
  assert_not_nil( a_thread )
  
  assert_not_nil( true, "A message")
  assert_not_nil( false, "A message")
  assert_not_nil( a_number, "A message")
  assert_not_nil( a_string, "A message")
  assert_not_nil( a_table, "A message")
  assert_not_nil( a_function, "A message")
  assert_not_nil( a_thread, "A message")
  
  assert_error(function() assert_not_nil(nil) end)
  assert_error(function() assert_not_nil(nil, "A message") end)
end

function nottypeassert.test_assert_not_boolean()
  assert_not_boolean( nil )
  assert_not_boolean( a_number )
  assert_not_boolean( a_string )
  assert_not_boolean( a_table )
  assert_not_boolean( a_function )
  assert_not_boolean( a_thread )
  
  assert_not_boolean( nil, "A message")
  assert_not_boolean( a_number, "A message")
  assert_not_boolean( a_string, "A message")
  assert_not_boolean( a_table, "A message")
  assert_not_boolean( a_function, "A message")
  assert_not_boolean( a_thread, "A message")
  
  assert_error(function() assert_not_boolean(true) end)
  assert_error(function() assert_not_boolean(true, "A message") end)
  assert_error(function() assert_not_boolean(false) end)
  assert_error(function() assert_not_boolean(false, "A message") end)
end

function nottypeassert.test_assert_not_number()
  assert_not_number( nil )
  assert_not_number( true )
  assert_not_number( false )
  assert_not_number( a_string )
  assert_not_number( a_table )
  assert_not_number( a_function )
  assert_not_number( a_thread )
  
  assert_not_number( nil, "A message")
  assert_not_number( true, "A message")
  assert_not_number( false, "A message")
  assert_not_number( a_string, "A message")
  assert_not_number( a_table, "A message")
  assert_not_number( a_function, "A message")
  assert_not_number( a_thread, "A message")
  
  assert_error(function() assert_not_number(a_number) end)
  assert_error(function() assert_not_number(a_number, "A message") end)
end

function nottypeassert.test_assert_not_string()
  assert_not_string( nil )
  assert_not_string( true )
  assert_not_string( false )
  assert_not_string( a_number )
  assert_not_string( a_table )
  assert_not_string( a_function )
  assert_not_string( a_thread )
  
  assert_not_string( nil, "A message")
  assert_not_string( true, "A message")
  assert_not_string( false, "A message")
  assert_not_string( a_number, "A message")
  assert_not_string( a_table, "A message")
  assert_not_string( a_function, "A message")
  assert_not_string( a_thread, "A message")
  
  assert_error(function() assert_not_string(a_string) end)
  assert_error(function() assert_not_string(a_string, "A message") end)
end

function nottypeassert.test_assert_not_table()
  assert_not_table( nil )
  assert_not_table( true )
  assert_not_table( false )
  assert_not_table( a_number )
  assert_not_table( a_string )
  assert_not_table( a_function )
  assert_not_table( a_thread )
  
  assert_not_table( nil, "A message")
  assert_not_table( true, "A message")
  assert_not_table( false, "A message")
  assert_not_table( a_number, "A message")
  assert_not_table( a_string, "A message")
  assert_not_table( a_function, "A message")
  assert_not_table( a_thread, "A message")
  
  assert_error(function() assert_not_table(a_table) end)
  assert_error(function() assert_not_table(a_table, "A message") end)
end

function nottypeassert.test_assert_not_function()
  assert_not_function( nil )
  assert_not_function( true )
  assert_not_function( false )
  assert_not_function( a_number )
  assert_not_function( a_string )
  assert_not_function( a_table )
  assert_not_function( a_thread )
  
  assert_not_function( nil, "A message")
  assert_not_function( true, "A message")
  assert_not_function( false, "A message")
  assert_not_function( a_number, "A message")
  assert_not_function( a_string, "A message")
  assert_not_function( a_table, "A message")
  assert_not_function( a_thread, "A message")
  
  assert_error(function() assert_not_function(a_function) end)
  assert_error(function() assert_not_function(a_function, "A message") end)
end

function nottypeassert.test_assert_not_thread()
  assert_not_thread( nil )
  assert_not_thread( true )
  assert_not_thread( false )
  assert_not_thread( a_number )
  assert_not_thread( a_string )
  assert_not_thread( a_table )
  assert_not_thread( a_function )
  
  assert_not_thread( nil, "A message")
  assert_not_thread( true, "A message")
  assert_not_thread( false, "A message")
  assert_not_thread( a_number, "A message")
  assert_not_thread( a_string, "A message")
  assert_not_thread( a_table, "A message")
  assert_not_thread( a_function, "A message")
  
  assert_error(function() assert_not_thread(a_thread) end)
  assert_error(function() assert_not_thread(a_thread, "A message") end)
end


-------------------------------------
-- Tests assert_type functions --
-------------------------------------

typeassert = TestCase("assert_type() Tests")

function typeassert.test_assert_nil()
  assert_nil( nil )
  assert_nil( nil, "A message" )
  
  assert_error( function() assert_nil( true ) end)
  assert_error( function() assert_nil( false ) end)
  assert_error( function() assert_nil( a_number ) end)
  assert_error( function() assert_nil( a_string ) end)
  assert_error( function() assert_nil( a_table ) end)
  assert_error( function() assert_nil( a_function ) end)
  assert_error( function() assert_nil( a_thread ) end)
  
  assert_error( function() assert_nil( true, "A message" ) end)
  assert_error( function() assert_nil( false, "A message" ) end)
  assert_error( function() assert_nil( a_number, "A message" ) end)
  assert_error( function() assert_nil( a_string, "A message" ) end)
  assert_error( function() assert_nil( a_table, "A message" ) end)
  assert_error( function() assert_nil( a_function, "A message" ) end)
  assert_error( function() assert_nil( a_thread, "A message" ) end)
end

function typeassert.test_assert_boolean()
  assert_boolean( true )
  assert_boolean( false )
  assert_boolean( true, "A message" )
  assert_boolean( false, "A message" )
  
  assert_error( function() assert_boolean( nil ) end)
  assert_error( function() assert_boolean( a_number ) end)
  assert_error( function() assert_boolean( a_string ) end)
  assert_error( function() assert_boolean( a_table ) end)
  assert_error( function() assert_boolean( a_function ) end)
  assert_error( function() assert_boolean( a_thread ) end)
  
  assert_error( function() assert_boolean( nil, "A message" ) end)
  assert_error( function() assert_boolean( a_number, "A message" ) end)
  assert_error( function() assert_boolean( a_string, "A message" ) end)
  assert_error( function() assert_boolean( a_table, "A message" ) end)
  assert_error( function() assert_boolean( a_function, "A message" ) end)
  assert_error( function() assert_boolean( a_thread, "A message" ) end)
end

function typeassert.test_assert_number()
  assert_number( a_number )
  assert_number( a_number, "A message" )
  
  assert_error( function() assert_number( nil ) end)
  assert_error( function() assert_number( true ) end)
  assert_error( function() assert_number( false ) end)
  assert_error( function() assert_number( a_string ) end)
  assert_error( function() assert_number( a_table ) end)
  assert_error( function() assert_number( a_function ) end)
  assert_error( function() assert_number( a_thread ) end)
  
  assert_error( function() assert_number( nil, "A message" ) end)
  assert_error( function() assert_number( true, "A message" ) end)
  assert_error( function() assert_number( false, "A message" ) end)
  assert_error( function() assert_number( a_string, "A message" ) end)
  assert_error( function() assert_number( a_table, "A message" ) end)
  assert_error( function() assert_number( a_function, "A message" ) end)
  assert_error( function() assert_number( a_thread, "A message" ) end)
end

function typeassert.test_assert_string()
  assert_string( a_string ) 
  assert_string( a_string, "A message" )
  
  assert_error( function() assert_string( nil ) end)
  assert_error( function() assert_string( true ) end)
  assert_error( function() assert_string( false ) end)
  assert_error( function() assert_string( a_number ) end)
  assert_error( function() assert_string( a_table ) end)
  assert_error( function() assert_string( a_function ) end)
  assert_error( function() assert_string( a_thread ) end)
  
  assert_error( function() assert_string( nil, "A message" ) end)
  assert_error( function() assert_string( true, "A message" ) end)
  assert_error( function() assert_string( false, "A message" ) end)
  assert_error( function() assert_string( a_number, "A message" ) end)
  assert_error( function() assert_string( a_table, "A message" ) end)
  assert_error( function() assert_string( a_function, "A message" ) end)
  assert_error( function() assert_string( a_thread, "A message" ) end)
end

function typeassert.test_assert_table()
  assert_table( a_table )
  assert_table( a_table, "A message" )
  
  assert_error( function() assert_table( nil ) end)
  assert_error( function() assert_table( true ) end)
  assert_error( function() assert_table( false ) end)
  assert_error( function() assert_table( a_number ) end)
  assert_error( function() assert_table( a_string ) end)
  assert_error( function() assert_table( a_function ) end)
  assert_error( function() assert_table( a_thread ) end)
  
  assert_error( function() assert_table( nil, "A message" ) end)
  assert_error( function() assert_table( true, "A message" ) end)
  assert_error( function() assert_table( false, "A message" ) end)
  assert_error( function() assert_table( a_number, "A message" ) end)
  assert_error( function() assert_table( a_string, "A message" ) end)
  assert_error( function() assert_table( a_function, "A message" ) end)
  assert_error( function() assert_table( a_thread, "A message" ) end)
end

function typeassert.test_assert_function()
  assert_function( a_function )
  assert_function( a_function, "A message" )
  
  assert_error( function() assert_function( nil ) end)
  assert_error( function() assert_function( true ) end)
  assert_error( function() assert_function( false ) end)
  assert_error( function() assert_function( a_number ) end)
  assert_error( function() assert_function( a_string ) end)
  assert_error( function() assert_function( a_table ) end)
  assert_error( function() assert_function( a_thread ) end)
  
  assert_error( function() assert_function( nil, "A message" ) end)
  assert_error( function() assert_function( true, "A message" ) end)
  assert_error( function() assert_function( false, "A message" ) end)
  assert_error( function() assert_function( a_number, "A message" ) end)
  assert_error( function() assert_function( a_string, "A message" ) end)
  assert_error( function() assert_function( a_table, "A message" ) end)
  assert_error( function() assert_function( a_thread, "A message" ) end)
end

function typeassert.test_assert_thread()
  assert_thread( a_thread )
  assert_thread( a_thread, "A message" )
  
  assert_error( function() assert_thread( nil ) end)
  assert_error( function() assert_thread( true ) end)
  assert_error( function() assert_thread( false ) end)
  assert_error( function() assert_thread( a_number ) end)
  assert_error( function() assert_thread( a_string ) end)
  assert_error( function() assert_thread( a_table ) end)
  assert_error( function() assert_thread( a_function ) end)
  
  assert_error( function() assert_thread( nil, "A message" ) end)
  assert_error( function() assert_thread( true, "A message" ) end)
  assert_error( function() assert_thread( false, "A message" ) end)
  assert_error( function() assert_thread( a_number, "A message" ) end)
  assert_error( function() assert_thread( a_string, "A message" ) end)
  assert_error( function() assert_thread( a_table, "A message" ) end)
  assert_error( function() assert_thread( a_function, "A message" ) end)
end


-------------------------------------------------
-- Tests assert_match() and assert_not_match() --
-------------------------------------------------

match = TestCase("match Tests")

function match.test_assert_match()
  
  assert_pass("assert_match(\"^Hello\", \"Hello World\") doesn't work!", function()
    local a_string = assert_match("^Hello", "Hello World")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_match(\"^Hello\", \"Hello World\", \"A Message\") doesn't work!", function()
    local a_string = assert_match("^Hello", "Hello World", "A message")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_match(\"World$\", \"Hello World\") doesn't work!", function()
    local a_string = assert_match("World$", "Hello World")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_match(\"World$\", \"Hello World\", \"A Message\") doesn't work!", function()
    local a_string = assert_match("World$", "Hello World", "A message")
    assert_equal("Hello World", a_string)
  end)
  
  assert_error("assert_match(\"Hello$\", \"Hello World\") doesn't fail!", function()
    assert_match("Hello$", "Hello World")
  end)
  
  assert_error("assert_match(\"Hello$\", \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_match("Hello$", "Hello World", "A message")
  end)
  
  assert_error("assert_match(\"^World\", \"Hello World\") doesn't fail!", function()
    assert_match("^World", "Hello World")
  end)
  
  assert_error("assert_match(\"^World\", \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_match("^World", "Hello World", "A message")
  end)
  
  assert_error("assert_match(nil, \"Hello World\") doesn't fail!", function()
    assert_match(nil, "Hello World")
  end)
  
  assert_error("assert_match(nil, \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_match(nil, "Hello World", "A message")
  end)
  
  assert_error("assert_match(\"^World\", nil) doesn't fail!", function()
    assert_match("^World", nil)
  end)
  
  assert_error("assert_match(\"^World\", nil, \"A Message\") doesn't fail!", function()
    assert_match("^World", nil, "A message")
  end)
  
end

function match.test_assert_not_match()
  assert_pass("assert_not_match(\"Hello$\", \"Hello World\") doesn't work!", function()
    local a_string = assert_not_match("Hello$", "Hello World")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_not_match(\"Hello$\", \"Hello World\", \"A Message\") doesn't work!", function()
    local a_string = assert_not_match("Hello$", "Hello World", "A message")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_not_match(\"^World\", \"Hello World\") doesn't work!", function()
    local a_string = assert_not_match("^World", "Hello World")
    assert_equal("Hello World", a_string)
  end)
  
  assert_pass("assert_not_match(\"^World\", \"Hello World\", \"A Message\") doesn't work!", function()
    local a_string = assert_not_match("^World", "Hello World", "A message")
    assert_equal("Hello World", a_string)
  end)
  
  assert_error("assert_not_match(\"^Hello\", \"Hello World\") doesn't fail!", function()
    assert_not_match("^Hello", "Hello World")
  end)
  
  assert_error("assert_not_match(\"^Hello\", \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_not_match("^Hello", "Hello World", "A message")
  end)
  
  assert_error("assert_not_match(\"World$\", \"Hello World\") doesn't fail!", function()
    assert_not_match("World$", "Hello World")
  end)
  
  assert_error("assert_not_match(\"World$\", \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_not_match("World$", "Hello World", "A message")
  end)
  
  assert_error("assert_not_match(nil, \"Hello World\") doesn't fail!", function()
    assert_not_match(nil, "Hello World")
  end)
  
  assert_error("assert_not_match(nil, \"Hello World\", \"A Message\") doesn't fail!", function()
    assert_not_match(nil, "Hello World", "A message")
  end)
  
  assert_error("assert_not_match(\"^World\", nil) doesn't fail!", function()
    assert_not_match("^World", nil)
  end)
  
  assert_error("assert_not_match(\"^World\", nil, \"A Message\") doesn't fail!", function()
    assert_not_match("^World", nil, "A message")
  end)
  
end




-------------------------------------
-- Tests to check the import stuff --
-------------------------------------

tc = TestCase("Import Modul")

function tc.test_privfenv()
  assert_table(_G)
  assert_table(_G._G)
  assert_equal(_G, _G._G)
  assert_nil(some_global_variable)
  some_global_variable = 123
  assert_equal(123, some_global_variable)
  
  lunit.setprivfenv()
  
  assert_table(_G)
  assert_table(_G._G)
  assert_equal(_G, _G._G)
  assert_equal(123, some_global_variable)
end

function tc.test_import()
  assert_pass(function()
    lunit.import("all")
    lunit.import("tests")
    lunit.import("checks")
    lunit.import("asserts")
    lunit.import("assertions")
  end)
  
  assert_error(function() lunit.import("not_exists") end)
  assert_error(function() lunit.import("run") end)
  assert_error(function() lunit.import("import") end)
end


do
  local executed_name = false
  local executed_anon = false
  local executed_multi_name = 0
  local executed_multi_anon = 0
  
  lunit.wrap("Wrapped function", function() executed_name = true end)
  lunit.wrap(function() executed_anon = true end)
  
  lunit.wrap("Wrapped multiple",
    function() executed_multi_name = (executed_multi_name + 1) * 2 end,
    function() executed_multi_name = (executed_multi_name + 3) * 4 end )
  
  lunit.wrap(
    function() executed_multi_anon = (executed_multi_anon + 1) * 2 end,
    function() executed_multi_anon = (executed_multi_anon + 3) * 4 end )
  
  local check_executed = TestCase("Check wrapped function execution")
  function check_executed.test()
    assert_true(executed_name)
    assert_true(executed_anon)
    assert_equal(20, executed_multi_name)
    assert_equal(20, executed_multi_anon)
  end
end




lunit.run()



