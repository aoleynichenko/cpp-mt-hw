#!/bin/sh
./genfile --sz 10240 --nosorted --unsorted unsorted10GB
./extsort unsorted10GB extsorted 2048
