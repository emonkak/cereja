
require "lunit"

testcase = lunit.TestCase("Simple Testcase")

function testcase.test_success()
  lunit.assert_true( true, "This test never fails.")
end

function testcase.test_failure()
  lunit.assert_true( false, "This test always fails!")
end

lunit.run()

