
-- Copyright 2016-2017 Alex(zchao1995@gmail.com)


local ffi = require "ffi"

local execute = os.execute
local open    = io.open
local remove  = os.remove
local getenv  = os.getenv
local C       = ffi.C

local sabo_core = ffi.load("libsabocore")



ffi.cdef [[

typedef struct {

    const char *code_bin_file;
    const char *executor;
    
    const char *spj_code_bin;
    const char *spj_executor;

    int spj_mode;

    int data_in_fd;
    int data_out_fd;
    int user_out_fd;

    const char **allow_so_file;
    int allow_so_file_n;

    int *allow_sys_call;
    int allow_sys_call_n;

    int time_limits;
    int memory_limits;

    int use_sandbox; /* FIXME, just for java now */

    const char *classpath; /* FIXME, for java class path */

} sabo_ctx_t;

typedef struct {
    int time_used;
    int memory_used;
    int judge_flag;

} sabo_res_t;

int fileno(struct FILE* stream);
const char *sabo_core_run(sabo_ctx_t *ctx, sabo_res_t *info);
int printf(const char *fmt, ...);


]]


local function getfd(FILE) return C.fileno(FILE) end


local function compile(cmd)

    if not cmd then
        return nil, "nil compile cmd"
    end

    local rv = execute(cmd)

    return rv
end


local function clean(path)

    if path:sub(-1) ~= '/' then
        path = path .. '/'
    end

    remove(path .. "Main")
    remove(path .. "Main.class")
    remove(path .. "user.out")
end



local bundles = {
    {
        func = test_ac,
        path = getenv("PWD") .. "/test_ac/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_ac/Code.cpp",
        code_exec = nil,

        spj_mode = false,
        spj_code = nil,
        spj_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wall -o ./test_ac/Main "
    }
}


local function test(bundle)

    local in_fd         = getfd(open(bundle.path .. bundle.data_in, "r"))
    local out_fd        = getfd(open(bundle.path .. bundle.data_out, "r"))
    local user_fd       = getfd(open(bundle.path .. bundle.user_out, "w"))
    local time_limits   = bundle.time_limts
    local memory_limits = bundle.memory_limits
    local use_sandbox   = bundle.use_sandbox and 1 or 0
    local spj_mode      = bundle.spj_mode and 1 or 0
    local code_file     = bundle.code_file
    local code_exec     = bundle.code_exec
    local spj_code      = bundle.spj_code
    local spj_exec      = bundle.spj_exec
    local classpath     = bundle.classpath
    local rv            = compile(bundle.compile .. code_file)

    local tab = {
        code_bin_file    = "Main",
        executor         = bundle.path .. "Main",

        spj_code_bi      = nil,
        spj_executor     = nil,

        spj_mode         = 0,

        data_in_fd       = in_fd,
        data_out_fd      = out_fd,
        user_out_fd      = user_fd,

        allow_so_file    = nil,
        allow_so_file_n  = 0,

        allow_sys_call   = nil,
        allow_sys_call_n = 0,

        time_limits      = time_limits,
        memory_limits    = memory_limits,

        use_sandbox      = 1,
        classpath        = nil
    }

    local info = {
        judge_flag  = -1, 
        time_used   = -1,
        memory_used = -1
    }

    ctx = ffi.new("sabo_ctx_t", tab)

    info = ffi.new("sabo_res_t", info)

    local err = sabo_core.sabo_core_run(ctx, info)

    print(err)
    print(info.judge_flag, info.time_used, info.memory_used)

    -- clean(bundle.path)
end


local function main()
    for _, bundle in ipairs(bundles) do
        test(bundle)
    end
end


main()
