/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "defines.h"
#include "string.c"
#define printf kprintf

char *tab = "0123456789ABCDEF";
int BASE;
int color;

#include "uart.c"
#include "kbd.c"
#include "timer.c"
#include "vid.c"
#include "exceptions.c"
#include "sdc.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}
int kprintf(char *fmt, ...);
void timer_handler();
extern int sdc_handler();

// IRQ interrupts handler entry point
//void __attribute__((interrupt)) kc_handler()
void kc_handler()
{
    int vicstatus, sicstatus;
    int ustatus, kstatus;

    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  
    //kprintf("vicstatus=%x sicstatus=%x\n", vicstatus, sicstatus);
    // VIC status BITs: timer0=4, uart0=13, uart1=14, SIC=31: KBD at 3

    if (vicstatus & (1<<4)){
         timer_handler(0);
	 // kprintf("TIMER "); // verify timer handler return to here
    }
    if (vicstatus & (1<<12)){ // Bit 12 
         uart_handler(&uart[0]);
	 // kprintf("U0 "); 
    }
    if (vicstatus & (1<<13)){ // bit 13
         uart_handler(&uart[1]);
    }
    if (vicstatus & 0x80000000){ // SIC interrupts=bit_31=>KBD at bit 3 
       if (sicstatus & (1<<3)){
          kbd_handler();
       }
       if (sicstatus & (1<<22)){
          sdc_handler();
       }
    }
//VIC_CLEAR =0;
}

char rbuf[512], wbuf[512];
char *line[2] = {"11111IS A TEST LINE", "this is a test line"};
int main()
{
  int i, sector, N; 
   UART *up;
   KBD  *kp;
 
   color = RED;
   row = col = 0; 
   fbuf_init();
   kbd_init();
   uart_init();
   up = &uart[0];
   kp = &kbd;

   /* enable timer0,1, uart0,1 SIC interrupts */
   VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4 
   VIC_INTENABLE |= (1<<12); // UART0 at bit12
   VIC_INTENABLE |= (1<<13); // UART1 at bit13
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
  
   /* enable KBD and SDC IRQ */
   SIC_INTENABLE |= (1<<3);  // KBD int=bit3 on SIC
   SIC_INTENABLE |= (1<<22); // SDC int=bit22 on SIC

   SIC_ENSET |= 1<<3;  // KBD int=3 on SIC
   SIC_ENSET |= 1<<22;  // SDC int=22 on SIC
   *(kp->base+KCNTL) = 0x12;

   kprintf("C3.4 start: test TIMER KBD UART SDC interrupt-driven drivers\n");
   timer_init();
   timer_start(0);

   /***********
   kprintf("test uart0 I/O: enter text from UART 0\n");
   while(1){
     ugets(up, line);
     //color=GREEN; kprintf("line=%s\n", line); color=RED;
     uprintf("  line=%s\n", line);
     if (strcmp(line, "end")==0)
	break;
   }

   //uprintf("test KBD inputs\n"); // print to uart0
   kprintf("test KBD inputs\n"); // print to LCD
   
   while(1){
      kgets(line);
      color = GREEN;
      //uprintf("line=%s\n", line);
      kprintf("line=%s\n", line);
      if (strcmp(line, "end")==0)
         break;
   }
   **************/
   color = CYAN;
   printf("test SDC DRIVER\n");
   sdc_init();
   N = 1;
   for (sector=0; sector < N; sector++){
     printf("WRITE sector %d: ", sector);
     for (i=0; i<512; i++)
        wbuf[i] = ' ';
     for (i=0; i<12; i++){
         kstrcpy(wbuf+i*40, line[sector % 2]);
     }
     putSector(sector, wbuf);
   }
   printf("\n");

   for (sector=0; sector < N; sector++){
     printf("READ  sector %d\n", sector);
     for (i=0; i<512; i++)
        rbuf[i] = ' ';

     getSector(sector, rbuf);
     for (i=0; i<512; i++){
       printf("%c", rbuf[i]);
     }
     printf("\n");
   }
   printf("in while(1) loop: enter keys from KBD or UART\n");
   while(1);
}
