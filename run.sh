#!/bin/sh
./lifter < $1 2>/dev/null &
pid=$!
sleep 150
echo 150 seconds elapsed, sending SIGINT
kill -INT $pid
sleep 10
echo 160 seconds elapsed, sending SIGINT
kill -KILL $pid
exit
