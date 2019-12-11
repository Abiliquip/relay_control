/* Program for testing actuators
 * Will be using microswitches for feedback at this stage
 * Uses seeed 4 relay board
 * adc pi, abelectronics.co.uk for voltage readings
 * Havnt got current board programed
 * 
 * Authour: Ryan Taylor
 * Date: 2/12/19
 * Company: Abiliquip
 * 
 * */

#include <stdio.h>
#include <stdbool.h>
#include <bcm2835.h>  // delay function
#include <pigpio.h> // when using pi gpio
#include <unistd.h>		//Needed for I2C port
#include <fcntl.h>		//Needed for I2C port
#include <sys/ioctl.h>		//Needed for I2C port
#include <linux/i2c-dev.h>	//Needed for I2C port

#include "ABE_ADCPi.h"
#include "init.h"
#include "control.h"
#include "actuator_control.h"

/* i2c relay board addresses */
#define RELAY1 0xfe // relay off only used
#define RELAY2 0xfb
#define RELAY3 0xfd
#define RELAY4 0xf7
#define RELAYOFF 0xff



/* Standard cycle testing constants */

#define STD_CYCLES 10
#define STD_PERCENTAGE 120

int file_i2c;
int length;
char buffer[2];

/* direction 1 is send out, direction 2 is send in 
 * Function to control the sequense of the H bridge, Uses the i2c board as relays*/
void hb_control(int direction){
    //----- OPEN THE I2C BUS -----
    char *filename = (char*)"/dev/i2c-1";
    if ((file_i2c = open(filename, O_RDWR)) < 0){
	    //ERROR HANDLING: you can check errno to see what went wrong
	    printf("Failed to open the i2c bus");
	    return;
    }
	    
    int addr = 0x20;          //<<<<<The I2C address of the slave
    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0){
	    printf("Failed to acquire bus access and/or talk to slave.\n");
	    //ERROR HANDLING; you can check errno to see what went wrong
	    return;
    }
	    
    //----- WRITE BYTES -----
    buffer[0] = 0x06;
    if(direction == 1){ //send out
        buffer[1] = 0xF6;	//DATA
    }
    else if(direction == 2){ // send in
        buffer[1] = 0xF9;	//DATA
    }
    else{
	buffer[1] = RELAYOFF; //OFF
    }
    length = 2;			//<<< Number of bytes to write
    if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
    {
	    /* ERROR HANDLING: i2c transaction failed */
	    printf("Failed to write to the i2c bus.\n");
    }
}



/* Takes in state data and decideds what to do or not do */
void act_control(int state){
    if(state == 0){
	hb_control(0);
	select_act(0);
    }
    else if (state == 1){ //actuator one out
	hb_control(1);
	select_act(1);
    }
    else if (state == 2){ //actuator one in
	hb_control(2);
	select_act(1);
    }
    else if (state == 3){ //actuator two out
	hb_control(1);
	select_act(2);
    }
    else if (state == 4){ //actuator two in
	hb_control(2);
	select_act(2);
    }
    else if (state == 5){ //actuator three out
	hb_control(1);
	select_act(3);
    }
    else if (state == 6){ //actuator three in
	hb_control(2);
	select_act(3);
    }
    else if (state == 7){ //actuator four out
	hb_control(1);
	select_act(4);
    }
    else if (state == 8){ //actuator four in
	hb_control(2);
	select_act(4);
    }
    else{
	hb_control(0);
	select_act(0);
    }
}




/* Read the micro switch gpio */
struct inputmicroswitches updatemicro(struct inputmicroswitches micro){
    micro.micro1in = gpioRead(mirco1inpin);
    micro.micro1out = gpioRead(mirco1outpin);
    micro.micro2in = gpioRead(mirco2inpin);
    micro.micro2out = gpioRead(mirco2outpin);
    micro.micro3in = gpioRead(mirco3inpin);
    micro.micro3out = gpioRead(mirco3outpin);
    micro.micro4in = gpioRead(mirco4inpin);
    micro.micro4out = gpioRead(mirco4outpin);
    return micro;
}

/* Function to get all the actuators in the in position */
int sendallhome(struct inputmicroswitches micro){
    if (micro.micro1in == 1){
	printf("Sending actuator 1 home\n");
	hb_control(2);
	select_act(1);
    }
    else if (micro.micro2in == 1){
	printf("Sending actuator 2 home\n");
	hb_control(2);
	select_act(2);
    }
    else if (micro.micro3in == 1){
	printf("Sending actuator 3 home\n");
	hb_control(2);
	select_act(3);
    }
    else if (micro.micro4in == 1){
	printf("Sending actuator 4 home\n");
	hb_control(2);
	select_act(4);
    }
    else{
	turn_all_off();
    }
    if (micro.micro1in == 0 && micro.micro2in == 0 && micro.micro3in == 0 && micro.micro4in == 0){
	printf("\nAll actuators are home\nEnding program\n\n");
	return 0;
    }
    return 1;
}

/* Used to start out program. Prints program option */
int select_mode(void){
    int mode = 0;
    int confermation = 0;

    printf("\n");
    printf("Please select mode\n");
    printf("Mode 1: Return actuators home \n");
    printf("Mode 2: Standard four actuator cycle, %d times, %d%% capacity, standard duty\n", STD_CYCLES, STD_PERCENTAGE);
    printf("Mode 3: Actuator 1 repeats, standard duty\n");
    while(confermation == 0){
	printf("Enter single digit\n\n");
	scanf("%d", &mode);
	printf("Was your number %d? \nEnter 1 for yes, 0 for no\n", mode);
	scanf("%d", &confermation);
    }
    printf("mode = %d\n", mode);
    return mode;
}

/* Function that will run at the start of the program to check if all the actuators are fully in
 * Ends the program after finishing */
int check_all_home(int mode, struct inputmicroswitches micro){
    int home = 0;
    
    if (mode == 2 || mode == 3){
	if (micro.micro1in == 1 || micro.micro2in == 1 || micro.micro3in == 1 || micro.micro4in == 1){
	    while (home == 0){
		printf("\nAn actuator is not home, All actuators need to be home to start\n"); 
		printf("Would you like to start homing process?\n");
		printf("Enter 1 for yes, 0 for no\n");
		scanf("%d", &home);
		if (home == 0){
		    return 0;
		}
	    }
	}
    }
    if (home == 1){
	printf("Moved to mode %d\n", 1);
	return 1;
    }
    return mode;
}

/* Displays the count of the cycling test. It will display at the end of the cycle */
int count_display(int count, int state, bool state_change){
    if(state_change == 1 && state == 0){
	count = count + 1;
	printf("Count for all four actuators is %d\n", count);
    }
    return count;
}

/* Main loop :) */
int main(int argc, char **argv)  {  
    init();
    int state = 0;
    bool state_change = 0;
    int old_state = 0;
    struct inputmicroswitches micro;
    micro = initmicrostrut();
    micro = updatemicro(micro);
    int mode = select_mode();
    mode = check_all_home(mode, micro);
    int estop = 0;
    int count = 0;
    
    while(mode != 0){
	delay(200);
	micro = updatemicro(micro);
	old_state = state;
	state = stateupdate(state, micro);
	state_change = check_state_change(state, old_state);
	estop = gpioRead(estop_pin);
	if (estop == 1){
	    turn_all_off();
	    printf("estop engaded\n" );
	}
	else if (estop == 0){
	    
	    if (mode == 1){ //send actuators home
		mode = sendallhome(micro);
	    }
	    if (mode == 2){
		act_control(state);
		count = count_display(count, state, state_change);
		state_change_delay(state_change, state);
		printf("mode 2\n");
		delay(100);
		printf("Pin 1: %G \n", read_voltage(0x68,1, 18, 1, 1)); // read from adc chip 1, channel 1, 18 bit, pga gain set to 1 and continuous conversion mode
		delay(100);
	    }
	    if (mode == 3){
		turn_all_off();
	    }
	    
	}
    }    
    
    
    
    deinit();
    return 0;  
} 
