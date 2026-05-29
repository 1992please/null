-- Build configuration
vim.opt.makeprg = "cmake --build build"
-- vim.opt.errorformat = "%f:%l:%c: %m,%f:%l: %m"
local executable_name = "null_engine"
local build_dir = "build/bin"
local executable_path = vim.fn.getcwd() .. "/" .. build_dir .. "/" .. executable_name
local build_types = { "Debug", "RelWithDebInfo", "Release" }
local current_build_index = 1
local term_buf = nil

-- Build type switching
vim.keymap.set('n', '<F4>', function()
  current_build_index = current_build_index % #build_types + 1
  print("Build type: " .. build_types[current_build_index])
end, { desc = 'Toggle Build Type' })

-- Cmake build files
vim.keymap.set('n', '<F5>', function()
  local build_type = build_types[current_build_index]
  vim.cmd('!rm -rf build')
  vim.cmd('!cmake -B build -DCMAKE_BUILD_TYPE=' .. build_type)
  print("Clean build: " .. build_type)
end, { desc = 'Clean Build' })

-- Build project
vim.keymap.set('n', '<F6>', function()
  vim.cmd('make')         -- Run cmake --build build
  vim.cmd('cwindow')      -- Open quickfix if errors
end, { desc = 'Build Project' })

-- Run Executable
vim.keymap.set('n', '<F7>', function()
  if vim.fn.filereadable(executable_path) == 1 then
    -- Close old terminal if it exists
    if term_buf and vim.api.nvim_buf_is_valid(term_buf) then
      vim.api.nvim_buf_delete(term_buf, { force = true })
    end
    vim.cmd('split | terminal cd ' .. build_dir .. '&& ./' .. executable_name)
    term_buf = vim.api.nvim_get_current_buf()
  else
    print("Executable not found: " .. executable_path .. ". Run build first (F5)")
  end
end, { desc = 'Run Executable' })

-- Show logs
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
