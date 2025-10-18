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
//#define printf kprintf

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
extern char _end;      /* defined by linker */
extern char stack_top;   /* defined by linker */

static char *heap_end;

void* _sbrk(int incr) {
 //printf("sbrk incr = %d\n",incr);
    if (heap_end == 0) {
        heap_end = &_end;
    }

    char *prev_heap_end = heap_end;
    char *new_heap_end  = heap_end + incr;

    if (new_heap_end >= &stack_top) {
        // Out of memory
        printf("out of memory in sbrk incr = %d\n",incr);
        return (void *) -1;
    }

    heap_end = new_heap_end;
    return (void *) prev_heap_end;
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
#include "ff.h"   // FatFs
FIL *fil_nos[40];
int filenospos=3;
int _open(char *name, int flags, int mode) {
    FIL *fp = malloc(sizeof(FIL));
    if (!fp) return -1;
    //name = "doom1.wad";
    printf("call open %s\n",name);
    if (f_open(fp, name, FA_READ) != FR_OK) {
        free(fp);
        return -1;
    }
    printf("open ok \n");
    int fd = filenospos;
    fil_nos[filenospos++]=fp;
    return fd;   // crude: cast pointer to int as FD
}

int _read(int fd, char *buf, int len) {
    FIL *fp = (FIL*)fil_nos[fd];
    UINT br;
    if (f_read(fp, buf, len, &br) != FR_OK) return -1;
    return br;
}
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

int _lseek(int fd, int pos, int whence) {
    FIL *fp = (FIL*)fil_nos[fd];//(FIL*)fd;
    if (whence == SEEK_SET) f_lseek(fp, pos);
    else if (whence == SEEK_CUR) f_lseek(fp, f_tell(fp) + pos);
    else if (whence == SEEK_END) f_lseek(fp, f_size(fp) + pos);
    return f_tell(fp);
}

int _close(int fd) {
    FIL *fp = (FIL*)fil_nos[fd];//(FIL*)fd;
    f_close(fp);
    free(fp);
    return 0;
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
