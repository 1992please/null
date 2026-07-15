-- Build configuration
local executable_name = "null_engine"
local presets = { "debug", "development", "shipping" }
local current_preset_index = 1
local term_buf = nil

local function get_build_dir(preset)
  return "build/" .. preset .. "/bin"
end

local function get_executable_path(preset)
  return vim.fn.getcwd() .. "/" .. get_build_dir(preset) .. "/" .. executable_name
end

local function get_compile_commands_path(preset)
  return "build/" .. preset .. "/compile_commands.json"
end

local function update_preset()
  local preset = presets[current_preset_index]

  -- set the build command
  vim.opt.makeprg = "cmake --build --preset " .. preset

  -- set the debug command
  local dap_ok, dap = pcall(require, 'dap')
  if dap_ok then
    dap.configurations.cpp = {
      {
        name = "Launch file",
        type = "codelldb",
        request = "launch",
        program = get_executable_path(preset),
        cwd = "${workspaceFolder}",
        stopOnEntry = false,
      },
    }
  end

end

update_preset()

-- Build type switching
vim.keymap.set('n', '<F4>', function()
  current_preset_index = current_preset_index % #presets + 1
  update_preset()
  print("Build type: " .. presets[current_preset_index])
end, { desc = 'Toggle Build Type' })

-- Configure project
vim.keymap.set('n', '<F5>', function()
  local preset = presets[current_preset_index]
  -- configure project
  vim.cmd('!cmake --preset ' .. preset)
  -- Copy compile_commands.json of the active preset to build/
  vim.cmd('!cp ' .. get_compile_commands_path(preset) .. ' build/compile_commands.json')
  -- Refresh LSP (restart LSP clients to reload compile_commands.json)
  if #vim.lsp.get_clients() > 0 then
    vim.cmd('lsp restart')
  end
  print("Configured: " .. preset)
end, { desc = 'Configure Project' })

-- Build project
vim.keymap.set('n', '<F6>', function()
  vim.cmd('make')         -- Run cmake --build --preset <preset>
  vim.cmd('cwindow')      -- Open quickfix if errors
end, { desc = 'Build Project' })

-- Run Executable
vim.keymap.set('n', '<F7>', function()
  local preset = presets[current_preset_index]
  local exe_path = get_executable_path(preset)
  if vim.fn.filereadable(exe_path) == 1 then
    -- Close old terminal if it exists
    if term_buf and vim.api.nvim_buf_is_valid(term_buf) then
      vim.api.nvim_buf_delete(term_buf, { force = true })
    end
    vim.cmd('split | terminal' .. exe_path)
    term_buf = vim.api.nvim_get_current_buf()
  else
    print("Executable not found: " .. exe_path .. ". Run build first (F5)")
  end
end, { desc = 'Run Executable' })

-- Show logs
vim.keymap.set('n', '<leader>gl', function()
  local preset = presets[current_preset_index]
  local log_path = get_build_dir(preset) .. "/logs/engine.log"
  if vim.fn.filereadable(log_path) == 1 then
    vim.cmd('split ' .. log_path)
    vim.cmd('setlocal autoread')
  else
    print("Log file not found: " .. log_path)
  end
end, { desc = 'View Engine Log' })

vim.keymap.set('n', '<leader>gd', function()
  print("Launching git directory diff...")
  vim.system({ "git", "difftool", "-d" }, { detach = true })
end, { desc = 'View Diff Tool' })

print("Lua Config Loaded successfully!!!!")
