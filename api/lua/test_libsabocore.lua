
local luacore = require "luacore"
local ffi = require "ffi"

ffi.cdef[[
    const char *sabo_core_run(sabo_ctx_t *ctx, sabo_res_t *info);
]]

local sabo_core = luacore.sabo_core

local res, err = sabo_core(1, 2, 3, 1000, 65536, 1, 0, "./", "./lauxlib.h", nil, nil)
print(err)
