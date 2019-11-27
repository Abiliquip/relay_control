/* Program for testing actuators
 * Will be using microswitches for feedback at this stage
 * Uses seeed 4 relay board
 * adc pi, abelectronics.co.uk for voltage readings
 * Havnt got current baord yet
 * 
 * Authour: Ryan Taylor
 * 
 * */

#include <stdio.h>
#include <stdbool.h>
#include <bcm2835.h> 

#define RELAY1 0xfe
#define RELAY2 0xfb
#define RELAY3 0xfd
#define RELAY4 0xf7
#define RELAYOFF 0xff

void init(void){
	bcm2835_init();
    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    bcm2835_i2c_set_baudrate(10000);    //1M baudrate
	
}

void deinit(void){
	bcm2835_i2c_end();  
    bcm2835_close();  
	
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


int main(int argc, char **argv)  {  
	init();

    int count = 0;
    int active_act = 1;
    int direction = 0;
    while(1)  {   
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
