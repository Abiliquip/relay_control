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
#include <bcm2835.h> 
#include <pigpio.h>


#define RELAY1 0xfe
#define RELAY2 0xfb
#define RELAY3 0xfd
#define RELAY4 0xf7
#define RELAYOFF 0xff

#define relaycus1 12
#define relaycus2 16
#define relaycus3 18
#define relaycus4 22

#define mirco1inpin 29
#define mirco1outpin 31
#define mirco2inpin 33
#define mirco2outpin 35
#define mirco3inpin 32
#define mirco3outpin 36
#define mirco4inpin 38
#define mirco4outpin 40

struct inputmicroswitches{
    int micro1in = 0;
    int micro2in = 0;
    int micro3in = 0;
    int micro4in = 0;
    int micro1out = 0;
    int micro2out = 0;
    int micro3out = 0;
    int micro4out = 0;
}

void init(void){
    //i2c init
    bcm2835_init();
    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    bcm2835_i2c_set_baudrate(10000);    //1M baudrate
    
    //gpio init
    if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }

   /* Set GPIO modes */
   gpioSetMode(relaycus1, PI_OUTPUT);
   gpioSetMode(relaycus2, PI_OUTPUT);
   gpioSetMode(relaycus3, PI_OUTPUT);
   gpioSetMode(relaycus4, PI_OUTPUT);	
   //   gpioSetMode(23, PI_INPUT);

}

void deinit(void){
    //deinit
    bcm2835_i2c_end();  
    bcm2835_close();  
    
    /* Stop DMA, release resources */
    gpioTerminate();
}

int select_relay(int relay){
	if (relay == 1){
		return RELAY1;
	}
	else if (relay == 2){
		return RELAY2;
	}
	else if (relay == 3){
		return RELAY3;
	}
	else if (relay == 4){
		return RELAY4;
	}
	return RELAYOFF;
}


void relay_control(bool state, int relay){
	char buf[2];
	int control = select_relay(relay);
	
	if (state == 1){
		buf[0] = 0x06;		//Address
        buf[1] = control;	//DATA
	}
	else if (state == 0){
		buf[0] = 0x06;     
        buf[1] = RELAYOFF; //OFF
	}
	bcm2835_i2c_write(buf,2);
}

void position_feedback(){}
void act_control(int active_act, int direction){
    //gpioWrite(18, 1); /* on */
    if(direction == 1){
	relay_control(1,1);
		
    }
    else if(direction == 2){
		
		
    }
    else{
	relay_control(0, 1);
	relay_control(0, 2);
	relay_control(0, 3);
	relay_control(0, 4);
    }
}
int stateupdate(int state, inputmicroswitches micro){
    if (state == 0){ // just turned on/restarted, act 1 must be in
	if (mirco1in == 1){
	    return 1;
	}
	else{
	    return 0;
	}
    }
    else if (state == 1){ // act 1 moved out
	if (mirco1out == 1){
	    return 2;
	}
	else{
	    return 1;
	}
    }
    else if (state == 2){ // act 1 finished, at in position
	if (mirco1in == 1){
	    return 3;
	}
	else{
	    return 2;
	}
    }
    else if (state == 3){ // act 2 starting
	if (mirco2out == 1){
	    return 4;
	}
	else{
	    return 3;
	}
    }
    else if (state == 4){ // act 2 finished, at in position
	if (mirco2in == 1){
	    return 5;
	}
	else{
	    return 4;
	}
    }
    else if (state == 5){ // act 3 starting
	if (mirco3out == 1){
	    return 6;
	}
	else{
	    return 5;
	}
    }
    else if (state == 6){ // act 3 finished, at in position
	if (mirco3in == 1){
	    return 7;
	}
	else{
	    return 6;
	}
    }
    else if (state == 7){ // act 4 starting
	if (mirco4out == 1){
	    return 8;
	}
	else{
	    return 7;
	}
    }
    else if (state == 8){ // act 3 finished, at in position
	if (mirco4in == 1){
	    return 0;
	}
	else{
	    return 8;
	}
    }
    return state;
}
inputmicroswitches updatemicro(inputmicroswitches micro){
    gpioRead(23)
    
}


int main(int argc, char **argv)  {  
    init();

    int count = 0;
    int active_act = 1;
    int direction = 0;
    int state = 0;
    struct inputmicroswitches micro;
    while(1){
	micro = updatemicro(micro);
	state = stateupdate(state, micro);
	    
	position_feedback();
	act_control(active_act, direction);
		
	relay_control(1,count);
	delay(500);
	relay_control(0,count);
	delay(500);
		
	if (count == 4){
		count = 0;
	}
	count++;
    }    
    deinit();
    return 0;  
} 
