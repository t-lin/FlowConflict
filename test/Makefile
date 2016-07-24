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

all: clib ctest
	ln -s $(MKFILE_DIR)../FlowRecords.py $(MKFILE_DIR)FlowRecords.py

# Create static library for FlowRecords
clib:
	cd $(MKFILE_DIR)..; make $@

ctest: $(TEST_DIR)test.cpp clib
	g++ $(CFLAGS) $< -I $(MKFILE_DIR).. -L $(MKFILE_DIR)../build -l flowrecords -l bitscan -o $@

clean:
	rm -f $(MKFILE_DIR)ctest
	rm -f $(MKFILE_DIR)FlowRecords.py

