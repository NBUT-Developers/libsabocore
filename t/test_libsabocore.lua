
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
        name = "test_rundone",
        path = getenv("PWD") .. "/test_rundone/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_rundone/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_rundone/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 0,
        min_time_used = 0,
        max_time_used = 100,
        min_memory_used = 0,
        max_memory_used = 500,
    },
    {
        name = "test_mc_1",
        path = getenv("PWD") .. "/test_mc/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_mc/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_mc/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 10,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_mc_2",
        path = getenv("PWD") .. "/test_mc_2/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_mc_2/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_mc_2/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 10,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_mc_3",
        path = getenv("PWD") .. "/test_mc_3/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_mc_3/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_mc_3/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 10,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_mc_4",
        path = getenv("PWD") .. "/test_mc_4/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_mc_4/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_mc_4/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 10,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_tle",
        path = getenv("PWD") .. "/test_tle/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_tle/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_tle/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 2,
        min_time_used = 1000,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_re_fpe",
        path = getenv("PWD") .. "/test_re_fpe/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_re_fpe/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_re_fpe/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 6,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_re_so",
        path = getenv("PWD") .. "/test_re_so/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_re_so/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_re_so/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 5,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_re",
        path = getenv("PWD") .. "/test_re/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_re/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_re/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 5,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,
    },
    {
        name = "test_mle",
        path = getenv("PWD") .. "/test_mle/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_mle/Code.cpp",
        code_exec = nil,

        use_sandbox = true,
        classpath = nil,

        compile = "g++ -O2 -Wmaybe-uninitialized -o ./test_mle/Main ",

        err = "cdata<const char *>: NULL",
        judge_flag = 3,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 65536,
        max_memory_used = 65536 * 10,
    },
    {
        name = "test_java_rundone",
        path = getenv("PWD") .. "/test_java_rundone/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_java_rundone/Main.java",
        code_exec = nil,

        use_sandbox = false,
        classpath = getenv("PWD") .. "/test_java_rundone/",

        compile = "javac ",

        err = "cdata<const char *>: NULL",
        judge_flag = 0,
        min_time_used = 0,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,

        run = true,
    },
    {
        name = "test_java_tle",
        path = getenv("PWD") .. "/test_java_tle/",

        data_in = "data.in",
        data_out = "data.out",
        user_out = "user.out",

        time_limts = 1000,
        memory_limits = 65536,

        code_file = "./test_java_tle/Main.java",
        code_exec = nil,

        use_sandbox = false,
        classpath = getenv("PWD") .. "/test_java_tle/",

        compile = "javac ",

        err = "cdata<const char *>: NULL",
        judge_flag = 2,
        min_time_used = 1000,
        max_time_used = 1000,
        min_memory_used = 0,
        max_memory_used = 65536,

        run = true,
    },
}


local function test(bundle)

    if not bundle.run then
        return
    end


    local in_fd         = getfd(open(bundle.path .. bundle.data_in, "r"))
    local out_fd        = getfd(open(bundle.path .. bundle.data_out, "r"))
    local user_fd       = getfd(open(bundle.path .. bundle.user_out, "w"))
    local time_limits   = bundle.time_limts
    local memory_limits = bundle.memory_limits
    local use_sandbox   = bundle.use_sandbox and 1 or 0
    local code_file     = bundle.code_file
    local code_exec     = bundle.code_exec
    local classpath     = bundle.classpath
    local rv            = compile(bundle.compile .. code_file)

    local tab = {
        code_bin_file    = "Main",
        executor         = bundle.path .. "Main",

        data_in_fd       = in_fd,
        data_out_fd      = out_fd,
        user_out_fd      = user_fd,

        allow_so_file    = nil,
        allow_so_file_n  = 0,

        allow_sys_call   = nil,
        allow_sys_call_n = 0,

        time_limits      = time_limits,
        memory_limits    = memory_limits,

        use_sandbox      = use_sandbox,
        classpath        = classpath
    }
    if tab.use_sandbox == 0 then
        tab.executor = "/usr/local/jdk/bin/java"
    end

    local info = {
        judge_flag  = -1, 
        time_used   = -1,
        memory_used = -1
    }

    ctx = ffi.new("sabo_ctx_t", tab)

    info = ffi.new("sabo_res_t", info)

    local err = sabo_core.sabo_core_run(ctx, info)
    

    clean(bundle.path)
    assert(tostring(err) == bundle.err)
    assert(info.judge_flag == bundle.judge_flag,
    "judge flag error, expected " .. bundle.judge_flag .. " but get " .. info.judge_flag)

    assert(info.time_used <= bundle.max_time_used, "time_used too large")
    assert(info.time_used >= bundle.min_time_used, "time_used too small")

    assert(info.memory_used <= bundle.max_memory_used, "memory_used too large")
    assert(info.memory_used >= bundle.min_memory_used, "memory_used too small")

    print(bundle.name .. " ... " .. "PASSED", " judge flag " .. info.judge_flag .. " time_used " .. info.time_used .. " memory_used " .. info.memory_used)
end


local function main()

    for _, bundle in ipairs(bundles) do
        test(bundle)
    end
end


main()
