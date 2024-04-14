CPU_DIR?=./cpu
INCLUDES += -I$(CPU_DIR)/include

CPU_OBJS:= \
	objs/cpu_tck_t.o \
	objs/cpu_node_t.o \
	objs/cpu_t.o \
	objs/cpu_process_t.o

objs/cpu_tck_t.o: $(CPU_DIR)/src/tck_t.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/cpu_node_t.o: $(CPU_DIR)/src/node_t.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/cpu_t.o: $(CPU_DIR)/src/cpu_t.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/cpu_process_t.o: $(CPU_DIR)/src/process_t.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;
