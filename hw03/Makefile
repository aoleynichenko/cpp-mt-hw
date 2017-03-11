# Alexander Oleynichenko, 2017
# Makefile for the Homework 3 (Techosphera Project)
# External sorting

CC=gcc
CCFLAGS=

TARGET_SORT=extsort
SRC_SORT=extsort.c data_type.c
OBJ_SORT=extsort.o data_type.o

TARGET_GEN=genfile
SRC_GEN=genfile.c data_type.c
OBJ_GEN=genfile.o data_type.o


all: $(TARGET_SORT) $(TARGET_GEN)

# External sorting
$(TARGET_SORT): $(OBJ_SORT)
	$(CC) -o $(TARGET_SORT) $(OBJ_SORT)

$(OBJ_SORT):
	$(CC) $(CCFLAGS) -c $(SRC_SORT)

# Test generator
$(TARGET_GEN): $(OBJ_GEN)
	$(CC) -o $(TARGET_GEN) $(OBJ_GEN)

$(OBJ_GEN):
	$(CC) $(CCFLAGS) -c $(SRC_GEN)

clean:
	rm *.o $(TARGET_GEN) $(TARGET_SORT)

