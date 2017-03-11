# Alexander Oleynichenko, 2017
# Makefile for the Homework 3 (Techosphera Project)
# External sorting

CC=gcc
CCFLAGS=-g

all: genfile extsort intsort

# External sorting
extsort: extsort.o data_type.o
	$(CC) -o extsort extsort.o data_type.o

extsort.o: extsort.c
	$(CC) $(CCFLAGS) -c extsort.c

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
