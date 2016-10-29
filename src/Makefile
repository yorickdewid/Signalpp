RANLIB=ranlib
LIBSRC=CryptoProvider.cpp Base64.cpp KeyHelper.cpp AccountManager.cpp MessageReceiver.cpp ProvisioningCipher.cpp Api.cpp Websocket.cpp WebSocketProtocol.cpp DeviceMessages.cpp
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

cleanall: clean
	$(RM) -rf *pb.h WebSocketProtocol.cpp DeviceMessages.cpp
