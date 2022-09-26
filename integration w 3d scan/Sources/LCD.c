#include "derivative.h"
#include "LCD.h"
/*************************************************************/
/*                    Initialize LCD                         */
/*************************************************************/

void init_lcd(void)
{
  LCD_DIR = 0xFF;      // configure LCD_DAT port for output 
  delay_1ms(100);      // Wait LCD to be ready 
  move_to_next_line(0x28,CMD);   // set 4-bit data, 2-line display
  move_to_next_line(0x0F,CMD);   // turn on display
  move_to_next_line(0x06,CMD);  // move cursor right 
  move_to_next_line(0x01,CMD);  // clear screen, move cursor to home 
  delay_1ms(2);      // wait until "clear display" command is complete 
}

/*************************************************************/
/*               Send character to LCD                       */
/*************************************************************/
void send_to_lcd (char *ptr)
{
  while (*ptr) {       // While character to send 
  move_to_next_line(*ptr,DATA); // Write data to LCD 
  delay_50us(1);     // Wait for data to be written
  ptr++;             // Go to next character 
  }
}

/*************************************************************/
/*              Move to second line and first row of LCD     */
/*************************************************************/
void move_to_next_line(char c, char type)
{
  char low, high;
  
  high = (c & 0xF0) >> 2; // Upper 4 bits of c 
  low = (c & 0x0F) << 2; // Lower 4 bits of c 
  
  if (type == DATA) LCD_DAT |= LCD_RS;// select LCD data register 
  
  else LCD_DAT &= (~LCD_RS); // select LCD command register 
  
  if (type == DATA)
  
     LCD_DAT = high|LCD_E|LCD_RS; // output upper 4 bits, E, RS high
      
  else

    LCD_DAT = high|LCD_E; // output upper 4 bits, E, RS low 
    LCD_DAT |= LCD_E; // pull E signal to high 
    __asm(nop); // Lengthen E 
    __asm(nop);
    __asm(nop);
    LCD_DAT &= (~LCD_E); // change E high to low 
    
  if (type == DATA)
  
    LCD_DAT = low|LCD_E|LCD_RS; // output lower 4 bits, E, RS high
    
  else
  
    LCD_DAT = low|LCD_E; // output lower 4 bits, E, RS low 
    LCD_DAT |= LCD_E; // change E low to high 
    __asm(nop); //Lengthen E
    __asm(nop);
    __asm(nop);
    LCD_DAT &= (~LCD_E); // change E high to low 
    delay_50us(1); // Wait for command to execute
    
}


#define D50US 133 // Inner loop takes 9 cycles; need 50x24 = 1200 cycles 
/*************************************************************/
/*                   50 us delay                             */
/*************************************************************/
void delay_50us(int n)
{
  volatile int c;
  for (;n>0;n--)    //run n times
  for (c=D50US;c>0;c--) ;
}
/*************************************************************/
/*                    1 ms delay                             */
/*************************************************************/

void delay_1ms(int n)
{
  for (;n>0;n--) delay_50us(200);  //run n times
}
