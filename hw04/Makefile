CXX=g++
CXXFLAGS=--std=c++11

all: chat-server

chat-server: chat-server.cpp Client.cpp
	$(CXX) $(CXXFLAGS) -o chat-server chat-server.cpp Client.cpp

clean:
	rm chat-server
