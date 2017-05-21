
/* Copyright Alex(zchao1995@gmail.com) */

#include <lua.h>
#include <string.h>
#include <lauxlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sabo_core.h"


static void create_sabo_ctx(lua_State *L, sabo_ctx_t *ctx);
static int run(lua_State *L);


static void
create_sabo_ctx(lua_State *L, sabo_ctx_t *ctx)
{
    const char *path;

    lua_getfield(L, -1, "code_bin_file"); /* code_bin_file, table */
    ctx->code_bin_file = lua_tostring(L, -1);
    if (ctx->code_bin_file == NULL) {
        luaL_error(L, "code_bin_file can not be NULL");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "language"); /* language, table */
    ctx->language = lua_tonumber(L, -1);
    if (ctx->language != SABO_JAVA && ctx->language != SABO_C_CPP) {
        luaL_error(L, "bad language");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "executor"); /* executor, table */
    ctx->executor = lua_tostring(L, -1);
    if (ctx->language == SABO_JAVA && ctx->executor == NULL) {
        luaL_error(L, "java path absent");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "classpath"); /* classpath, table */
    ctx->classpath = lua_tostring(L, -1);
    if (ctx->language == SABO_JAVA && ctx->classpath == NULL) {
        luaL_error(L, "java classpath absent");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "time_limits"); /* time_limits, table */
    ctx->time_limits = lua_tonumber(L, -1);
    if (ctx->time_limits <= 0) {
        luaL_error(L, "bad time_limits");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "memory_limits"); /* memory_limits, table */
    ctx->memory_limits = lua_tonumber(L, -1);
    if (ctx->memory_limits <= 0) {
        luaL_error(L, "bad memory_limits");
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "data_in"); /* data_in, table */
    path = lua_tostring(L, -1);
    ctx->data_in_fd = open(path, O_RDONLY);
    if (ctx->data_in_fd <= 0) {
        luaL_error(L, "bad data_in path: %s", strerror(errno));
    }

    lua_pop(L, 1); /* table */

    lua_getfield(L, -1, "user_out"); /* user_out_fd, table */
    path = lua_tostring(L, -1);
    ctx->user_out_fd = open(path, O_WRONLY);
    if (ctx->user_out_fd <= 0) {
        close(ctx->data_in_fd);
        luaL_error(L, "bad user_out path: %s", strerror(errno));
    }

    lua_pop(L, 1); /* table */
}


/* 
 * Well, we need to fetch the args from the Lua stack,
 * So, pass a Lua table is a good idea.
 */
static int
run(lua_State *L)
{
    sabo_ctx_t ctx;
    sabo_res_t info;
    int        n, type, rc;

    n = lua_gettop(L);
    if (n != 1) {
        luaL_error(L, "1 argument is expected, but seen %d", n);
    }

    type = lua_type(L, -1);
    if (type != LUA_TTABLE) {
        luaL_error(L, "table is expected, but got %s", lua_typename(L, type));
    }

    create_sabo_ctx(L, &ctx);

    rc = sabo_core_run(&ctx, &info);

    lua_createtable(L, 0, 3);

    lua_pushnumber(L, info.judge_flag);
    lua_setfield(L, -2, "judge_flag");

    lua_pushnumber(L, info.time_used);
    lua_setfield(L, -2, "time_used");

    lua_pushnumber(L, info.memory_used);
    lua_setfield(L, -2, "memory_used");

    close(ctx->data_in_fd);
    close(ctx->user_out_fd);

    lua_pushnumber(L, rc); /* rc table */

    return 2;
}


static const struct luaL_Reg lcore_lib[] = {
    { "run", run },
    { NULL, NULL }
};


int
luaopen_lcore(lua_State *L)
{
#if LUA_VERSION_NUM < 502
    luaL_register(L, "lcore", lcore_lib);
#else
    luaL_newlib(L, lcore_lib);
#endif
    return 1;
}
