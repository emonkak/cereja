
require "lunit"

lunit.setprivfenv()
lunit.import "assertions"

testcase = lunit.TestCase("Complicated")


function testcase:setup()
  self.foobar = "Hello World"
end

function testcase:teardown()
  self.foobar = nil
end


function testcase:test1()
  assert_equal("Hello World", self.foobar)
  self.foobar = string.sub(self.foobar, 1, 5)
  assert_equal("Hello", self.foobar)
end

function testcase:test2()
  assert_equal("Hello World", self.foobar)
  self.foobar = string.sub(self.foobar, -5)
  assert_equal("World", self.foobar)
end


lunit.run()

