# FlowConflict
An optimized C++ library for conflict detection in OpenFlow 1.0 rules
Includes a Python wrapper module enabling use in Python-based programs

Depends on the *bitscan* library. To build it, simply run the **setup_bitscan.sh** script

To compile FlowConflict library...
  - Both C++ and Python libraries, simply run **make**
  - Just the C++ library, run **make clib**
  - Just the Python library, run **make pylib**
  - Clean all, run **make clean**

Tested with:
  - Ubuntu 12.04 (kernel 3.2) and 14.04 (kernel 3.13)
  - g++ 4.6 and 4.8
