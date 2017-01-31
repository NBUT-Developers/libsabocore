#include "sabo_core.h"
#include <Python.h>
#include <stdlib.h>


char*
process_arg(PyObject *arg) {
    PyObject *temp = PyUnicode_AsUTF8String(arg);
    return PyBytes_AsString(temp);
}


static PyObject*
py_run(PyObject *self, PyObject *args) {
    PyObject* judge_config;
    sabo_ctx_t ctx;
    PyArg_ParseTuple(args, "O", &judge_config);

    ctx.executor      = process_arg(PyDict_GetItemString(judge_config, "executor"));
    ctx.data_in_fd    = atoi(process_arg(PyDict_GetItemString(judge_config, "data_in_fd")));
    ctx.user_out_fd   = atoi(process_arg(PyDict_GetItemString(judge_config, "user_out_fd")));
    ctx.time_limits   = atoi(process_arg(PyDict_GetItemString(judge_config, "time_limits")));
    ctx.memory_limits = atoi(process_arg(PyDict_GetItemString(judge_config, "memory_limits")));
    ctx.use_sandbox   = atoi(process_arg(PyDict_GetItemString(judge_config, "use_sandbox")));

    sabo_res_t res;

    res.time_used = -1;
    res.memory_used = -1;
    const char *err = sabo_core_run(&ctx, &res);

    return Py_BuildValue("(siii)", err, res.judge_flag, res.time_used, res.memory_used);
}


static PyMethodDef coreMethods[] = {
    {"run", py_run, METH_VARARGS, "Sabo judger core"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef coreModule = {
    PyModuleDef_HEAD_INIT,
    "sabo_core",
    NULL,
    -1,
    coreMethods
};


/* initial */
PyMODINIT_FUNC
PyInit_sabo_core() {
    return PyModule_Create(&coreModule);
}
