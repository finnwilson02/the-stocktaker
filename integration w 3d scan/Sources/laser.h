#ifndef __laser_h
#define __laser_h

// get the latest laser measurement
void GetLatestLaserSample(unsigned long *sample);


// initialise the laser timer
void laserInit(void);

void convertLaserUnits(unsigned long *sample, unsigned long *scaled_sample, long *servo_pos, long *servo_tilt);

// interrupt for reading the laser signal
__interrupt void TC1_ISR(void);

#endif