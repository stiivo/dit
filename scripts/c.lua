
local cscope = require("cscope")

function on_ctrl(key)
   if key == "D" then
      cscope.goto_definition()
   end
end

