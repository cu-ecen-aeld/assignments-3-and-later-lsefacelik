OBJS	= writer.o
SOURCE	= writer.c
HEADER	= 
OUT	= writer
CC	 =$(CROSS_COMPILE)gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

writer.o: writer.c
	$(CC) $(FLAGS) writer.c 


clean:
	rm -f $(OBJS) $(OUT)