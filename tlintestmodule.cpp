#include <Python.h>
#include <iostream>
#include <utility>
#include <ctime>
#include <unordered_map>

//#include <bitscan.h>
#include "FlowRecords.cpp"
using namespace std;

// Declare single global instance
// TODO: In the future, figure out how to allow creation of multiple instances
myGlobalClass myglobal;


/* ========== HELPER FUNCTIONS ========== */
// Helper function for printing out a Python object...
const char *printPyObject(PyObject *obj) {
    PyObject* objectsRepresentation = PyObject_Repr(obj);
    return PyString_AsString(objectsRepresentation);
}

//unsigned long get_time_in_ms() {
//    return (unsigned long)((double(clock()) / CLOCKS_PER_SEC) * 1000);
//}


/* ========== EXPOSED MODULE METHODS ========== */
static PyObject *tlintest_increment(PyObject *self, PyObject *args) {
    myglobal.increment();

    Py_RETURN_NONE;
}

static PyObject *tlintest_test(PyObject *self, PyObject *args) {
    PyObject *mydict = NULL;
    PyArg_ParseTuple(args, "O", &mydict);

    if (!mydict) {
        cout << "Error parsing..." << endl;
        Py_RETURN_NONE;
    }

    if (PyDict_Check(mydict)) {
        cout << "Is a dictionary!" << endl;

        PyObject *keyList = PyDict_Keys(mydict); // get keys of dict as a list
        Py_ssize_t listSize = PyList_Size(keyList); // get size of list
        PyObject *item = NULL;
        for (Py_ssize_t i = 0; i < listSize; i++) {
            item = PyList_GetItem(keyList, i);
            cout << printPyObject(item) << endl;
        }
    }
    else
        cout << "Not a dictionary! :(" << endl;

    Py_RETURN_NONE;
}


/* Receives dictionary and parses all the parameters that are recognized
 * On error, returns None
 * On success, will return ...???
 */
static PyObject *tlintest_insertFlow(PyObject *self, PyObject *args, PyObject *keywords) {
    FlowEntry flow;
    static char *kwlist[] = {"dpid", "in_port",
                             "dl_dst", "dl_src", "dl_type", "dl_vlan", "dl_vlan_pcp",
                             "nw_src", "nw_dst", "nw_proto", "nw_tos",
                             "tp_src", "tp_dst",
                             NULL}; // Sentinel value

    /* Format reference: https://docs.python.org/2/c-api/arg.html
     *  - K = Unsigned long long
     *  - H = Unsigned short
     *  - B = Unsigned char
     *  - I = Unsigned int
     */
    if (!PyArg_ParseTupleAndKeywords(args, keywords, "KHKKHHBIIBBHH", kwlist,
             &flow.dpid, &flow.in_port,
             &flow.dl_dst, &flow.dl_src, &flow.dl_type, &flow.dl_vlan, &flow.dl_vlan_pcp,
             &flow.nw_src, &flow.nw_dst, &flow.nw_proto, &flow.nw_tos,
             &flow.tp_src, &flow.tp_dst))
        return NULL;

    //cout << "Received params are: " << endl;
    //printf("DPID: %llu, in_port: %hu\n", dpid, in_port);
    //printf("dl_src: %llu, dl_dst: %llu, dl_type: %hu, dl_vlan: %hu, dl_vlan_pcp: %hhu\n",
    //        dl_dst, dl_src, dl_type, dl_vlan, dl_vlan_pcp);
    //printf("nw_src: %u, nw_dst: %u, nw_proto: %hhu, nw_tos: %hhu\n",
    //        nw_src, nw_dst, nw_proto, nw_tos);
    //printf("tp_src: %hu, tp_dst: %hu\n\n", tp_src, tp_dst);

    //return Py_BuildValue("i", myglobal.insertFlow(flow, true)); // print conflicts
    return Py_BuildValue("i", myglobal.insertFlow(flow));
}

static PyMethodDef TestMethods[] = {
    {"increment", tlintest_increment, METH_VARARGS, "Increment the global."},
    {"test", tlintest_test, METH_VARARGS, "testing..."},
    {"insertFlow", (PyCFunction)tlintest_insertFlow, METH_VARARGS | METH_KEYWORDS, "Insert a flow entry"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



/* Define strings representing the various OpenFlow fields here within the module
 * Defining them here allows Python developers to import them for consistency
 *  - i.e.
 *      >>> import mytestmodule
 *      >>> print mytestmodule.ETH_DST
 *      dl_dst
 */
//const char FIELD_TO_MEMBERNAME[][20] = {"IN_PORT", "in_port",
//                                        "ETH_SRC", "dl_src",
//                                        "ETH_DST", "dl_dst",
//                                        "ETH_TYPE", "dl_type",
//                                        "ETH_VLAN", "dl_vlan",
//                                        "ETH_VLAN_PCP", "dl_vlan_pcp",
//                                        "IP_SRC", "nw_src",
//                                        "IP_DST", "nw_dst",
//                                        "IP_PROTO", "nw_proto",
//                                        "IP_TOS", "nw_tos",
//                                        "TP_SRC", "tp_src",
//                                        "TP_DST", "tp_dst"};
//
//
//#define NUMFIELDS_OF10 12
//
//// OF10_FIELDS macro is used to define a Python dictionary object
////                    KEY        VALUE
//#define OF10_FIELDS "IN_PORT", "in_port", \
//                    "ETH_SRC", "dl_src", \
//                    "ETH_DST", "dl_dst", \
//                    "ETH_TYPE", "dl_type", \
//                    "ETH_VLAN", "dl_vlan", \
//                    "ETH_VLAN_PCP", "dl_vlan_pcp", \
//                    "IP_SRC", "nw_src", \
//                    "IP_DST", "nw_dst", \
//                    "IP_PROTO", "nw_proto", \
//                    "IP_TOS", "nw_tos", \
//                    "TP_SRC", "tp_src", \
//                    "TP_DST", "tp_dst"

PyMODINIT_FUNC inittlintest() {
    // Create module and add methods
    PyObject *module = Py_InitModule("tlintest", TestMethods);

    if (module) {
        // Add module member variables
        //for (int i = 0; i < NUMFIELDS_OF10; i++) {
        //    PyModule_AddStringConstant(module, FIELD_TO_MEMBERNAME[i*2], FIELD_TO_MEMBERNAME[i*2+1]);
        //}

        ////test create dict
        //string format;
        //for (int i = 0; i < NUMFIELDS_OF10; i++)
        //    format += "s:s";

        //format = "{" + format + "}";
        //PyModule_AddObject(module, "OF10_FIELDS", Py_BuildValue(format.c_str(), OF10_FIELDS));
    }
}
