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
#include <bcm2835.h>  // i2c board
#include <pigpio.h> // when using pi gpio
#include "ABE_ADCPi.h"
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port

/* i2c relay board addresses */
#define RELAY1 0xfe // relay off only used
#define RELAY2 0xfb
#define RELAY3 0xfd
#define RELAY4 0xf7
#define RELAYOFF 0xff

/* Relays for controling which actuator is used */
#define relaycus1 4
#define relaycus2 17
#define relaycus3 27
#define relaycus4 22

/* GPIO used for microswitches */
#define mirco1inpin 10
#define mirco1outpin 9
#define mirco2inpin 11
#define mirco2outpin 14
#define mirco3inpin 15
#define mirco3outpin 18
#define mirco4inpin 23
#define mirco4outpin 24

/*  GPIO used for extenral switches */
#define estop_pin 25

/* delays for program */
#define BETWEEN_ACT_DELAY 500
#define FOUR_MOTOR_DUTY_DELAY 3000

/* Standard cycle testing constants */

#define STD_CYCLES 10
#define STD_PERCENTAGE 120

/* A structure to hold the micro switch data */
struct inputmicroswitches{
    int micro1in;
    int micro2in;
    int micro3in;
    int micro4in;
    int micro1out;
    int micro2out;
    int micro3out;
    int micro4out;
};

/* Initail micro switch setting function */
struct inputmicroswitches initmicrostrut(void){
    struct inputmicroswitches micro;
    micro.micro1in = 0;
    micro.micro2in = 0;
    micro.micro3in = 0;
    micro.micro4in = 0;
    micro.micro1out = 0;
    micro.micro2out = 0;
    micro.micro3out = 0;
    micro.micro4out = 0;
    return micro;
}

/*  Initializeation function - Run at start of main */
int init(void){
    //i2c init
    //bcm2835_init();
    //bcm2835_i2c_begin();                //Start I2C operations.
    //bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    //bcm2835_i2c_set_baudrate(10000);    //1M baudrate
    
    //gpio init
    if (gpioInitialise() < 0){
	fprintf(stderr, "pigpio initialisation failed\n");
	return 1;
    }

    /* Set GPIO modes 
    * These are for selecting a singal actuator */
    gpioSetMode(relaycus1, PI_OUTPUT);
    gpioSetMode(relaycus2, PI_OUTPUT);
    gpioSetMode(relaycus3, PI_OUTPUT);
    gpioSetMode(relaycus4, PI_OUTPUT);	
    
    /* GPIO micro switches */
    gpioSetMode(mirco1inpin, PI_INPUT);
    gpioSetMode(mirco1outpin, PI_INPUT);
    gpioSetMode(mirco2inpin, PI_INPUT);
    gpioSetMode(mirco2outpin, PI_INPUT);
    gpioSetMode(mirco3inpin, PI_INPUT);
    gpioSetMode(mirco3outpin, PI_INPUT);
    gpioSetMode(mirco4inpin, PI_INPUT);
    gpioSetMode(mirco4outpin, PI_INPUT);
    gpioSetMode(estop_pin, PI_INPUT);
    
    /* Set internal pull up resister */
    gpioSetPullUpDown(mirco1inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco1outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco2inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco2outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco3inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco3outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco4inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco4outpin, PI_PUD_UP);
    gpioSetPullUpDown(estop_pin, PI_PUD_UP);
    return 0;
}

/* run at the end of main loop */
void deinit(void){
    //deinit
    //bcm2835_i2c_end();  
    //bcm2835_close();  
    
    /* Stop DMA, release resources */
    gpioTerminate();
}

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

    /*char buf[2];
    buf[0] = 0x06; 
    
    if(direction == 1){ //send out
        buf[1] = 0xF6;	//DATA
    }
    else if(direction == 2){ // send in
        buf[1] = 0xF9;	//DATA
    }
    else{
	buf[1] = RELAYOFF; //OFF
    }*/
    /*bcm2835_init();
    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    bcm2835_i2c_set_baudrate(10000); 
    bcm2835_i2c_write(buf,2);
    bcm2835_i2c_end();  
    bcm2835_close();  */
}

/* This function selects the actuator to run */
void select_act(int actuator){
    if (actuator == 0){
	gpioWrite(relaycus1, 0);
	gpioWrite(relaycus2, 0);
	gpioWrite(relaycus3, 0);
	gpioWrite(relaycus4, 0);
    }
    else if (actuator == 1){
	gpioWrite(relaycus1, 1);
	gpioWrite(relaycus2, 0);
	gpioWrite(relaycus3, 0);
	gpioWrite(relaycus4, 0);
    }
    else if (actuator == 2){
	gpioWrite(relaycus1, 0);
	gpioWrite(relaycus2, 1);
	gpioWrite(relaycus3, 0);
	gpioWrite(relaycus4, 0);
    }
    else if (actuator == 3){
	gpioWrite(relaycus1, 0);
	gpioWrite(relaycus2, 0);
	gpioWrite(relaycus3, 1);
	gpioWrite(relaycus4, 0);
    }
    else if (actuator == 4){
	gpioWrite(relaycus1, 0);
	gpioWrite(relaycus2, 0);
	gpioWrite(relaycus3, 0);
	gpioWrite(relaycus4, 1);
    }
}

/* Simple function to turn all actuators */
void turn_all_off(void){
    hb_control(0);
    select_act(0);
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

/* Takes micro switch data and previous state to deturmine the next state */
int stateupdate(int previous_state, struct inputmicroswitches micro){
    if (previous_state == 0){ // just turned on/restarted, act 1 must be in
	if (micro.micro1in == 0){ // 0 is fully in
	    return 1;
	}
	else{
	    return 0;
	}
    }
    else if (previous_state == 1){ // act 1 moved out
	if (micro.micro1out == 1){
	    return 2;
	}
	else{
	    return 1;
	}
    }
    else if (previous_state == 2){ // act 1 finished, at in position
	if (micro.micro1in == 0){
	    return 3;
	}
	else{
	    return 2;
	}
    }
    else if (previous_state == 3){ // act 2 starting
	if (micro.micro2out == 1){
	    return 4;
	}
	else{
	    return 3;
	}
    }
    else if (previous_state == 4){ // act 2 finished, at in position
	if (micro.micro2in == 0){
	    return 5;
	}
	else{
	    return 4;
	}
    }
    else if (previous_state == 5){ // act 3 starting
	if (micro.micro3out == 1){
	    return 6;
	}
	else{
	    return 5;
	}
    }
    else if (previous_state == 6){ // act 3 finished, at in position
	if (micro.micro3in == 0){
	    return 7;
	}
	else{
	    return 6;
	}
    }
    else if (previous_state == 7){ // act 4 starting
	if (micro.micro4out == 1){
	    return 8;
	}
	else{
	    return 7;
	}
    }
    else if (previous_state == 8){ // act 3 finished, at in position
	if (micro.micro4in == 0){
	    return 0;
	}
	else{
	    return 8;
	}
    }
    return previous_state;
}

/* Checks if the state has changed. Mainly used to add delay at the end of actuator stroke */
bool check_state_change(int state, int previous_state){
    if (previous_state != state){
	printf("state change\n");
	return 1;
    }
    else {
	return 0;
    }
    return 0;
}

/* If state flag has changed then delay actuator
 * Delay the last actuator to account for duty cycle of actuators */
void state_change_delay(bool state_change, int state){
    if (state_change == 1){
	turn_all_off();
	delay(BETWEEN_ACT_DELAY);
    }
    if (state_change == 1 && state == 0){
	turn_all_off();
	delay(FOUR_MOTOR_DUTY_DELAY);
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
    
    //setvbuf (stdout, NULL, _IONBF, 0); // needed to print to the command line
    delay(100);
    printf("Pin 1: %G \n", read_voltage(0x68,1, 18, 1, 1)); // read from adc chip 1, channel 1, 18 bit, pga gain set to 1 and continuous conversion mode
    delay(100);
    
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
