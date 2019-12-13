OBJS	= main.o ABE_ADCPi.o init.o control.o actuator_control.o current.o
SOURCE	= main.c ABE_ADCPi.c init.c control.c actuator_control.c current.c
HEADER	= ABE_ADCPi.h init.h control.h actuator_control.h struct_def.h current.h barcode.h
OUT	= opt
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lbcm2835 -lpigpio -lrt -lpthread -lgcc_s

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c 
	$(CC) $(FLAGS) main.c 


clean:
	rm -f $(OBJS) $(OUT)