#!/bin/bash

if [ $(id -u) -eq 0 ]; then
	echo "Deleting previous log files";
	rm -rf logs
	mkdir logs
    echo "Deleting previous compiles";
    rm -rf client e server
    echo "Compiling ...";
	gcc -W -Wall -lm handlers.c client.c -o client
	gcc -W -Wall -lm handlers.c emulator.c -o e -L. -lrequests -lcurl
	gcc -W -Wall -lm handlers.c server.c -o server
	echo "Finished";
else
	echo "Only root may run this script"
	exit 1
fi
