all: FlowRecords.cpp tlintestmodule.cpp
	g++ -std=c++0x -fno-strict-aliasing -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes -fPIC -I/usr/include/python2.7 -c tlintestmodule.cpp -o build/temp.linux-x86_64-2.7/tlintestmodule.o
	g++ -pthread -shared -Wl,-O3 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro build/temp.linux-x86_64-2.7/tlintestmodule.o -lbitscan -o build/lib.linux-x86_64-2.7/tlintest.so

