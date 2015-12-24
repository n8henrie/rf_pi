CXX = g++-4.9
CXXFLAGS = -std=c++11

all: send RFSniffer print_sched

send: RCSwitch.o send.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi
	$(CXX) -shared -fPIC $+ -o $@.so -lwiringPi
	-sudo setcap cap_sys_nice+ep $@

RFSniffer: RCSwitch.o RFSniffer.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

print_sched: print_sched.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@

node:
	npm install .

.PHONY: all clean clean-all node

clean:
	$(RM) -r *.o *.so __pycache__ send RFSniffer 

clean-all: clean
	$(RM) -r node_modules
