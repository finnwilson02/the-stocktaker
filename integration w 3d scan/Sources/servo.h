#ifndef SERVO_H
#define SERVO_H

extern int servo_toggle;
extern long iterator_counter;
extern long tilt_up;

void PWMinitialise(void);

// sets servo in elevation and azimuth
// note: this requires verification and calibration 
void setServoPose(int azimuth, int elevation);


// interrupt used for cycling through the servo positions
__interrupt void TC6_ISR(void);

void Init_TC6 (void);


#endif