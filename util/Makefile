BIN=signalclient
INCLUDE=-I../include
LIBS=-lleveldb
CXXFLAGS=-g -O0 -Wall -std=c++11

all:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) signalclient.cpp -o $(BIN)

clean:
	$(RM) -rf $(BIN)
