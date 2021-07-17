#!/bin/bash
gcc -pthread -o client client.c
sudo tc qdisc del dev lo root
echo "Enter 1 to run program against netem Network delay."
echo "Enter 2 to run program against netem Network packet loss."
echo "Enter 3 to run program against netem Network packet reordering."
echo "Enter 4 to run program against netem Network packet corruption."
read choice
if  [ $choice -eq 1 ]
then
	sudo tc qdisc del dev lo root
	sudo tc qdisc add dev lo root netem delay 1ms
	for i in {1..15}
	do
		sudo tc qdisc change dev lo root netem delay $((i*100))ms		 
		./client logo.gif new_logo.gif 127.0.0.1 5005 5000
		sleep 5
	done
	python3 delay.py

elif [ $choice -eq 2 ]
then
	sudo tc qdisc del dev lo root
	sudo tc qdisc add dev lo root netem loss 1%
	for i in {1..15}
	do
		sudo tc qdisc change dev lo root netem loss $((i*5))%	 
		./client logo.gif new_logo.gif 127.0.0.1 5005 5000
		sleep 5
	done
	python3 loss.py

elif [ $choice -eq 3 ]
then
	sudo tc qdisc del dev lo root
	sudo tc qdisc add dev lo root netem delay 40ms reorder 25%
	for i in {1..15}
	do
		sudo tc qdisc change dev lo root netem delay 40ms reorder $((i*5))%	 
		./client logo.gif new_logo.gif 127.0.0.1 5005 5000
		sleep 5
	done
	python3 reorder.py

elif [ $choice -eq 4 ]
then
	sudo tc qdisc del dev lo root
	sudo tc qdisc add dev lo root netem corrupt 0.1%
	for i in {1..15}
	do
		sudo tc qdisc change dev lo root netem corrupt $((i*5))% 
		./client logo.gif new_logo.gif 127.0.0.1 5005 5000
		sleep 5
	done
	python3 corrupt.py
fi







