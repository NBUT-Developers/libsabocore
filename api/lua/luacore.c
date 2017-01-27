
/* Copyright 2016-2017 Alex(zchao1995@gmail.com) */

#include <lua.h>
#include <lauxlib.h>
#include "sabo_core.h"


/* 
 * The luaC stack item order
 *
 * -----------------------------  top
 * | array(NULL)allow_sys_call |
 * -----------------------------
 * | int:allow_sys_call_n      |
 * -----------------------------
 * | array(NULL):allow_so_file |
 * -----------------------------
 * | int:allow_so_file_n       |
 * -----------------------------
 * | string(NULL):classpath    |
 * -----------------------------
 * | string(NULL):spj_code_bin |
 * -----------------------------
 * | string(NULL):spj_executor |
 * -----------------------------
 * | string:code_bin_file      |
 * -----------------------------
 * | string(NULL):executor     |
 * -----------------------------
 * | int:spj_mode              |
 * -----------------------------
 * | int:use_sandbox           |
 * -----------------------------
 * | int:memory_limits         |
 * -----------------------------
 * | int:time_limits           |
 * -----------------------------
 * | int:user_out_fd           |
 * -----------------------------
 * | int:data_out_fd           |
 * -----------------------------
 * | int:data_in_fd            |
 * ----------------------------- bottom
 */
static int
sabo_core(lua_State *L)
{
    sabo_ctx_t ctx;
    sabo_res_t info;
    const char *err;
    
    ctx.data_in_fd    = luaL_checkint(L, 1);
    ctx.data_out_fd   = luaL_checkint(L, 2);
    ctx.user_out_fd   = luaL_checkint(L, 3);
    ctx.time_limits   = luaL_checkint(L, 4);
    ctx.memory_limits = luaL_checkint(L, 5);
    ctx.use_sandbox   = luaL_checkint(L, 6);
    ctx.spj_mode      = luaL_checkint(L, 7);
    
    if (ctx.use_sandbox) {
        ctx.executor = luaL_checklstring(L, 8, NULL);
    } else {
        ctx.executor = lua_tolstring(L, 8, NULL);
    }

    ctx.code_bin_file = luaL_checklstring(L, 9, NULL);

    if (ctx.spj_mode) {
        ctx.spj_executor = luaL_checklstring(L, 10, NULL);
        ctx.spj_code_bin = luaL_checklstring(L, 11, NULL);

    } else {

        ctx.spj_executor = lua_tolstring(L, 10, NULL);
        ctx.spj_code_bin = lua_tolstring(L, 11, NULL);
    }

    /* ctx.classpath = lua_tolstring(L, 12, NULL); */
    
    /* ctx.allow_so_file_n = luaL_checkint(L, 13); */

    /* ctx.allow_sys_call_n = luaL_checkint(L, 15); */


    err = sabo_core_run(&ctx, &info);
    if (err == NULL) {
        err = "";
    }

    lua_pushstring(L, err);
    lua_pushnumber(L, info.judge_flag);
    lua_pushnumber(L, info.time_used);
    lua_pushnumber(L, info.memory_used);

    return 4;
}


static const struct luaL_Reg sabo_core_lib[] = {

    { "sabo_core", sabo_core },
    { NULL, NULL}
};

int
luaopen_luacore(lua_State *L)
{
#if LUA_VERSION_NUMBER < 502
    luaL_register(L, "sabo_core", sabo_core_lib);
#else
    lua_newlib(L, sabo_core_lib);
#endif
    
    return 1;
}
