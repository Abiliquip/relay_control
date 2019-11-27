/* Program for testing actuators
 * Will be using microswitches for feedback at this stage
 * Uses seeed 4 relay board
 * adc pi, abelectronics.co.uk for voltage readings
 * Havnt got current baord yet
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
		buf[0] = 0x06;	//Address
        buf[1] = control;	//DATA
	}
	else if (state == 0){
		buf[0] = 0x06;     
        buf[1] = RELAYOFF; //LED OFF
	}
	bcm2835_i2c_write(buf,2);
}


int main(int argc, char **argv)  
{  

    if (!bcm2835_init())return 1;  
    bcm2835_i2c_begin();                //Start I2C operations.
    bcm2835_i2c_setSlaveAddress(0x20);  //I2C address
    bcm2835_i2c_set_baudrate(10000);    //1M baudrate
    int count = 0;
    while(1)  {   
		relay_control(1,count);
		delay(500);
		relay_control(0,count);
		delay(500);
		
		if (count == 4){
			count = 0;
		}
		count++;
    }    
    bcm2835_i2c_end();  
    bcm2835_close();  
    return 0;  
} 
