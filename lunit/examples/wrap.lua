
require "lunit"


lunit.wrap(function()
  -- Anonymous wrapped function
  lunit.assert_fail("I will always fail")
end)


lunit.wrap("Named wrapped Test", function()
  lunit.assert_string("I should always succeed!")
end)


lunit.run()

