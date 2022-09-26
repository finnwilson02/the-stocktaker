#include "cmd2LCD.h"
#include "derivative.h"

void cmd2LCD (char cmd) 
{  
		char hnibble, lnibble;
    PORTK = 0x00; // EN, RS=0    

    // sending higher nibble        
    hnibble = cmd &0xF0;      // mask first 4 bits
    PORTK = 0x02;             // EN = 1    
    hnibble >>=2;             // shift two bits to the right to align with PORTK positions
    PORTK = hnibble|0x02;

		// Slight delay
    asm("nop");
    asm("nop");
    asm("nop");

		// Reset Port K after first nibble is sent
    PORTK = 0x00; // EN,RS=0    
    
    // Sending lower nibble:
    lnibble = cmd &0x0F;
    PORTK = 0x02; 
    lnibble <<= 2;
    PORTK = lnibble|0x02;
    asm("nop");
    asm("nop");
    asm("nop");    
    PORTK = 0x00;
    
    // Wait 50 us for order to complete (enough time for most IR instructions)
		// I realise it's 1ms here but I was just lazy to write a us delay
    delay_ms(1);
}