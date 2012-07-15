#!/bin/sh
./lifter < $1 &
pid=$!

( sleep 150 ; kill -INT $pid; sleep 10; kill -KILL $pid ) &
wait $pid
