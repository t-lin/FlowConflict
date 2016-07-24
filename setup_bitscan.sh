#!/bin/bash
TOP_DIR=$(cd $(dirname "$0") && pwd)

# Pull in bitscan submodule
cd ${TOP_DIR}
git submodule init
git submodule update

# Add headers to the system for easy #include's
cd ${TOP_DIR}/bitscan
HEADERS=`ls *.h`
for i in ${HEADERS}; do
    sudo ln -fs ${TOP_DIR}/bitscan/${i} /usr/local/include/${i}
done

# Determine compiler version for std flag
COMPILER_VERSION=`g++ -dumpversion`
if (( `echo "${COMPILER_VERSION} <= 4.6" | bc` )); then
    STD=c++0x
else
    STD=c++11
fi

# Compile bitscan source code
OUTPUT=`g++ -std=${STD} -O3 -Wall -fPIC -c *.c* 2>&1`
ERROR=`echo "$OUTPUT" | grep -i error`
if [[ -n ${ERROR} ]]; then
    echo "${ERROR}"
    exit 1
fi

# Compile the library and update the system library cache
LIB_NAME=libbitscan.so
MAJOR_VERS=1
MINOR_VERS=0
LIB_FILENAME=${LIB_NAME}.${MAJOR_VERS}.${MINOR_VERS}

g++ -shared -Wl,-soname,${LIB_NAME}.${MAJOR_VERS} -o ${LIB_FILENAME} *.o
sudo ln -fs ${TOP_DIR}/bitscan/${LIB_FILENAME} /usr/local/lib/${LIB_NAME}.${MAJOR_VERS}
sudo ln -fs ${TOP_DIR}/bitscan/${LIB_FILENAME} /usr/local/lib/${LIB_NAME}

sudo ldconfig
