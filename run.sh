#!/bin/sh
./lifter < $1 2>/dev/null &
pid=$!
sleep 150
kill -INT $pid
sleep 10
kill -KILL $pid
