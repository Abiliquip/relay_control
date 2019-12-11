OBJS	= main.o ABE_ADCPi.o
SOURCE	= main.c ABE_ADCPi.c
HEADER	= ABE_ADCPi.h
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