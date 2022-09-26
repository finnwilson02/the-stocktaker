#include "derivative.h"
#include <math.h> 
#include "servo.h"

#define ZERO_ELEVATION_DUTY 4600
#define ZERO_AZIMUTH_DUTY 2000

int servo_toggle;


void PWMinitialise(void){
    // set PP5 and PP7 for pwm output 
    PWMCLK = 0; // select clock A
    PWMPOL = 0xA0; // PWM5 and PWM7 output start high
    PWMCTL = 0xC0; // 16-bit PWM: use PWM4 and PWM5 for PWM5, PWM6 and PWM7 for PWM7
    PWMCAE = 0; // Center aligned
    PWMPRCLK = 0x33; // PWM Clock prescaler to 8 

    // set the PWM period appropriate for servos
    PWMPER45 = 0xEA6A;
    PWMPER67 = 0xEA6A;

    // set the initial duty cycle for both servos
    PWMDTY45 = ZERO_ELEVATION_DUTY;
    PWMDTY67 = ZERO_AZIMUTH_DUTY;
    
    PWME  |= 0xFF;      // enable PWM0
}

void setServoPose(int azimuth, int elevation){  
    PWMDTY45 = (int)(ZERO_ELEVATION_DUTY + elevation);  // Sets elevation to duty cycle using PWM 45
    PWMDTY67 = (int)(ZERO_AZIMUTH_DUTY + azimuth);   // Sets azimuth to duty cycle using PWM 67
}


void Init_TC6 (void) {
  TSCR1_TEN = 1;
  
  TSCR2 = 0x00;   // prescaler 1, before 32 = 0x04
  TIOS_IOS6 = 1;   // set channel 6 to output compare
    
  TCTL1_OL6 = 1;    // Output mode for ch6
  TIE_C6I = 1;   // enable interrupt for channel 6

  TFLG1 |= TFLG1_C6F_MASK;
}


// variables to make the servo move back and forth
// note: This is just to demonstrate the function of the servo
long iterator_counter = 0;
long tilt_up = 0;
int test_object = 0;
char buffer[128]; 


// the interrupt for timer 6 which is used for cycling the servo
#pragma CODE_SEG __NEAR_SEG NON_BANKED /* Interrupt section for this module. Placement will be in NON_BANKED area. */
__interrupt void TC6_ISR(void) { 
   
  TC6 = TCNT + 64000;   // interrupt delay depends on the prescaler
  TFLG1 |= TFLG1_C6F_MASK;

  if (servo_toggle == 0) {
    
    iterator_counter++; 
  }
  else if (servo_toggle == 2){          
    iterator_counter--; 
  } 
  else if(servo_toggle == 4){
    tilt_up = 30;
    iterator_counter++;
  }  
  else if(servo_toggle == 6){
    tilt_up = 30;
    iterator_counter--;
  }  
 /* else if(servo_toggle == 8){
    tilt_up = 50;
    iterator_counter++;
  }    
  else if(servo_toggle == 10){
    tilt_up = 50;
    iterator_counter--;
  }   */
    
       
  if ((iterator_counter == 960) || (iterator_counter == -960) && (servo_toggle<=6)) { 
    servo_toggle++;
    iterator_counter = 0;
    //sprintf(buffer, "Turn!\n");
  }                                                          

  
  // This is how we will pan every time we reach a magnet
  setServoPose(2450 + iterator_counter, tilt_up);    
}

