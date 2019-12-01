OBJS	= main.o
SOURCE	= main.c
HEADER	= 
OUT	= opt
CC	 = gcc
FLAGS	 = -g -c -Wall 
LFLAGS	 = -lbcm2835 -lpigpio -lrt -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c 


clean:
	rm -f $(OBJS) $(OUT)