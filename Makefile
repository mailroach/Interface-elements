PROGRAM = interface-elem
CXX     = g++

LIBS= -lboost_regex-mt -lfreetype 
LIBDIRS = 

INCLUDEDIRS = 

CFLAGS = -O0 -g -Wall -std=c++0x $(INCLUDEDIRS)
LFLAGS = $(LIBS) $(LIBDIRS)

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) -o $@ -c $<

clean:
	rm $(PROGRAM) $(OBJECTS)


