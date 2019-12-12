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

//#include "struct_def.h"
#include "init.h"
#include "control.h"
#include "actuator_control.h"
#include "current.h"



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


typedef struct barcode_s{
    int barcode_1;
    int barcode_2;
    int barcode_3;
    int barcode_4;
}barcode_tag;

barcode_tag init_barcode(){
    barcode_tag barcode;
    barcode.barcode_1 = 0;
    barcode.barcode_2 = 0;
    barcode.barcode_3 = 0;
    barcode.barcode_4 = 0;
    return barcode;
}

barcode_tag bar_code_check(int mode){
    barcode_tag barcode = init_barcode();
    int confermation = 0;
    
    
    printf("Would you like to input a serial number?\n");
    while(confermation == 0){
	printf("Enter single digit\n\n");
	scanf("%d", &mode);
	printf("Was your number %d? \nEnter 1 for yes, 0 for no\n", mode);
	scanf("%d", &confermation);
    } 
    
    if (mode == 2){
	barcode.barcode_1 = 2;
	barcode.barcode_2 = 2;
	barcode.barcode_3 = 2;
	barcode.barcode_4 = 2;
    }
    else if(mode == 3){
	barcode.barcode_1 = 3;
	barcode.barcode_2 = 3;
	barcode.barcode_3 = 3;
	barcode.barcode_4 = 3;
    }
    return barcode;
}

void display_barcode(barcode_tag barcode){
    printf("%d\n", barcode.barcode_1);
    printf("%d\n", barcode.barcode_2);
    printf("%d\n", barcode.barcode_3);
    printf("%d\n", barcode.barcode_4);
}

/* Main loop :) */
int main(int argc, char **argv)  {  
    deinit();
    
    //Initialstion for device and variables
    init();
    int state4 = 0;
    bool state_change4 = 0;
    int old_state4 = 0;
    int state1 = 0;
    bool state_change1 = 0;
    int old_state1 = 0;
    struct inputmicroswitches micro;
    micro = initmicrostrut();
    micro = updatemicro(micro);
    int mode = select_mode();
    mode = check_all_home(mode, micro);
    int estop = 0;
    int count = 0;
    float avg_cur = 3;
    float prev_avg_cur = 0;
    barcode_tag barcode;
    
    //Check bar code
    barcode = bar_code_check(mode);
    record_barcode();
    display_barcode(barcode);
    
    while(mode != 0){
	delay(100);
	micro = updatemicro(micro);
	
	estop = gpioRead(estop_pin);
	if (estop == 1){
	    turn_all_off();
	    printf("ESTOP engaded\n" );
	}
	else if (estop == 0){
	    if (mode == 1){ 						//send actuators home
		mode = sendallhome(micro);
	    }
	    else if (mode == 2){ 					//run four actuators
		old_state4 = state4; 					//Updates old state
		state4 = stateupdate(state4, micro, mode); 		//checks if new state is needed
		state_change4 = check_state_change(state4, old_state4); //if state is changed needs delay
		act_control(state4); 					//Function to move actuator
		count = count_display(count, state4, state_change4); 	//update cycle count
		state_change_delay(state_change4, state4, mode); 	//delay at the end of movement
		avg_cur = update_current(avg_cur, prev_avg_cur); 	//get new current value
		mode = check_current(avg_cur, mode);			//check current is large
		prev_avg_cur = avg_cur; 				//used for moving average calc
		printf("current = %.2f\n", avg_cur);
	    }
	    else if (mode == 3){ 					// run one actuator
		old_state1 = state1;
		state1 = stateupdate(state1, micro, mode);
		state_change1 = check_state_change(state1, old_state1);
		act_control(state1);
		count = count_display(count, state1, state_change1);
		state_change_delay(state_change1, state1, mode);
		avg_cur = update_current(avg_cur, prev_avg_cur);
		mode = check_current(avg_cur, mode);
		prev_avg_cur = avg_cur;
		printf("current = %.2f\n", avg_cur);
	    }
	}
    }    
    
    
    turn_all_off();
    deinit();
    printf("\nThe program has ended\n\n");
    return 0;  
} 
