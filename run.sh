#!/bin/sh

function trapped {
	echo got SIGCHLD $0
	exit
}


trap 'trapped' SIGCHLD

./lifter < $1 &
pid=$!
sleep 150
echo 150 seconds elapsed, sending SIGINT
kill -INT $pid
sleep 10
echo 160 seconds elapsed, sending SIGINT
kill -KILL $pid
exit
