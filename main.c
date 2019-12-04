/* Program for testing actuators
 * Will be using microswitches for feedback at this stage
 * Uses seeed 4 relay board
 * adc pi, abelectronics.co.uk for voltage readings
 * Havnt got current board yet
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
    bcm2835_init();
    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    bcm2835_i2c_set_baudrate(10000);    //1M baudrate
    
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
    
    gpioSetPullUpDown(mirco1inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco1outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco2inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco2outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco3inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco3outpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco4inpin, PI_PUD_UP);
    gpioSetPullUpDown(mirco4outpin, PI_PUD_UP);
    return 0;
}

/* run at the end of main loop */
void deinit(void){
    //deinit
    bcm2835_i2c_end();  
    bcm2835_close();  
    
    /* Stop DMA, release resources */
    gpioTerminate();
}


/* direction 1 is send out, direction 2 is send in 
 * Function to control the sequense of the H bridge, Uses the i2c board as relays*/
void hb_control(int direction){
    char buf[2];
    //int control = select_relay(relay);
    buf[0] = 0x06; 
    
    if(direction == 1){ //send in
        buf[1] = 0xF9;		//DATA
	
    }
    else if(direction == 2){ // send out
        buf[1] = 0xF6;		//DATA
    }
    else{
	buf[1] = RELAYOFF; //OFF
    }
    bcm2835_i2c_write(buf,2);
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
    else if (state == 6){ //actuator four out
	hb_control(1);
	select_act(4);
    }
    else if (state == 6){ //actuator four in
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

int main(int argc, char **argv)  {  
    
    init();
    int state = 0;
    struct inputmicroswitches micro;
    micro = initmicrostrut();
    //	int i = 0;
    //select_act(1);
    while(1){
	micro = updatemicro(micro);
	/*select_act(i);
	printf("%d\n", i);
	delay(3000);
	
	if (i ==4){
	    i = 0;
	}
	i++;*/
	delay(500);
	state = stateupdate(state, micro);
	printf("state = %d\n", state);
	//act_control(state);
	printf("%d%d%d%d%d%d%d%d\n", micro.micro1in, micro.micro1out, micro.micro2in, micro.micro2out, micro.micro3in, micro.micro3out, micro.micro4in, micro.micro4out);
	/*delay(3000);
	hb_control(2);
	printf("2\n");
	delay(3000);
	hb_control(1);
	printf("1\n");*/
	
	//act_control(state);
		
    }    
    
    
    
    deinit();
    return 0;  
} 
