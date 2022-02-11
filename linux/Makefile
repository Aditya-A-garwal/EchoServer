
SRC_PATH := src
BIN_PATH := build

all: client server

client: $(SRC_PATH)/client_cpp.cpp $(SRC_PATH)/socket_utils.hpp $(SRC_PATH)/socket_utils.cpp
	g++ $(SRC_PATH)/client_cpp.cpp $(SRC_PATH)/socket_utils.cpp -o $(BIN_PATH)/client

server: $(SRC_PATH)/server_cpp.cpp $(SRC_PATH)/socket_utils.hpp $(SRC_PATH)/socket_utils.cpp
	g++ $(SRC_PATH)/server_cpp.cpp $(SRC_PATH)/socket_utils.cpp -o $(BIN_PATH)/server

clean:
	rm $(BIN_PATH)/client
	rm $(BIN_PATH)/server
