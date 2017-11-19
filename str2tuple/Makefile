CXX=g++
CXXFLAGS=-std=c++14 -march=native -Wall -Wextra -Wpedantic

RM=rm -f

#SRCS=$(shell ls ./src/*/*.cpp)
#OBJS=$(subst .cpp,.o,$(SRCS))

all: CXXFLAGS += -O3
all: main

debug: CXXFLAGS += -DDEBUG -g -O0
debug: main

static: CXXFLAGS += -O3 -static
static: main

main: $(OBJS) 
	$(CXX) ./src/main.cpp $(CXXFLAGS) #$(OBJS)

#depend: .depend

#.depend: $(SRCS)
#	$(RM) ./.depend
#	$(CXX) $(CXXFLAGS) -MM $^ >> ./.depend;

#clean:
#	$(RM) $(OBJS)
	
#include .depend
