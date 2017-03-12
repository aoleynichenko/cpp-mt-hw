#!/bin/bash

./genfile --sz 20   # 20 numbers
# it generates extsorted and sorted files

./extsort unsorted extsorted 5
# use unsorted file, write sorted numbers to extsorted
# store only 5 numbers in fast memory

echo 'UNSORTED'
./firstn unsorted 20
echo 'EXTSORTED'
./firstn extsorted 20

