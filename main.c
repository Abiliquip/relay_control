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

#include "struct_def.h"
#include "ABE_ADCPi.h"
#include "init.h"
#include "control.h"
#include "actuator_control.h"




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
		//printf("mode 2\n");
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
