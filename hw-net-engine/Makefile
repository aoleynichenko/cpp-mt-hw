TEST_FILES = ../thirdparty/gtest/gtest-all.cc ../thirdparty/gtest/gtest_main.cc
DEPS_FILES = ../thirdparty/backward-cpp-1.3/backward.cpp
HDR = include/coroutine/engine.h include/coroutine/net-engine.h include/coroutine/channel.h
SRC = src/engine.cpp src/net-engine.cpp src/channel.cpp test/engine_test.cpp

all: tests.done

tests.done: engine_test
	./engine_test && touch tests.done

engine_test: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o engine_test $(SRC) $(DEPS_FILES) $(TEST_FILES) -Iinclude -I../thirdparty -lpthread -lbfd

clean:
	rm engine_test
