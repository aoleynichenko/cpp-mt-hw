# Alexander Oleynichenko, 2017
# Makefile for the Homework 3 (Techosphera Project)
# External sorting

CC=gcc
CXX=g++
CCFLAGS=
CXXFLAGS=--std=c++11

all: genfile extsort intsort

# External sorting
extsort: extsort.o data_type.o
	$(CXX) -o extsort extsort.o data_type.o -lm

extsort.o: extsort.cpp
	$(CXX) $(CXXFLAGS) -c extsort.cpp

# Internal sorting
intsort: intsort.o data_type.o
	$(CC) -o intsort intsort.o data_type.o

intsort.o: intsort.c
	$(CC) $(CCFLAGS) -c intsort.c

# Test generator
genfile: genfile.o data_type.o
	$(CC) -o genfile genfile.o data_type.o

genfile.o: genfile.c
	$(CC) $(CCFLAGS) -c genfile.c

clean:
	rm *.o genfile extsort intsort

test:
	./genfile --sz 50
	./extsort unsorted extsorted 10
	diff sorted extsorted
	rm sorted unsorted extsorted

