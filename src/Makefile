RANLIB=ranlib
LIBSRC=AccountManager.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

CXXFLAGS=-Wall -g -O0 -std=c++11
LOADLIBES=-L./

TSECLIB=libtsec.a
TARGETS=$(TSECLIB)

all: $(TARGETS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGETS): $(LIBOBJ)
	ar rcs $(TSECLIB) $(LIBOBJ)
	ranlib $(TSECLIB)

clean:
	$(RM) -rf *.o $(TARGETS) $(TSECLIB)
