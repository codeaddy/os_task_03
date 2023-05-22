#!/bin/bash

gcc mark7_client.c -o client
gcc mark7_observer.c -o observer
gcc mark7_server.c -o server -lpthread

./server 127.0.0.1 8080 &

gnome-terminal -- $PWD/observer 127.0.0.1 8080 &

sleep 2

# Запустите клиентов
for i in {1..5}
do
   gnome-terminal -- $PWD/client 127.0.0.1 8080 &
   sleep 1
done

wait
