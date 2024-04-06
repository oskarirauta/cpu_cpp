all: world
CXX?=g++
CXXFLAGS?=--std=c++23 -Wall -fPIC -g
LDFLAGS?=-L/lib -L/usr/lib

INCLUDES+= -I./include

OBJS:= \
	objs/main.o

CPU_DIR:=.
include common/Makefile.inc
include throws/Makefile.inc
include logger/Makefile.inc
include ./Makefile.inc

world: example

$(shell mkdir -p objs)

objs/main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

example: $(COMMON_OBJS) $(THROWS_OBJS) $(SCANNER_OBJS) $(LOGGER_OBJS) $(CPU_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -L. $(LIBS) $^ -o $@;

.PHONY: clean
clean:
	@rm -rf objs
	@rm -f example
