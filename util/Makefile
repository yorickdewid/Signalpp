BIN=signalclient
INCLUDE=-I../include
LIBS=-lleveldb -lcrypto -lcurl
CXXFLAGS=-g -O0 -Wall -std=c++11 -I../libsignal-protocol-c/src/ -DENABLE_LOGGING -Wno-unused-function -Wno-unused-variable

all:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) signalclient.cpp ../src/libtsec.a ../libsignal-protocol-c/build/src/libsignal-protocol-c.a -o $(BIN)

clean:
	$(RM) -rf $(BIN)
