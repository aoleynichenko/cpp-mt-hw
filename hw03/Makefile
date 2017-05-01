# Alexander Oleynichenko, 2017
# Makefile for the Homework 3 (Techosphera Project)
# External sorting

CC=gcc
CXX=g++
CCFLAGS=
CXXFLAGS=--std=c++11

all: genfile extsort

# External sorting program
extsort: main.cpp InputWay.cpp OutputWay.cpp
	$(CXX) $(CXXFLAGS) -o extsort main.cpp InputWay.cpp OutputWay.cpp -lm

# Test generator
genfile: genfile.c
	$(CC) $(CCFLAGS) genfile.c -o genfile

clean:
	rm genfile extsort

test:
	./genfile --sz 100
	./extsort unsorted extsorted 3
	diff sorted extsorted
	rm sorted unsorted extsorted

