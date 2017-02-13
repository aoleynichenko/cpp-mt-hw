TEST_FILES = ../thirdparty/gtest/gtest-all.cc ../thirdparty/gtest/gtest_main.cc
SRC = skiplist_test.cpp
HDR = skiplist.h


all: tests.done

skiplist_test: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o skiplist_test $(SRC) -I../thirdparty $(TEST_FILES) -lpthread

tests.done: skiplist_test
	./skiplist_test
	touch tests.done
