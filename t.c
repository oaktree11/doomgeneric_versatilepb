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
#include "diskio.c"
#include "ff.c"

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
}


char arr[512];
copy(char *p,char *p1, int sz){
for (int i=0;i<sz;i++){
  p[i]=p1[i];
  }
}

int zero(){
	for (int i=0;i<512;i++)
		arr[i]=0;
}


char rbuf[512], wbuf[512];
char *line[2] = {"THIS IS A TEST LINE", "this is a test line"};
//#include "ff.h"      // FatFs header
#if !FF_FS_NORTC && !FF_FS_READONLY
DWORD get_fattime (void)
{
	

	/* Get local time */
	//if (!rtc_gettime(&rtc)) return 0;
   int month = 1;
   int mday = 1;
   int hour = 1;
   int min = 1;
   int sec = 11;
   /* Pack date and time into a DWORD variable */
   return	  ((DWORD)(2025 - 1980) << 25)
			| ((DWORD)month << 21)
			| ((DWORD)mday << 16)
			| ((DWORD)hour << 11)
			| ((DWORD)min << 5)
			| ((DWORD)sec << 1);
}
#endif
#define printf uprintf
FATFS fs;       // Work area (filesystem object)
FIL fil;        // File object
DIR dir;        // Directory object
FILINFO fno;    // File information

void fatfs_test(void) {
    FRESULT fr;

    // Mount filesystem on drive 0 ("" means default)
    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("f_mount failed: %d\n", fr);
        return;
    }
    printf("Mounted filesystem OK\n");

   
}

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
   *(kp->base+KCNTL) = 0x12; // for the keyboard

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
   N = 3;
   
   zero();
fatfs_test();
   while(1){
      
   }
}
