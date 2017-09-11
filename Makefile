#Gregory Macri
#Makefile for knish shell

CFILES= knish.c  
OFILES=$(CFILES:.c=.o)
CC=gcc -g -pthread -lreadline -lncurses -Wall

knish:	$(OFILES)
	$(CC) -o knish $(OFILES) 

clean::
	/bin/rm -f knish $(OFILES)
