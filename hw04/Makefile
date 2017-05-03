CXX=g++
CXXFLAGS=--std=c++11

all: chat-server

chat-server: chat-server.cpp Client.cpp ChatException.cpp
	$(CXX) $(CXXFLAGS) -o chat-server chat-server.cpp Client.cpp ChatException.cpp

clean:
	rm chat-server
