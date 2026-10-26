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

// uart.c file
/************************************
UART0 base address: 0x101f1000;
UART1 base address: 0x101f2000;
UART2 base address: 0x101f3000;
UART3 base address: 0x10009000;

// UART's flag register at 0x18
//  7    6    5    4    3    2   1   0
// TXFE RXFF TXFF RXFE BUSY
// TX FULL : 0x20
// TX empty: 0x80
// RX FULL : 0x40
// RX empty: 0x10
// BUSY=1 :  0x08
***********************************/
int kprintf(char *fmt, ...);

/******** bytes offsets: for char *base ********/
#define UDR  0x00
#define UDS  0x04
#define UFR  0x18
#define CNTL 0x2C
#define MASK 0x38
#define MIS  0x40 
#define ICR 0x44
#define SBUFSIZE 128



typedef volatile struct uart{
  char *base;     // base address; as char *
  u32 n;          // uart number 0-3

  char inbuf[SBUFSIZE];
  int indata, inroom, inhead, intail;
  char outbuf[SBUFSIZE];
  int outdata, outroom, outhead, outtail;
  int txon; // 1=TX interrupt is on
}UART;

UART uart[4];     // 4 UArt structures
#ifdef old
int uart_init()
{
  int i;
  UART *up;
  for (i=0; i<4; i++){   // uart0 to uart2 are adjacent 
    up = &uart[i];
    up->base = (char *)(0x101f1000 + i*0x1000);

    *(up->base+0x2C) &= ~0x10; // disable FIFO
    *(up->base+0x38) |= 0x30;
    
    up->n = i;
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;

    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    up->txon = 0;
  }  
  uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
}
#endif

int uart_init()
{
  int i;
  UART *up;
  for (i = 0; i < 4; i++) {
    up = &uart[i];
    up->base = (char *)(0x101f1000 + i * 0x1000);

    /* --- proper PL011 bring-up (needed for QEMU ≥ ~8/9 and real HW) --- */
    *(volatile u32 *)(up->base + 0x30) = 0;          /* UARTCR = 0 (disable) */
    *(volatile u32 *)(up->base + 0x24) = 1;          /* IBRD */
    *(volatile u32 *)(up->base + 0x28) = 0;          /* FBRD */
    *(volatile u32 *)(up->base + 0x2C) = 0x60;       /* LCRH: 8-bit, FIFO off */
    *(volatile u32 *)(up->base + 0x44) = 0x7FF;      /* ICR: clear all */
    *(volatile u32 *)(up->base + 0x30) = 0x301;      /* UARTCR: UARTEN|TXE|RXE */
    *(volatile u32 *)(up->base + 0x38) = 0x10;       /* IMSC: RX only for now */

    up->n = i;
    up->indata = up->inhead = up->intail = 0;
    up->inroom = SBUFSIZE;
    up->outdata = up->outhead = up->outtail = 0;
    up->outroom = SBUFSIZE;
    up->txon = 0;
  }
  uart[3].base = (char *)(0x10009000);
  return 0;
}

void uart_handler(UART *up) 
{
  u8 mask, mis;
  mask = *(up->base + MASK);  // read MASK register
  mis = *(up->base + MIS);  // read MIS register
  //kprintf("uart%d interrupt mask=%x MIS=%x\n", up->n, mask, mis);
  /* clear FIRST, then service */
    *(volatile u32 *)(up->base + ICR) = mis & 0x30;
    if (mis & 0x10) {
        while (!(*(volatile u32 *)(up->base + UFR) & 0x10))  // RXFE
            do_rx(up);
    }
    
  if (mis & 0x20)
    do_tx(up);
  //*(up->base + 0x44) = mis & 0x30;   /* clear the sources we just handled */
}
int got_square,got_esc;
int do_rx(UART *up)
{
  char c;
  color = GREEN;

  while(!(*(up->base + UFR) & 0x40));
  c = *(up->base+UDR);
//uprintf("rx interrupt: %c\n", c);


  up->inbuf[up->inhead++] = c; 
  up->inhead %= SBUFSIZE;
  up->indata++; up->inroom--;

  color=RED;
}

int do_tx(UART *up)
{
  char c;
  //kprintf("TX interrupt\n");
  if (up->outdata <= 0){
    // disable TX interrupt; return
    *(up->base+MASK) = 0x10; // mask out TX interrupt
    up->txon = 0;
    return;
  }
  c = up->outbuf[up->outtail++];
  up->outtail %= 128;

  while( *(up->base+UFR) & 0x20 ); // loop while FR=TXF  
  *(up->base+UDR) = (int)c;        // write c to DR
  
  up->outdata--; up->outroom++;
}
       
upeek(UART *up){
return up->indata>0;
}

int ugetc(UART *up)
{
  char c;
  while(up->indata <= 0); // loop until up->data > 0 READONLY, no need for lock

  c = up->inbuf[up->intail++];
  up->intail %= SBUFSIZE;

  // updating variables: must disable interrupts
  int_off();
    up->indata--; up->inroom++;
  int_on();
  return c;
}


// TO DO: UART outputs should be intertupt-driven also 
int uputc(UART *up, char c)
{

/* Always push the first few characters polled – newer QEMU is picky
     about TX interrupt enable timing right after UARTCR is set. */
  if (up->outdata == 0 && !up->txon) {
    while (*(up->base + UFR) & 0x20)
      ;
    *(up->base + UDR) = (int)c;
    return 1;
  }

  /* original queue + TX-interrupt path unchanged */
  if (up->txon) {
    if (up->outdata < SBUFSIZE) {
      up->outbuf[up->outhead++] = c;
      up->outhead %= SBUFSIZE;
      int_off();
      up->outdata++; up->outroom--;
      int_on();
    }
    return 1;
  }

  // txon==0 means TX is off => output c & enable TX interrupt
  // PL011 TX is riggered only if write char, else no TX interrupt
  int i = *(up->base+UFR);         // read FR
  while( *(up->base+UFR) & 0x20 ); // loop while FR=TXF  
  *(up->base+UDR) = (int)c;        // write c to DR
  *(up->base+MASK) |= 0x30;// 0000 0000: bit5=TX mask bit4=RX mask
  up->txon = 1;
}

int ugets(UART *up, char *s)
{
  //kprintf("%s", "in ugets: ");
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);}

int urpx(UART *up, int x)
{
  char c;
  if (x){
     c = tab[x % 16];
     urpx(up, x / 16);
  }
  uputc(up, c);
}

int uprintx(UART *up, int x)
{
  uprints(up, "0x");
  if (x==0)
    uputc(up, '0');
  else
    urpx(up, x);
  uputc(up, ' ');
}

int urpu(UART *up, int x)
{
  char c;
  if (x){
     c = tab[x % 10];
     urpu(up, x / 10);
  }
  uputc(up, c);
}

int uprintu(UART *up, int x)
{
  if (x==0)
    uputc(up, '0');
  else
    urpu(up, x);
  uputc(up, ' ');
}

int uprinti(UART *up, int x)
{
  if (x<0){
    uputc(up, '-');
    x = -x;
  }
  uprintu(up, x);
}

int ufprintf(UART *up, char *fmt,...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  while(*cp){
    if (*cp != '%'){
      uputc(up, *cp);
      if (*cp=='\n')
	uputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': uputc(up, (char)*ip);      break;
    case 's': uprints(up, (char *)*ip);  break;
    case 'd': uprinti(up, *ip);           break;
    case 'u': uprintu(up, *ip);           break;
    case 'x': uprintx(up, *ip);  break;
    }
    cp++; ip++;
  }
}

int uprintf(char *fmt, ...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  UART *up = &uart[0];

  while(*cp){
    if (*cp != '%'){
      uputc(up, *cp);
      if (*cp=='\n')
	uputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch(*cp){
    case 'c': uputc(up, (char)*ip);      break;
    case 's': uprints(up, (char *)*ip);   break;
    case 'd': uprinti(up, *ip);           break;
    case 'u': uprintu(up, *ip);           break;
    case 'x': uprintx(up, *ip);  break;
    }
    cp++; ip++;
  }
}

