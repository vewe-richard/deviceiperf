CC = gcc
objects = main.c server.c client.c
cmdobjs = command.c

all: deviceiperf command

deviceiperf: $(objects)
	$(CC) -g -pthread -o $@ $+

command: $(cmdobjs)
	$(CC) -g -pthread -o $@ $+

%.o:%.c
	$(CC) -c $+
