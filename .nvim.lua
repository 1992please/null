-- Build configuration
vim.opt.makeprg = "cmake --build build"
-- vim.opt.errorformat = "%f:%l:%c: %m,%f:%l: %m"
local executable_name = "null_engine"
local build_dir = "build/bin"
local executable_path = vim.fn.getcwd() .. "/" .. build_dir .. "/" .. executable_name


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
vim.keymap.set('n', '<F7>', function()
  if vim.fn.filereadable(executable_path) == 1 then
    vim.cmd('terminal cd ' .. build_dir .. '&& ./' .. executable_name)
  else
    print("Executable not found: " .. executable_path .. ". Run build first (F5)")
  end
end, { desc = 'Run Executable' })

vim.keymap.set('n', '<leader>ll', function()
  local log_path = build_dir .. "/logs/engine.log"
  if vim.fn.filereadable(log_path) == 1 then
    vim.cmd('split ' .. log_path)
    vim.cmd('setlocal autoread')
  else
    print("Log file not found: " .. log_path)
  end
end, { desc = 'View Engine Log' })

local dap = require('dap')
dap.configurations.cpp = {
  {
    name = "Launch file",
    type = "codelldb",
    request = "launch",
    program = executable_path,
    cwd = "${workspaceFolder}/" .. build_dir,
    stopOnEntry = false,
  },
}

print("Lua Config Loaded successfully!!!!")
