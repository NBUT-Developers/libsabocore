[![Build Status](https://travis-ci.org/tokers/libsabocore.svg?branch=master)](https://travis-ci.org/tokers/libsabocore.svg?branch=master)

libsabocore
===========

---------------------------------------

<img src="./trivial/sabo.png" />

> Sabo is the Revolutionary Army's Chief of Staff, recognized as the "No. 2" of the > 
> entire organization, ranking directly under Monkey D. Dragon himself.[2] Next to > 
> being the sworn brother of Monkey D. Luffy and Portgas D. Ace, he is of noble > 
> descent, the son of Outlook III, as well as via his parents the adopted brother of 
> Stelly, the current king of the Goa Kingdom, whom Sabo has never recognized as such.

Synopsis
--------

```c
#include "sabo_core.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


void
judge()
{
    sabo_ctx_t ctx;
    sabo_res_t res;
    const char *err, *judge_flag;

    ctx.user_out_fd = open("/path/to/user.out", O_WRONLY);
    if (ctx.user_out_fd < 0) {
        fprintf(stderr, "failed to open user.out: %s\n", strerror(errno));
        return;
    }

    ctx.data_in_fd = open("/path/to/data.in", O_RDONLY);
    if (ctx.data_in_fd < 0) {
        fprintf(stderr, "failed to open user.out: %s\n", strerror(errno));
        return;
    }

    ctx.executor = "/path/to/Main"; /* Main is an ELF format file which can be executed */
    ctx.code_bin_file = "Main";

    ctx.time_limits = 1000;
    ctx.memory_limits = 65536;

    ctx.use_sandbox = 1; /* use ptrace to monitor the user process */

    ctx.classpath = NULL; /* only for java */

    /*
     * or for java
     * ctx.executor = "/path/to/java/bin/java";
     * ctx.code_bin_file = "Main";
     * ctx.use_sandbox = 0
     * ctx.classpath = "path/to/classfile/"
     * note: you need not set use_sandbox to 1 because we use jvm's strategy
     */

    err = sabo_core_run(&ctx, &res);

    if (err) {
        fprintf(stderr, "failed to run user code: %s\n", err);
        return;
    }

    switch (res.judge_flag) {

    case SABO_DONE:
        judge_flag = "done";
        break;

    case SABO_TLE:
        judge_flag = "TLE";
        break;

    case SABO_MLE:
        judge_flag = "MLE";
        break;

    case SABO_RE:
        judge_flag = "RE";
        break;

    case SABO_RE_DBZ:
        judge_flag = "RE(FPE)";
        break;

    case SABO_MC:
        judge_flag = "MC";
        break;

    default:
        judge_flag = "SYSERR";
        break;
    }

    fprintf(stdout, "judge flag: %s time_used: %d, memory_used: %d\n",
                    judge_flag, res.time_used, res.memory_used);
}

```

You need to compile the libsabocore code with `make` and you will get `libsacore.a` and `libsabocore.so`.
<br>


Introduction
-----------

`libsabocore` is an ACM/ICPC judger core lib in ANSI C. <br>
One can use `libsabocore` with packaging it under any languages, such as my another program [sabo](https://github.com/tokers/sabo), they are not twin(the latter's core interface is ugly and is internal) but the principle is same(ptrace). <br>

The only thing you need to care is the structure `sabo_ctx_t` and `sabo_res_t`, the former is the container that you pass the needed args to `libsabocore`, and you can get result from the latter.
<br>

Data structure and interface
--------------------------
Now, let's see both of them and the interface.

```c
typedef struct {

    const char *code_bin_file;
    const char *executor;


    int data_in_fd;
    int user_out_fd;

    int time_limits;
    int memory_limits;

    int use_sandbox; /* FIXME, just for java now */

    const char *classpath; /* FIXME, for java class path */

} sabo_ctx_t;
```

|item| meaning|
|---|---|
|`code_bin_file`| the file name of the code(compiled)|
|`executor`| the code interpreter(full name) for java, executor is the java binary path(/path/to/java), for C/C++, executor is the ELF file path(/path/to/a.out)|
|`data_in_fd`| the file descriptor of data.in|
|`user_out_fd`| the file descriptor of user.out|
|`time_limits`| the time resource limit of your process|
|`memory_limits`| the memory resource limit of your process|
|`use_sandbox`| if use_sandbox is 1, then ptrace will be used to monitor your process, for java program, you can just set it to 0 because we use JVM's strategy|
|`classpath`| used for java, you need to set an appropriate path where you hope java will find class in there|
<br>

```c
typedef struct {
    int time_used;
    int memory_used;
    int judge_flag;

} sabo_res_t;
```

|item| meaning |
|----|---------|
|`time_used`| cost time of your process|
|`memory_used`| cost memory of your process|
|`judge_flag`| judge result of you process|

For the meaning of `judge_flag`, you can see these macros.

```c
#define   SABO_DONE      0
#define   SABO_TLE       2
#define   SABO_MLE       3
#define   SABO_RE        5
#define   SABO_RE_DBZ    6
#define   SABO_SYSERR    8
#define   SABO_MC        10
```

|item| meaning |
|----|---------|
|`SABO_DONE`| your process run successfully(Not Accepted !)|
|`SABO_TLE`| Time Limits Exceed|
|`SABO_MLE`| Memory Limits Exceed|
|`SABO_RE`| Runtime Error|
|`SABO_RE_DBZ`|Runtime Error because the `SIGFPE`(`floating point exception`)|
|`SABO_SYSERR`|some args invalid and you can get an error string from the interface|
|`SABO_MC`| you process is malicious(open some important file or calling some invalid syscall)|
<br>

```c
const char *sabo_core_run(sabo_ctx_t *ctx, sabo_res_t *info);
```

An error string will be returned, error is NULL if there is no error.
<br>

API
---

There is one python3 api in api/python3, you can get the so file if run this command.

```bash
python3 setup.py build_ext
```

these api was wrote by myself, it is so simple that you are not expected to use it directly, but you can reference it.

If you are the guy who like LUA, then luajit will be recommended, you can call C funtions with pure LUA code. See the t/test_libsabocore.lua.

Others also work.


Others
-----

- the syscall list and open file list can be custom in the future.
- if you want `SPECIAL JUDGE`, you can run the code with libsabocore firstly, then run the spjcode directly because the spj code is in mastery.
