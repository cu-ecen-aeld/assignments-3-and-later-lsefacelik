# Makefile for server folder contents

SOURCE	= aesdsocket.c
OBJS= ${SOURCE:.c=.o}
HEADER	= 
OUT = aesdsocket
SRCS= ${HEADER} ${SOURCE}
CC	 =$(CROSS_COMPILE)gcc
FLAGS	 = -g -c -Wall -Werror
LFLAGS	 = 

all: aesdsocket

default: aesdsocket

aesdsocket: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

aesdsocket.o: aesdsocket.c
	$(CC) $(FLAGS) aesdsocket.c 


clean:
	rm -f $(OBJS) $(OUT)