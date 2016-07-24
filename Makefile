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

	# By default, builds the library and tries to put a symlink in the global Python path
	# If you do not have sudo access, switch the comment on the two lines below to put the symlink in the local directory
	#ln -fs $(MKFILE_DIR)build/pyFlowRecords.so _FlowRecords.so
	sudo ln -fs $(MKFILE_DIR)build/pyFlowRecords.so /usr/local/lib/python2.7/dist-packages/_FlowRecords.so

ctest: test.cpp clib
	g++ $(CFLAGS) $< -L ./build -l flowrecords -l bitscan -o $@

clean:
	rm -f *.o
	rm -f *.a
	rm -rf build
	rm -f *.so
	rm -f ctest

