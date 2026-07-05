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

local function update_preset()
  local preset = presets[current_preset_index]
  vim.opt.makeprg = "cmake --build build/" .. preset
  
  local dap_ok, dap = pcall(require, 'dap')
  if dap_ok then
    dap.configurations.cpp = {
      {
        name = "Launch file",
        type = "codelldb",
        request = "launch",
        program = get_executable_path(preset),
        cwd = "${workspaceFolder}/" .. get_build_dir(preset),
        stopOnEntry = false,
      },
    }
  end

  print("Build type: " .. preset)
end

-- Initialize the default preset (debug) silently on load
update_preset()

-- Build type switching
vim.keymap.set('n', '<F4>', function()
  current_preset_index = current_preset_index % #presets + 1
  update_preset()
end, { desc = 'Toggle Build Type' })

-- Configure project
vim.keymap.set('n', '<F5>', function()
  local preset = presets[current_preset_index]
  vim.cmd('!cmake --preset ' .. preset)
  print("Configured: " .. preset)
end, { desc = 'Configure Project' })

-- Build project
vim.keymap.set('n', '<F6>', function()
  vim.cmd('make')         -- Run cmake --build build/<preset>
  vim.cmd('cwindow')      -- Open quickfix if errors
end, { desc = 'Build Project' })

-- Run Executable
vim.keymap.set('n', '<F7>', function()
  local preset = presets[current_preset_index]
  local bdir = get_build_dir(preset)
  local exe_path = get_executable_path(preset)
  if vim.fn.filereadable(exe_path) == 1 then
    -- Close old terminal if it exists
    if term_buf and vim.api.nvim_buf_is_valid(term_buf) then
      vim.api.nvim_buf_delete(term_buf, { force = true })
    end
    vim.cmd('split | terminal cd ' .. bdir .. ' && ./' .. executable_name)
    term_buf = vim.api.nvim_get_current_buf()
  else
    print("Executable not found: " .. exe_path .. ". Run build first (F5)")
  end
end, { desc = 'Run Executable' })

-- Show logs
vim.keymap.set('n', '<leader>ll', function()
  local preset = presets[current_preset_index]
  local log_path = get_build_dir(preset) .. "/logs/engine.log"
  if vim.fn.filereadable(log_path) == 1 then
    vim.cmd('split ' .. log_path)
    vim.cmd('setlocal autoread')
  else
    print("Log file not found: " .. log_path)
  end
end, { desc = 'View Engine Log' })

print("Lua Config Loaded successfully!!!!")
