CXX = g++-4.9
CXXFLAGS = -std=c++11

all: send RFSniffer

send: RCSwitch.o send.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi
	$(CXX) -shared -fPIC $+ -o $@.so -lwiringPi
	-sudo setcap cap_sys_nice+ep $@

RFSniffer: RCSwitch.o RFSniffer.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

.PHONY: all clean

clean:
	$(RM) -r *.o *.so __pycache__ send RFSniffer 
