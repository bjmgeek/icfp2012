#!/bin/sh
./lifter < $1 &
sleep 150
kill -INT $!
sleep 10
kill -KILL $!
