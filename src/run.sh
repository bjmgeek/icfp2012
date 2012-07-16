#!/bin/sh
./lifter < $1 &
pid=$!

( sleep 150 ; kill -INT $pid; sleep 10; kill -KILL $pid ) 2>&1 > /dev/null &
wait $pid
