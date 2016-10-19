PBC=protoc
MV=mv

all:
	$(PBC) DeviceMessages.proto --cpp_out ../src/
	$(PBC) WebSocketProtocol.proto --cpp_out ../src/
	$(MV) ../src/DeviceMessages.pb.cc ../src/DeviceMessages.cpp
	$(MV) ../src/WebSocketProtocol.pb.cc ../src/WebSocketProtocol.cpp
