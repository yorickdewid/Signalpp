RANLIB=ranlib
LIBSRC=CryptoProvider.cpp Base64.cpp AccountManager.cpp ProvisioningCipher.cpp Api.cpp Websocket.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

CXXFLAGS=-Wall -g -O0 -std=c++11 -I../libsignal-protocol-c/src/ -DENABLE_LOGGING -Wno-unused-function -Wno-unused-variable
INCLUDE=
LIBES=

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
