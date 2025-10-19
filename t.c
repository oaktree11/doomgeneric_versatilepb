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


char *tab = "0123456789ABCDEF";
int BASE;
int color;
#include "doomkeys.h"
#include "uart.c"
#include "kbd.c"
#include "timer.c"
#include "vid.c"
#include "exceptions.c"
#include "sdc.c"
#include "diskio.c"
#include "ff.c"
#define printf uprintf
 UART *up,*up1;
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
	  kprintf("U0 "); 
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
char pp[40];

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
        //printf("out of memory in sbrk incr = %d\n",incr);
        return (void *) -1;
    }

    heap_end = new_heap_end;
    return (void *) prev_heap_end;
}


/* VersatilePB Timer0 (SP804) base */
#define TIMER0_BASE   0x101E2000

/* Timer registers (offsets) */
#define TIMER_LOAD    0x00
#define TIMER_VALUE   0x04
#define TIMER_CONTROL 0x08
#define TIMER_BGLOAD  0x18

/* Control bits */
#define TIMER_CTRL_ENABLE      (1 << 7)
#define TIMER_CTRL_PERIODIC    (1 << 6)
#define TIMER_CTRL_INT_ENABLE  (1 << 5)
#define TIMER_CTRL_PRESCALE_1  (0 << 2)
#define TIMER_CTRL_PRESCALE_16 (1 << 2)
#define TIMER_CTRL_PRESCALE_256 (2 << 2)
#define TIMER_CTRL_32BIT       (1 << 1)
#define TIMER_CTRL_ONESHOT     (1 << 0)

static inline void mmio_write(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}
static inline uint32_t mmio_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}
//#ifdef NOINTERRUPTS
/* We’ll configure Timer0 as a free-running downcounter at 1 MHz (1 tick = 1 µs). */
void timer_init1(void) {
    // Disable first
    mmio_write(TIMER0_BASE + TIMER_CONTROL, 0);

    // Load max value
    mmio_write(TIMER0_BASE + TIMER_LOAD, 0xFFFFFFFF);

    // Enable, 32-bit, no interrupt, prescale=1
    mmio_write(TIMER0_BASE + TIMER_CONTROL,
        TIMER_CTRL_ENABLE | TIMER_CTRL_32BIT);
}

//#endif
//int got_square,got_esc;
int keys[300];
int set_new_pos;
int prev_key;
int cnt_prev_key;
int prev_key_value;
int DG_GetKey(int* pressed, unsigned char* doomKey){
  int ch;
 
  if ( !upeek(up))
     return 0;
 cnt_prev_key=0;
  ch = ugetc(up);
 
  *doomKey = ch;

  
  if (ch == 10 || ch == 13)
     ch= *doomKey = KEY_ENTER;
   
  *pressed=1;
uprintf("sending key %d\n",ch);  
  
  return 1;
}
/* Return milliseconds since boot */
uint32_t DG_GetTicksMs(void) {
    static uint32_t last_value = 0;
    static uint64_t ticks_us = 0;

    uint32_t value = mmio_read(TIMER0_BASE + TIMER_VALUE);
    // SP804 is a downcounter, so detect wrap
    if (value > last_value) {
        // wrapped around
        ticks_us += (0xFFFFFFFFu - value + last_value);
    } else {
        ticks_us += (last_value - value);
    }
    last_value = value;

    return (uint32_t)(ticks_us / 1000); // convert to ms
}

/* Busy-wait sleep */
void DG_SleepMs(int ms) {
    uint32_t start = DG_GetTicksMs();
    while ((DG_GetTicksMs() - start) < (uint32_t)ms) {
        // spin
    }
}

extern char _binary_panda_bmp_start;
extern char _binary_porkcar_bmp_start;
int yuy1;
int _exit(){
uprintf("exiting program");
while(1)
yuy1++;
}
int color;

/*** this is for converted images with |h|w|pixels| format ****
int show(char *p, int startRow)
{ 
   int h, w, row, col, pixel; 
   unsigned char r, g, b;

   h = (*(p+3)<<24) + (*(p+2)<<16) + (*(p+1)<<8) + *p;
   p += 4;
   w = (*(p+3)<<24) + (*(p+2)<<16) + (*(p+1)<<8) + *p;
   p += 4;          // skip over 8 byes

   uprintf("h=%d w=%d\n", h, w);
   //  if (h > 480) h = 480;
   //if (w > 640) w = 640;

   row = startRow; col = 0;
   while(1){
     r = *p; g = *(p+1); b = *(p+2);
     pixel = (b<<16) + (g<<8) + r;
     //     fb[row*640 + col] = pixel;
     fb[row*WIDTH + col] = pixel;
     p += 3;         // advance p by 3 bytes
     col++;
     if (col >= w){  // to line width of jpg image
        col = 0;
        row++;
     }
     if (row >= h+startRow)
        break;
   }
}
*******************************************/
void DG_SetWindowTitle(const char * title)
{
  uprintf(up,"title %s",title);
}
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
   char c, *p;
   int mode;
   char *argv[10];
   argv[0]="hello";
   argv[1]="-iwad";
   argv[2]="doom1.wad";
  // argv[3]="-file";
  
 //  argv[2]= "new_1lev.wad";
  
   
   
   
   
int i, sector, N; 
  
   KBD  *kp;
 
   color = RED;
   row = col = 0; 
   fbuf_init();
   kbd_init();
   uart_init();
   up = &uart[0];
   up1 = &uart[1];
   kp = &kbd;

   /* enable timer0,1, uart0,1 SIC interrupts */
   //VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4 
   VIC_INTENABLE |= (1<<12); // UART0 at bit12
   VIC_INTENABLE |= (1<<13); // UART1 at bit13
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
  
   /* enable KBD and SDC IRQ */
   //SIC_INTENABLE |= (1<<3);  // KBD int=bit3 on SIC
   SIC_INTENABLE |= (1<<22); // SDC int=bit22 on SIC

   //SIC_ENSET |= 1<<3;  // KBD int=3 on SIC
   SIC_ENSET |= 1<<22;  // SDC int=22 on SIC
   //*(kp->base+KCNTL) = 0x12; // for the keyboard

   uprintf("C3.4 start: test TIMER KBD UART SDC interrupt-driven drivers\n");
   timer_init1();
   //timer_start(0);
    sdc_init();
   uprintf("enter a key from this UART : ");
   
   fatfs_test();
   /*for (int yu=0;yu<40;yu++){
         if (!malloc(yu*10) )
            printf("%d\n",yu);
      }*/
   //p = &_binary_panda_bmp_start;
   //show_bmp(p, 0, 80); 
doomgeneric_Create(3, argv);
  

    while (1)
    {
        doomgeneric_Tick();
    
    // uprintf("enter a key from this UART : ");
    // int ch=ugetc(up);
//uprintf("%d",ch);     
   /*  p = &_binary_porkcar_bmp_start;
     show_bmp(p, 120, 0);

     uprintf("enter a key from this UART : ");
     ugetc(up);
     p = &_binary_panda_bmp_start;

     show_bmp(p, 120, 80);   uprintf("enter a key from this UART : ");
     ugetc(up);

     p = &_binary_porkcar_bmp_start;
     show_bmp(p, 0, 0);   uprintf("enter a key from this UART : ");
     ugetc(up);
*/
   }
   while(1);   // loop here  
}
