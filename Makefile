# For compiling the C++ and Python libraries
# Essentially same as using setup.py, but w/ O3 optimization

MKFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
COMPILER_VERS_LEQ46 := $(shell echo "`g++ -dumpversion` <= 4.6" | bc)
ifeq "$(COMPILER_VERS_LEQ46)" "1"
    STD = c++0x
else
    STD = c++11
endif

CFLAGS = -std=$(STD) -O3 -g -Wall

all: clib pylib

# Create static library for FlowRecords
clib: build/libflowrecords.a

build/libflowrecords.a: build/FlowRecords.o
	ar rcs $@ $^

build/FlowRecords.o: FlowRecords.cpp FlowRecords.h
	mkdir -p build
	g++ $(CFLAGS) -fPIC -c $< -o $@

# TODO: Make this part more proper later... currently hard-coded af
pylib: pyFlowRecords.cpp FlowRecords.h clib
	mkdir -p build
	g++ $(CFLAGS) -fno-strict-aliasing -DNDEBUG -fwrapv -Wstrict-prototypes -fPIC -I/usr/include/python2.7 -c $< -o $(MKFILE_DIR)build/pyFlowRecords.o
	g++ -pthread -shared -Wl,-O3 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro build/pyFlowRecords.o -lbitscan -L $(MKFILE_DIR)build -l flowrecords -o $(MKFILE_DIR)build/pyFlowRecords.so

install: pylib
	# This requires sudo acccess to place symlinks into the global Python path
	# If you do not have sudo access, open this Makefile and read further comments
# If no sudo access is available, you should develop from within the main project directory
#	1. Comment out the two active lines below
#	2. Add the following line to put a symlink to the C extension module in the main project directory
#		ln -fs $(MKFILE_DIR)build/pyFlowRecords.so $(MKFILE_DIR)_FlowRecords.so
#	3. Save and exit this Makfile, then re-run 'make install'
	sudo ln -fs $(MKFILE_DIR)build/pyFlowRecords.so /usr/local/lib/python2.7/dist-packages/_FlowRecords.so
	sudo ln -fs $(MKFILE_DIR)FlowRecords.py /usr/local/lib/python2.7/dist-packages/FlowRecords.py

clean:
	rm -f *.o
	rm -f *.a
	rm -rf build
	rm -f *.so

