#!/bin/bash

gcc mark5_client.c -o client
gcc mark5_server.c -o server -lpthread

./server 127.0.0.1 8080 &

sleep 2

for i in {1..5}
do
   gnome-terminal -- $PWD/client 127.0.0.1 8080 &
   sleep 1
done

wait
