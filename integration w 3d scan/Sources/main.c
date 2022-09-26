#include <hidef.h>      /* common defines and macros */
#include <assert.h>
#include "derivative.h"      /* derivative-specific definitions */

// most correct one.

// need this for string functions
#include <stdio.h>
#include <math.h>

#include "pll.h"
#include "simple_serial.h"

#include "l3g4200d.h"

#include "servo.h"
#include "laser.h"

#include "gyro.h"

#include "lcd.h"

#include "position_funcs.h"

#include "cmd2LCD.h"

#include "delay_func.h"

void speak(int pitch,int duration);
void magnet_check(int read_magnetx, int read_magnety, int *x_count, int *y_count, int *x_mag, int *y_mag);
void location_status(int arr[], int len, int idx); 
void update_position(int *x_mag, int *y_mag, int aisle[], int sec[]);  
int loop; 
int i_avg;
int z;

int n; 

// GF additions
double average(double* array,int length);
double sum = 0;
double average_calc;
int output_size;

int durations[12] = {139,147,156,165,175,185,196,207,220,233,261};
/*Initialise an array containing frequency for each note and associated variable for duration*/
int pitches[12] = {3597,3401,3212,3042,2867,2710,2554,2400,2276,2148,2031,1914};

typedef struct {
  int sentinel;
  int x;
  int y;
  int z;
  int area;
//  int end_sentinel;
} packetOut;    

double rad = (2*3.141592)/360;

char *msg1;
char *msg2; 
char *secmsg1;
char *secmsg2;
char *secmsg3;
char *secmsg4;
char aisle_num_str[1]; 

int mag_count_x;
int mag_count_y;
int g; 

int x_count;
int y_count; 
int x_mag;
int y_mag;
int aisle[2];
int sec[4]; 

int ready; 
 
//int aisles[] = {1,2}; 
// int sections[] = {1,2,3};
 
char lcd_buffer[128]; 
char lcd_section_buffer[128]; 

int dec_flag = 0; 
void printErrorCode(IIC_ERRORS error_code) {
  char buffer[128];  
  switch (error_code) {
    case NO_ERROR: 
      sprintf(buffer, "IIC: No error\r\n");
      break;
    
    case NO_RESPONSE: 
      sprintf(buffer, "IIC: No response\r\n");
      break;
    
    case NAK_RESPONSE:
      sprintf(buffer, "IIC: No acknowledge\r\n");
      break;
    
    case IIB_CLEAR_TIMEOUT:
      sprintf(buffer, "IIC: Timeout waiting for reply\r\n");
      break;
    
    case IIB_SET_TIMEOUT: 
      sprintf(buffer, "IIC: Timeout not set\r\n");
      break;
    
    case RECEIVE_TIMEOUT:
      sprintf(buffer, "IIC: Received timeout\r\n");
      break;
    
    case IIC_DATA_SIZE_TOO_SMALL:
      sprintf(buffer, "IIC: Data size incorrect\r\n");
      break;

    default:
      sprintf(buffer, "IIC: Unknown error\r\n");
      break;
  }
    
  SerialOutputString(buffer, &SCI1);
}

void main(void) {
  
  double dataX;
  double dataY;
  double dataZ;
  
  AccelRaw read_accel;
  AccelScaled scaled_accel;

  GyroRaw read_gyro;
  MagRaw read_magnet;
  
  packetOut outputData;
  unsigned char transmit_buffer[sizeof(packetOut)];
  unsigned char *transmit_buffer_ptr = &transmit_buffer; 
  
  IIC_ERRORS error_code = NO_ERROR;
  
  char buffer[128];
  
  unsigned long singleSample;
  unsigned long scaledSample;
  long servoPan;
  long servoTilt;
  
  int array_counter = 0;
  double distances[5];
  double distance_avg;
  double pan;
  double tilt;
  double distance_horizontal;  
  

  // make sure the board is set to 24MHz
  //  this is needed only when not using the debugger
  PLL_Init();

  // initialise PWM
  PWMinitialise();
  setServoPose(2450, 0);


  
  // initialise the simple serial
  SerialInitialise(BAUD_9600, &SCI1);
  
  aisle_depth = 0;
  aisle_entry = 0;
  DDRH = 0x00; // set port H to input. 
  DDRB = 0xFF; // set port B to output. 
  DDRJ = 0xFF; // set port J to output. 
  
  PTJ = 0x00; // set port J low. 
  PORTB = 0x00; // LEDs OFF.
  
   
  x_mag = 0;
  y_mag = 0;
  x_count = 0;
  y_count = 0; 
  aisle[0] = 3;
  aisle[1] = 3; 
  ready = 0; 
  
  msg1 = " Aisle: 1"; 
  msg2 = " Aisle: 2"; 
  secmsg1 = "Section: 1 R";
  secmsg2 = "Section: 2 R";
  secmsg3 = "Section: 2 L";
  secmsg4 = "Section: 1 L"; 
  // initialise the sensor suite
  error_code = iicSensorInit();
  
  // write the result of the sensor initialisation to the serial
/*  if (error_code == NO_ERROR) {
    sprintf(buffer, "NO_ERROR\r\n");
    SerialOutputString(buffer, &SCI1);
  } else {
    sprintf(buffer, "ERROR %d\r\n");
    SerialOutputString(buffer, &SCI1);
  }   */ // this block was jack testing. 

  laserInit();
  
  openlcd();   
  cmd2LCD(0x01); // hmmmmm 
  delay_ms_test(3); // JUST INTRODUCED. 

  Init_TC6();
  
  // Disable motion until requested
  TIE_C6I = 0;
  
	EnableInterrupts;
  //COPCTL = 7;
  _DISABLE_COP();


  

  for(;;) {
  
  
    // read the raw values
    error_code = getRawDataGyro(&read_gyro);   
    if (error_code != NO_ERROR) {
      printErrorCode(error_code);   
       
      error_code = iicSensorInit();
      printErrorCode(error_code);   
    }
    
    error_code = getRawDataAccel(&read_accel);
    if (error_code != NO_ERROR) {
      printErrorCode(error_code);   
    
      error_code = iicSensorInit();
      printErrorCode(error_code); 
    }
    
    error_code = getRawDataMagnet(&read_magnet);
    
    GetLatestLaserSample(&singleSample);
  
    // convert the acceleration to a scaled value
    convertUnits(&read_accel, &scaled_accel);    

    // Check for robber!
    if((read_accel.y < -300) || (read_accel.z < -300)){
      for(;;){
        speak(pitches[5], durations[10]);
        PORTB = 255;
        speak(pitches[1], durations[10]);
        PORTB = 0;
      }
    }   
    
    // positioning code: 
    
    // find_way_point(read_magnet.x, read_magnet.y); 
    magnet_check(read_magnet.x, read_magnet.y, &x_count, &y_count, &x_mag, &y_mag);  
    
    update_position(&x_mag, &y_mag, aisle, sec);
    
    if (aisle[0] == 1){
      
     cmd2LCD(0x01);
     delay_ms(2);   
     send_to_lcd(msg1);
      
      
    }else if (aisle[1] == 1){
    
     cmd2LCD(0x01);
     delay_ms(2);   
     send_to_lcd(msg2);
     
    }
    
    if (sec[0] == 1){
       cmd2LCD(0xC0);
       delay_ms_test(2);   
       send_to_lcd(secmsg1);
       
    } else if (sec[1] == 1){
       cmd2LCD(0xC0);
       delay_ms_test(2);   
       send_to_lcd(secmsg2); 
      
    } else if (sec[2] == 1){
        cmd2LCD(0xC0);
        delay_ms_test(2);   
        send_to_lcd(secmsg3);
    } else if (sec[3] == 1){
        cmd2LCD(0xC0);
        delay_ms_test(2);   
        send_to_lcd(secmsg4); 
    }
    
    // if we are at a section
    
    if ((sec[0] == 1 || sec[1] == 1 || sec[2] == 1 || sec[3] == 1) && ready == 1){  // if any of the sections are bossin
   
       // poll for button input. Want program to get stuck here if no button is pushed. 
    //   int condition = 1;
        
      // while (condition){
        
        if (PTH == 254){   // if button pushed. 
          
       //  condition = 0;
         ready = 0; 
         // Enable pan and tilt
         TIE_C6I = 1; 
         
         outputData.area = 0;
       
         // Code for finding aisle and section
         for(z = 0; z<2; z++){
            outputData.area += aisle[z]*(z*4);
         }
         for(z = 0; z<4; z++){
            outputData.area += sec[z]*z;
         }    
               
         
         // Set up sentinels (georgia edit)
          outputData.sentinel = 0xABCD;
        //  outputData.end_sentinel = 0xDCBA;
         
          // do some other shit 
          while(servo_toggle < 7){
            
            GetLatestLaserSample(&singleSample);
            convertLaserUnits(&singleSample, &scaledSample, &servoPan, &servoTilt);
            
            distances[array_counter] = (double) scaledSample;
            
            if (distances[array_counter] < 0 || distances[array_counter] > 400){
              array_counter--;
            }
  
            array_counter++;
   
            if(array_counter == 5){
            
              distance_avg = average(&distances,5);
              pan = (double) servoPan;
              tilt = (double) servoTilt;
              
              distance_horizontal = distance_avg*cos(rad*tilt);
              
              dataX = distance_horizontal*sin(rad*pan)*10;
              dataY = distance_horizontal*cos(rad*pan)*10;
              dataZ = distance_avg*sin(rad*tilt)*10;
              
              outputData.x = (int)dataX;
              outputData.y = (int)dataY;
              outputData.z = (int)dataZ;
              
              output_size = sizeof(outputData);
              
              SerialOutputBytes((char*)&outputData, output_size, &SCI1); 
              array_counter = 0; 
            } 
            
            if(servo_toggle == 1){
              setServoPose(2450, 0);
              servo_toggle++;
            } 
            else if(servo_toggle == 3){
              setServoPose(2450, 30);
              servo_toggle++;
            }
            else if(servo_toggle == 5){
              setServoPose(2450, 30);
              servo_toggle++;
            }
          /*  else if(servo_toggle == 7){
              setServoPose(2450, 50);
              servo_toggle++;
            }
            else if(servo_toggle == 9){
              setServoPose(2450, 50);
              servo_toggle++;
            }   */
          }
        }
       //}
    } 
    
    
    // Send empty struct to python
    outputData.sentinel = 0;
    //outputData.end_sentinel = 0;
    outputData.area = 0;
    outputData.x = 0;
    outputData.y = 0;
    outputData.z = 0;
    
    SerialOutputBytes((char*)&outputData, output_size, &SCI1); 
    
    servo_toggle = 0;
    TIE_C6I = 0;                // Disable interrupt for output compare 6 thus disabling motion
       
    // format the string of the sensor data to go the the serial    
 //   sprintf(buffer, "%d, %d, %d\r\n",read_accel.x, read_accel.y, read_accel.z);
    
    // output the data to serial
    //SerialOutputString(buffer, &SCI1);
    
 

  } /* loop forever */
  
  /* please make sure that you never leave main */
  
  
  
}





void magnet_check(int read_magnetx, int read_magnety, int *x_count, int *y_count, int *x_mag, int *y_mag){
  
  if (read_magnetx == 240 && read_magnety != 240){ 
    *x_count = *x_count + 1;
    if (*x_count == 5){ //magnet detected after 5 consecutive 240 values
      *x_mag += 1; //add 1 to the x magnet count
      delay_ms_test(1000); 
    }
  } 
  else{
    *x_count = 0; //if value does not equal 240 counter is reset
  }
  
  if (read_magnety == 240 && read_magnetx != 240){
    *y_count += 1;
     ready = 1; 
    if (*y_count == 5){
      *y_mag += 1;
      delay_ms_test(1000);
    }
  } 
  else{
    *y_count = 0;
  }
  
  
  
  
}



//defines the status of each aisle and section
void location_status(int arr[], int len, int idx){
  int i; 
  for (i = 0; i < len; i++){
    
    if (i == idx){
      arr[i] = 1;
      
    }
    else{
      arr[i] = 0;
    }
  }
}





void update_position(int *x_mag, int *y_mag, int aisle[], int sec[]){
  
  if (*x_mag >= 1 && *y_mag == 0){ //if robot is inside neither aisle, determine which entrance it is at.
    if (*x_mag%2 == 0){
      location_status(aisle, 2, 1);
    } 
    else{
      location_status(aisle, 2, 0);
    }
  } 
  else if (*y_mag > 0){ //if the robot is inside an aisle determine what section and side (left or right) it is scanning
    loop = 0;
    location_status(sec, 4, *y_mag-1); 
    
    if (*y_mag == 4){
      loop = 1;
    }
    
  if ((aisle[0] == 1) && (loop == 1) && (*x_mag%2 == 0)){ //accounting for when x magnet is detected leaving an aisle
    *x_mag -= 1;
    *y_mag = 0;
  } 
  else if ((aisle[1] == 1) && (loop == 1) && (*x_mag%2 != 0)){
    *x_mag -= 1;
    *y_mag = 0;
  }
}

}

double average(double* array,int length){
  
    sum = 0;
   //char buffer[128];
   for(i_avg=0; i_avg<length; i_avg++){
    
      sum += array[i_avg];
      
      //sprintf(buffer, "%lu\r\n", array[i_avg]);
      //SerialOutputString(buffer, &SCI1);
   }
   
   average_calc = sum/length;
   return average_calc;
}

void speak(int pitch,int duration){

DDRT = 0xFF;   /*Configure port T as output*/

if (duration == 0){       /*If pause, turn speaker off for duration of one note*/
  
    PTT = 0x00;
    for (n=0;n<1000;n++){
        for (m=0;m<2000;m++){
        
        }
    }
    return;
}
    
for (n=0;n<duration;n++){      /*loop for length of time note is                                      uto be played*/
   
    PTT = 0x20;                /*drive speaker HIGH*/

    for(m=0;m<pitch;m++){      /*pause to acheive target frequency*/

    }

    PTT = 0x00;                /*drive speaker LOW*/

    for(m=0;m<pitch;m++){      /*pause to acheive target frequency*/

    }
  
}
  
  
}
