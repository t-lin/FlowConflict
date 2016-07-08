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
    sudo rm /usr/local/include/${i} 2> /dev/null
    sudo ln -s ${TOP_DIR}/bitscan/${i} /usr/local/include/${i}
done

# Compile bitscan source code
OUTPUT=`g++ -std=c++11 -O3 -Wall -fPIC -c *.c* 2>&1`
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
sudo rm /usr/local/lib/${LIB_NAME}.${MAJOR_VERS} 2> /dev/null
sudo ln -s ${TOP_DIR}/bitscan/${LIB_FILENAME} /usr/local/lib/${LIB_NAME}.${MAJOR_VERS}
sudo rm /usr/local/lib/${LIB_NAME} 2> /dev/null
sudo ln -s ${TOP_DIR}/bitscan/${LIB_FILENAME} /usr/local/lib/${LIB_NAME}

sudo ldconfig
