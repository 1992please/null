-- Build configuration
vim.opt.makeprg = "cmake --build build"
-- vim.opt.errorformat = "%f:%l:%c: %m,%f:%l: %m"
-- NOTE: we need to include a way to build and run release and shipping for testing
-- Clean build
vim.keymap.set('n', '<F5>', function()
  vim.cmd('!rm -rf build')
  vim.cmd('!cmake -B build -DCMAKE_BUILD_TYPE=Debug')
end, { desc = 'Clean Build' })

-- Build project
vim.keymap.set('n', '<F6>', function()
  vim.cmd('make')         -- Run cmake --build build
  vim.cmd('cwindow')      -- Open quickfix if errors
end, { desc = 'Build Project' })

-- Run executable (F6)
-- Adjust the path to match your executable location
local executable_path = "./build/bin/null_engine"  -- Change this!
vim.keymap.set('n', '<F7>', function()
  -- Check if executable exists
  if vim.fn.filereadable(executable_path) == 1 then
    vim.cmd('!' .. executable_path)
  else
    print("Executable not found: " .. executable_path .. ". Run build first (F5)")
  end
end, { desc = 'Run Executable' })


local dap = require('dap')
dap.configurations.cpp = {
  {
    name = "Launch file",
    type = "codelldb",
    request = "launch",
    program = executable_path,
    cwd = vim.fn.getcwd(),
    stopOnEntry = false,
  },
}

print("Project config loaded: makeprg=" .. vim.opt.makeprg:get())
