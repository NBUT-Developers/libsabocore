#include <sabo_core.h>
#include <Python.h>


PyMODINIT_FUNC PyInit_sabo_core(void);
static PyObject* py_run(PyObject *self, PyObject *args);
char* process_arg(PyObject *arg);


char*
process_arg(PyObject *arg) {
    PyObject *temp = PyUnicode_AsUTF8String(arg);
    return PyBytes_AsString(temp);
}


static PyObject*
py_run(PyObject *self, PyObject *args) {
    PyObject* judge_config;
    sabo_run_config config;
    PyArg_ParseTuple(args, "O", &judge_config);

    config.exe           = process_arg(PyDict_GetItemString(judge_config, "exe"));
    config.code_path     = process_arg(PyDict_GetItemString(judge_config, "code_path"));
    config.in_path       = process_arg(PyDict_GetItemString(judge_config, "in_path"));
    config.out_path      = process_arg(PyDict_GetItemString(judge_config, "out_path"));
    config.user_path     = process_arg(PyDict_GetItemString(judge_config, "user_path"));
    config.spj_path      = process_arg(PyDict_GetItemString(judge_config, "spj_path"));
    config.time_limits   = atoi(process_arg(PyDict_GetItemString(judge_config, "time_limits")));
    config.memory_limits = atoi(process_arg(PyDict_GetItemString(judge_config, "memory_limits")));
    config.is_spj        = atoi(process_arg(PyDict_GetItemString(judge_config, "is_spj")));
    config.use_sandbox   = atoi(process_arg(PyDict_GetItemString(judge_config, "use_sandbox")));
    config.err_path      = "/dev/null";

    sabo_result_info res;
    res.judge_flag = SABO_UNKNOWN;

    if (config.code_path == NULL || config.in_path == NULL ||
        config.out_path == NULL || config.user_path == NULL ||
        (config.is_spj && config.spj_path == NULL) ||
         (config.use_sandbox == 0 && config.exe == NULL)
         || config.time_limits == 0 || config.memory_limits == 0) {
        res.judge_flag = SABO_SYSERR;

        return Py_BuildValue("(iii)", res.judge_flag, 0, 0);
    }


    res.time_used = -1;
    res.memory_used = -1;
    sabo_core_init();
    sabo_core_run(&config, &res);

    if (res.judge_flag == SABO_TLE) {
        res.time_used = config.time_limits;
    } else if (res.judge_flag == SABO_MLE) {
        res.memory_used = config.memory_limits;
    }

    return Py_BuildValue("(iii)", res.judge_flag, res.time_used, res.memory_used);
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
