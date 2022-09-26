/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand

/* Include the derivative-specific header file */
#include <mc9s12dg256.h>


#define LCD_DAT PORTK // Port K drives LCD data pins, E, and RS 
#define LCD_DIR DDRK // Direction of LCD port 
#define LCD_E 0x02 // LCD E signal 
#define LCD_RS 0x01 // LCD Register Select signal 
#define CMD 0 // Command type for move_to_next_line
#define DATA 1 // Data type for move_to_next_line

void init_lcd(void); // Initialize LCD display 
void move_to_next_line(char c, char type); // Write command or data to LCD controller 
void send_to_lcd (char *ptr); // Write a string to the LCD display 
void delay_50us(int n); // Delay n*50us
void delay_1ms(int n); // Delay n*1ms
