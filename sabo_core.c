
/*
 * Copyright (C) 2016-2017 Alex(zchao1995@gmail.com)
 */


#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <sys/fcntl.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "sabo_core.h"


#if __WORDSIZE == 64
    #define SYSCALL(reg) ((reg)->orig_rax)
    #define REG_ARG_1(reg) ((reg)->rdi)
    #define REG_ARG_2(reg) ((reg)->rsi)
#else
    #define SYSCALL(reg) ((reg)->orig_eax)
    #define REG_ARG_1(reg) ((reg)->ebx)
    #define REG_ARG_2(reg) ((reg)->ecx)
#endif


#define   SABO_UNLIMIT          -1
#define   SABO_ALLOWED          1
#define   SABO_FORBIDDEN        0
#define   SABO_UNKNOWN          (-1)
#define   SABO_INTERNAL_ERROR   (-2)

#define   FALSE                 0
#define   TRUE                  1
#define   SABO_DEFTIME          1000
#define   SABO_DEFMEM           65536

#define   SABO_BUFFER_SIZE      1024

#define   GOOD_FD(fd)   (fcntl(fd, F_GETFD) != -1 && errno != EBADF)
#define   GOOD_FILE(file, mode) (access(file, mode) == 0)

typedef struct {
    const char *first;
    int         second;
} sabo_sofile_t;

typedef struct {
    int last_use_mem;
    char err[NAME_MAX];
} sabo_global_t;

sabo_global_t sg_data;
int sabo_syscall[SABO_BUFFER_SIZE];

int sabo_syscall_whitelist[] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 21, 59, 89, 158, 231};

sabo_sofile_t sabo_sofile_whitelist[] = {

    { "ld.so.cache"   , O_RDONLY | O_CLOEXEC },
    { "libstdc++.so.6", O_RDONLY | O_CLOEXEC },
    { "libc.so.6"     , O_RDONLY | O_CLOEXEC },
    { "libm.so.6"     , O_RDONLY | O_CLOEXEC },
    { "libgcc_s.so.1" , O_RDONLY | O_CLOEXEC },
};


static void sabo_core_init();
static void sabo_set_limit(const sabo_ctx_t *ctx);
static void sabo_child_run(const sabo_ctx_t *ctx);
static void sabo_monitor_run(pid_t child, const sabo_ctx_t *ctx, sabo_res_t *resinfo);
static void sabo_check(sabo_ctx_t *ctx);
static void sabo_kill(pid_t child);
static int sabo_check_accessfile(const char *filepath, const int size, int flags, const sabo_ctx_t *ctx);
static int sabo_hack_open_file(struct user_regs_struct *reg, pid_t child, const sabo_ctx_t *ctx);
static int sabo_hack_syscall(int syscall_num, const sabo_ctx_t *ctx);
static unsigned int sabo_get_process_runtime(const struct rusage *runinfo);
static unsigned int sabo_get_process_runmem(const struct rusage *runinfo, int language, pid_t child);
static unsigned int sabo_get_proc_status(const char *item, pid_t pid);


static void
sabo_core_init()
{

    int i, call_num;
    int all, one, count;

    all = sizeof(sabo_sofile_whitelist);
    one = sizeof(sabo_sofile_t);

    count = all / one;

    sg_data.last_use_mem = 0;
    sg_data.err[0] = '\0';

    memset(sabo_syscall, SABO_FORBIDDEN, sizeof(sabo_syscall));

    for (i = 0; i < count; ++i) {

        call_num = sabo_syscall_whitelist[i];
        sabo_syscall[call_num] = SABO_ALLOWED;
    }
}


/*
 * This function is used to check the file whether is allowed to open when the
 * child process called the open system call, if the file is not in the
 * file_white_list, user solution will be judged as MC(malicious code)
 */
static int
sabo_check_accessfile(const char *filepath, const int size, int flags, const sabo_ctx_t *ctx)
{

    int i, all, one, count, len;
    const char *file;

    all = sizeof(sabo_sofile_whitelist);
    one = sizeof(sabo_sofile_t);
    count = all / one;

    file = filepath + size - 1;
    len = size;

    while (*file != '/' && len) {
        file--;
        len--;
    }

    if (*file == '/') {
        file++;
    }

    for (i = 0; i < count; ++i) {

        if (!strncmp(file, sabo_sofile_whitelist[i].first, len)
            && (flags & sabo_sofile_whitelist[i].second)
            == sabo_sofile_whitelist[i].second)
        {
            return SABO_ALLOWED;
        }
    }

    return SABO_FORBIDDEN;
}


static int
sabo_hack_syscall(int syscall_num, const sabo_ctx_t *ctx)
{

    int  i;
    int  size;
    int *p;

    p = sabo_syscall_whitelist;
    size = sizeof(sabo_syscall_whitelist) / sizeof(int);

    for (i = 0; i < size; i++) {
        
        if (p[i] == syscall_num) {
            return SABO_ALLOWED;
        }
    }

    return SABO_FORBIDDEN;
}


static unsigned int
sabo_get_proc_status(const char *item, pid_t pid)
{
    static char name[NAME_MAX];
    unsigned int res;
    int itemlen;

    snprintf(name, sizeof(name), "/proc/%d/status", pid);
    FILE *p = fopen(name, "r");

    if (p == NULL) {
        return sg_data.last_use_mem;
    }

    res = 0;
    itemlen = strlen(item);

    while (fgets(name, NAME_MAX - 1, p)) {

        name[NAME_MAX - 1] = '\0';
        if (strncmp(item, name, itemlen) == 0) {
            sscanf(name + itemlen + 1, "%d", &res);
            break;
        }
    }

    fclose(p);
    sg_data.last_use_mem = res;

    return res;
}


/*
 * this function is used to get the filepath and flag, reference:
 * https://github.com/lodevil/Lo-runner
 * if long is 4 bytes, 4 char
 * if long is 8 bytes, 8 char
 * FIXME: this logic need to improve
 */
static int
sabo_hack_open_file(struct user_regs_struct *reg, pid_t child, const sabo_ctx_t *ctx)
{

    static long file_temp[NAME_MAX];
    long t;
    int i, j, flag, size;
    const char *test;

    size = 0;

    for (i = 0; i < NAME_MAX; ++i) {

        t = ptrace(PTRACE_PEEKDATA, child, REG_ARG_1(reg) + i * sizeof(long), NULL);

        file_temp[i] = t;
        test = (const char *) &file_temp[i];
        flag = FALSE;

        for (j = 0; j < (int) sizeof(long); ++j) {

            if (!test[j]) {
                file_temp[size - 1] = 0;
                flag = TRUE;
                break;
            }

            size++;
        }

        if (flag) {
            break;
        }
    }

    return sabo_check_accessfile((const char *) file_temp, size, REG_ARG_2(reg), ctx);
}


static unsigned int
sabo_get_process_runtime(const struct rusage *runinfo)
{
    /*
     * Get the running time
     * time = cpu time + user time
     */

    unsigned int time_used;
    time_used = runinfo->ru_utime.tv_sec * 1000 + runinfo->ru_utime.tv_usec / 1000;
    time_used += runinfo->ru_stime.tv_sec * 1000 + runinfo->ru_stime.tv_usec / 1000;

    return time_used;
}


static unsigned int
sabo_get_process_runmem(const struct rusage *runinfo, int language, pid_t child)
{
    /*
     * Get the used memory
     * ru_maxrss maybe the result is larger than the real usage
     * about ru_maxrss:
     * This is the maximum resident set size used (in kilobytes).
     * For RUSAGE_CHILDREN, this is the resident set size of the
     * largest child, not the maximum resident set
     * size of the process tree
     */

    if (!language) {
        /* for java */
        return runinfo->ru_minflt * (getpagesize() >> 10);

    } else {
        return sabo_get_proc_status("VmData:", child);
    }
}


static void
sabo_kill(pid_t child)
{
    kill(child, SIGKILL);
    wait(&child);
}


static void
sabo_monitor_run(pid_t child, const sabo_ctx_t *ctx, sabo_res_t *res)
{
    int                     runstat, language;
    int                     time_used, memory_used;
    int                     judge_flag, signal;
    long long               syscall;
    struct rusage           runinfo;
    struct user_regs_struct reg;

    time_used   = -1;
    judge_flag  = SABO_UNKNOWN;
    language = ctx->language;

    for ( ;; ) {

        /* block the monitor process */
        wait4(child, &runstat, 0, &runinfo);

        memory_used = sabo_get_process_runmem(&runinfo, language, child);

        if (memory_used == -1) {

            judge_flag = SABO_SYSERR;
            time_used = 0;
            memory_used = 0;
            sabo_kill(child);
            goto done;
        }

        time_used = sabo_get_process_runtime(&runinfo);

        if (time_used > ctx->time_limits) {

            sabo_kill(child);
            judge_flag = SABO_TLE;
            time_used = ctx->time_limits;
            goto done;
        }

        if (memory_used > ctx->memory_limits) {

            sabo_kill(child);
            judge_flag = SABO_MLE;
            memory_used = ctx->memory_limits > memory_used ? ctx->memory_limits : memory_used;
            goto done;
        }

        if (WIFEXITED(runstat)) { /* if the child process exit */

            judge_flag = SABO_DONE; /* Note: this AC just stand that the user program is run successfully */

            goto done;

        } else if (WIFSTOPPED(runstat)) {

            signal = WSTOPSIG(runstat);
            switch (signal) {

            case SIGFPE:

                judge_flag = SABO_RE_DBZ;
                sabo_kill(child);
                goto done;

            case SIGSEGV:

                judge_flag = SABO_RE;
                sabo_kill(child);
                goto done;

            case SIGALRM:

                /* Time Limit Exceed CPU TIME or USER TIME */
                judge_flag = SABO_TLE;

                time_used = ctx->time_limits;
                sabo_kill(child);
                goto done;

            case SIGTRAP:

                if (!language) {
                    ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                    continue;
                }

                ptrace(PTRACE_GETREGS, child, NULL, &reg);
                syscall = SYSCALL(&reg);

                if (syscall == SYS_open) {

                    if (sabo_hack_open_file(&reg, child, ctx) == SABO_FORBIDDEN) {
                        /* use forbidden dynamic shared file */
                        judge_flag = SABO_MC;
                        sabo_kill(child);
                        goto done;
                    }
                }

                if (sabo_hack_syscall(syscall, ctx) == SABO_FORBIDDEN) {

                    judge_flag = SABO_MC;
                    sabo_kill(child);
                    goto done;
                }

                ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                continue;
            }

        } else {

            /* Other case will be treated as MC */
            judge_flag = SABO_MC;
            goto done;
        }
    }

done:

    res->judge_flag = judge_flag;
    res->time_used = time_used;
    res->memory_used = memory_used;
}


static void
sabo_set_limit(const sabo_ctx_t *ctx)
{
    /*time_limits */
    struct itimerval timer;

    gettimeofday(&timer.it_value, NULL);

    timer.it_interval.tv_usec = 0;
    timer.it_interval.tv_sec = 0;

    timer.it_value.tv_sec = ctx->time_limits / 1000; /* seconds */
    timer.it_value.tv_usec = ctx->time_limits % 1000 * 1000; /* microseconds */

    setitimer (ITIMER_REAL, &timer, NULL);

    /* memory_limits KB */
    struct rlimit mem_limits; mem_limits.rlim_max = ctx->memory_limits * 1024;
    mem_limits.rlim_cur = ctx->memory_limits * 1024;

    setrlimit(RLIMIT_DATA, &mem_limits);
}


static void
sabo_child_run(const sabo_ctx_t *ctx)
{

    int rv;

    rv = dup2(ctx->data_in_fd, STDIN_FILENO);
    if (rv < 0) {
        sprintf(sg_data.err, "data_in_fd: %s\n", strerror(errno));
        return;
    }

    rv = dup2(ctx->user_out_fd, STDOUT_FILENO);
    if (rv < 0) {
        sprintf(sg_data.err, "user_out_fd: %s\n", strerror(errno));
        return;
    }

    freopen("/dev/null", "a", stderr);

    /*
     * set time limits and memory limits
     * but for spj src file, unnecessary
     * compare user.out with data.out
    */

    /*Trace itself */
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    sabo_set_limit(ctx);

    /* exec the user process */
    if (ctx->language == SABO_JAVA) {
        rv = execl(ctx->executor, ctx->code_bin_file, NULL);
        if (rv < 0) {
            sprintf(sg_data.err, "execl: %s(executor)\n", strerror(errno));
        }

    } else {

        /*Execute the java program with the jvm security policy */
        rv = execl(ctx->executor, "java", "-cp", ctx->classpath, "-Xss8M", "-Djava.security.manager", "-Djava.security.policy==policy", "-Djava.awt.headless=TRUE", ctx->code_bin_file, NULL);
        sprintf(sg_data.err, "execl for java: %s\n", strerror(errno));
    }
}


static void
sabo_check(sabo_ctx_t *ctx)
{

    if (ctx->time_limits < 0) {
        ctx->time_limits = SABO_DEFTIME;
    }

    if (ctx->time_limits < 0) {
        ctx->memory_limits = SABO_DEFMEM;
    }

}


const char *
sabo_core_run(sabo_ctx_t *ctx, sabo_res_t *info)
{
    pid_t child;
    int fd[2];
    int rv, flags, n;

    if (ctx == NULL) {
        return "args: ctx null";
    }

    if (info == NULL) {
        return "args: info null";
    }

    rv = pipe(fd);
    if (rv < 0) {
        sprintf(sg_data.err, "create pipe failed %s\n", strerror(errno));
        return sg_data.err;
    }

    sabo_core_init();

    sabo_check(ctx);

    if ((child = fork()) < 0) {
        return strerror(errno);
    }

    if (child == 0) {
        close(fd[0]);

        sabo_child_run(ctx);

        write(fd[1], sg_data.err, strlen(sg_data.err));
        exit(EXIT_FAILURE);

    } else {
        close(fd[1]);
        usleep(100);
        
        flags = fcntl(fd[0], F_GETFL);
        fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

        n = read(fd[0], sg_data.err, sizeof(sg_data.err));
        if (n == -1 && errno == EAGAIN) {
            sg_data.err[0] = '\0';
            sabo_monitor_run(child, ctx, info);
        }
    }

    if (sg_data.err[0] != '\0') {
        info->time_used = -1;
        info->memory_used = -1;
        info->judge_flag = SABO_SYSERR;

        return sg_data.err;
    }

    close(fd[0]);

    return NULL;
}
