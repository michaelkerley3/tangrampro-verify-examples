CC=gcc
CXX=g++
CPPFLAGS=  -Wall -Wpointer-arith \
                   -Wwrite-strings -Wcast-qual -Wcast-align -Wformat-security \
                   -pedantic-errors -fstack-protector-all \
                   -fPIC -std=c++11 -pthread -Wno-deprecated-declarations

ROOTDIR=$(shell pwd)
SRCDIR=$(ROOTDIR)/src
CSIINC=$(ROOTDIR)/code-gen/include
GENERICAPIINC=$(ROOTDIR)/code-gen/local_install/include/tangramgenericapicpp
TRANSPORTINC=$(ROOTDIR)/code-gen/local_install/include/tangramTransports/base
TRANSPORTZMQINC=$(ROOTDIR)/code-gen/local_install/include/tangramTransports/zmq
SERIALINC=$(ROOTDIR)/code-gen/local_install/include/tangramdirectserializer
RANDINC=$(ROOTDIR)/code-gen/local_install/include/tangramrandpopulater
SRCS:=$(shell find $(SRCDIR) -name '*.cpp')
OBJS:=$(SRCS:%=$(OBJDIR)/%.o)
LIBS=-l:libtangramgeneric.a -l:libtangramgenericapi.a -l:libtangramrandpopulater.a -l:libtangramdirectserializer.a -lzmq -l:libtangram_transport.so
LDFLAGS=-L$(ROOTDIR)/code-gen/build/libs -L$(ROOTDIR)/code-gen/local_install/lib $(LIBS)


INCLUDES=-I$(CSIINC) -Iinclude -I$(GENERICAPIINC) -I$(TRANSPORTINC) -I$(TRANSPORTZMQINC) -I$(SERIALINC) -I$(RANDINC)

ethan : $(OBJS)
	$(CXX) $(INCLUDES) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
