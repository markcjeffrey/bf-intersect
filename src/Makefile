TARGET = sigintersect
CXX = g++
CCC = gcc
PROFILEFLAGS =#-fprofile-arcs -ftest-coverage
DEBUG=-g -Wall -D DEBUG
CXXFLAGS =$(DEBUG) $(RELEASE) $(PROFILEFLAGS)
LINKFLAGS =$(PROFILEFLAGS) $(RELEASE)


OBJECTS = sigintersect.o \
          in_parameters.o \
          signature.o \
          signature_batched.o \
          signature_unpartition.o \
          bitvector.o \
          pbxhash.o \
          hashzebchuk.o \
          hashpbxfullwin.o \

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LINKFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CCC) $(CXXFLAGS) -c $< -o $@

release:
	$(MAKE) $(MAKEFILE) DEBUG="" \
	RELEASE="-Wall -O3 -funroll-loops -mmmx"

clean:
	rm -rf *.o *.*~ Makefile~ $(TARGET)

