
require "lunit"

lunit.import "all"

testcase = TestCase("Import")

function testcase.test_import()
  assert_true(true, "Should never fail.") 
end

lunit.run()
