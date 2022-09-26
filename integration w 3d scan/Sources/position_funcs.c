// define mag_count_x and mag_count_y as externs.  
#include "position_funcs.h"
#include "lcd.h"
#include "delay_func.h"
/*
extern mag_count_x;
extern mag_count_y; 
extern aisle_entry; 
extern aisle_depth;    */ 

void find_way_point(int x, int y){
    
   if (x == 240 && y != 240){
      mag_count_x++; 
  }else if (y == 240 && x != 240){
      mag_count_y++; 
  }
}
