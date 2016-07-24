# For compiling the C++ and Python libraries
# Essentially same as using setup.py, but w/ O3 optimization

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

build/FlowRecords.o: FlowRecords.cpp
	mkdir -p build
	g++ $(CFLAGS) -fPIC -c $< -o $@

# TODO: Make this part more proper later... currently hard-coded af
#pylib: FlowRecords.cpp tlintestmodule.cpp
pylib: tlintestmodule.cpp clib
	mkdir -p build
	g++ $(CFLAGS) -fno-strict-aliasing -DNDEBUG -fwrapv -Wstrict-prototypes -fPIC -I/usr/include/python2.7 -c $< -o build/tlintestmodule.o
	g++ -pthread -shared -Wl,-O3 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro build/tlintestmodule.o -lbitscan -L ./build -l flowrecords -o build/tlintest.so
	ln -fs build/tlintest.so tlintest.so

clean:
	rm *.o
	rm *.a
	rm -rf build
	rm *.so

