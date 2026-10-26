/* uart.c - interrupt-driven PL011 with RX/TX queues */
#include "defines.h"

#define UDR   0x00
#define UFR   0x18
#define LCRH  0x2C
#define IMSC  0x38
#define MIS   0x40
#define ICR   0x44

#define SBUFSIZE 128

typedef volatile struct {
    char *base;
    char inbuf[SBUFSIZE];
    int  indata, inhead, intail;
    char outbuf[SBUFSIZE];
    int  outdata, outhead, outtail;
    int  txon;
} UART;
int kprintf(char *fmt, ...);
UART uart0;
UART uart[4];    
extern void int_off(void);
extern void int_on(void);
#define UARTCR  0x30   /* Control Register */
void uart_init(UART *up)
{
    //UART *up = &uart0;
    if (up == &uart[0])
    up->base = (char *)UART0_BASE_ADDR;
    else
       up->base = (char *)UART1_BASE_ADDR;
    /* Disable UART while we configure it */
    *(volatile u32 *)(up->base + UARTCR) = 0;

    /* Baud rate (QEMU mostly ignores it, but real HW / strict models want it) */
    *(volatile u32 *)(up->base + 0x24) = 1;     /* IBRD */
    *(volatile u32 *)(up->base + 0x28) = 0;     /* FBRD */

    /* 8-bit, no parity, 1 stop, FIFO disabled */
    *(volatile u32 *)(up->base + LCRH) = 0x60;  /* WLEN=8, FEN=0 */

    /* Clear any pending interrupts */
    *(volatile u32 *)(up->base + ICR) = 0x7FF;

    /* Enable UART + TX + RX */
    *(volatile u32 *)(up->base + UARTCR) = 0x301;  /* UARTEN|TXE|RXE */

    /* Only RX interrupt at start */
    *(volatile u32 *)(up->base + IMSC) = 0x10;

    up->indata = up->inhead = up->intail = 0;
    up->outdata = up->outhead = up->outtail = 0;
    up->txon = 0;
}

static void do_rx(UART *up)
{
    char c;
    /* data already present */
    if (*(volatile u32 *)(up->base + UFR) & 0x10)  /* RXFE */
        return;

    c = *(volatile u32 *)(up->base + UDR);

    if (up->indata < SBUFSIZE) {
        up->inbuf[up->inhead++] = c;
        up->inhead %= SBUFSIZE;
        up->indata++;
    }
}

static void do_tx(UART *up)
{
    char c;

    if (up->outdata <= 0) {
        /* Nothing left – turn TX interrupt off, leave RX on */
        *(volatile u32 *)(up->base + IMSC) &= ~0x20;
        up->txon = 0;
        return;
    }

    c = up->outbuf[up->outtail++];
    up->outtail %= SBUFSIZE;

    /* Wait until there is space (should be immediate with FIFO off) */
    while (*(volatile u32 *)(up->base + UFR) & 0x20)
        ;
    *(volatile u32 *)(up->base + UDR) = (u32)c;
    up->outdata--;
}

void uart_handler(UART *up)
{
    //UART *up = &uart0;
    u32 mis = *(volatile u32 *)(up->base + MIS);
    if (mis & 0x10) {
        while (!(*(volatile u32 *)(up->base + UFR) & 0x10))  // RXFE
            do_rx(up);
    }
   
    if (mis & 0x20)
        do_tx(up);

    /* clear the sources we handled */
    *(volatile u32 *)(up->base + ICR) = mis & 0x30;
}

/* non-blocking check */
int uart_rx_ready(void)
{
    return uart0.indata > 0;
}
upeek(UART *up){
return up->indata>0;
}

char ugetc(UART *up)
{
    //UART *up = &uart0;
    char c;
    while (up->indata <= 0)
        ;
    c = up->inbuf[up->intail++];
    up->intail %= SBUFSIZE;
    int_off();
    up->indata--;
    int_on();
    return c;
}

void uputc(UART *up,char c)
{
    //UART *up = &uart0;

    if (up->txon) {
        /* Already transmitting – just queue */
        if (up->outdata < SBUFSIZE) {
            up->outbuf[up->outhead++] = c;
            up->outhead %= SBUFSIZE;
            int_off();
            up->outdata++;
            int_on();
        }
        return;
    }

    /*
     * First character of a new burst:
     * write it directly (polled) so it is guaranteed to leave,
     * then enable the TX interrupt for any subsequent queued chars.
     */
    while (*(volatile u32 *)(up->base + UFR) & 0x20)
        ;
    *(volatile u32 *)(up->base + UDR) = (u32)c;

    /* If the software queue already has data, enable TX IRQ */
    if (up->outdata > 0) {
        *(volatile u32 *)(up->base + IMSC) |= 0x20;
        up->txon = 1;
    }
}

void uputs(UART *up,const char *s)
{
    while (*s) {
        if (*s == '\n')
            uputc(up,'\r');
        uputc(up,*s++);
    }
}
void ugets(UART *up,char *s)
{
  //kprintf("%s", "in ugets: ");
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}
/* simple printf that goes to UART (and also used by kprintf on video) */
static const char *hex = "0123456789ABCDEF";

void uprintf(const char *fmt, ...)
{
    int *ap = (int *)&fmt + 1;
    char c;
    UART *up = &uart[0];
    while ((c = *fmt++)) {
        if (c != '%') {
            if (c == '\n') uputc(up,'\r');
            uputc(up,c);
            continue;
        }
        c = *fmt++;
        if (c == 'c') uputc(up,(char)*ap++);
        else if (c == 's') uputs(up,(char *)*ap++);
        else if (c == 'd' || c == 'u') {
            int n = *ap++;
            char buf[16]; int i = 0;
            if (n < 0 && c == 'd') { uputc(up,'-'); n = -n; }
            if (n == 0) buf[i++] = '0';
            while (n) { buf[i++] = '0' + (n % 10); n /= 10; }
            while (i--) uputc(up,buf[i]);
        } else if (c == 'x') {
            u32 n = (u32)*ap++;
            int i;
            uputs(up,"0x");
            for (i = 28; i >= 0; i -= 4)
                uputc(up,hex[(n >> i) & 0xF]);
        }
    }
}
void uprintf1(const char *fmt, ...)
{
    int *ap = (int *)&fmt + 1;
    char c;
    UART *up = &uart[1];
    while ((c = *fmt++)) {
        if (c != '%') {
            if (c == '\n') uputc(up,'\r');
            uputc(up,c);
            continue;
        }
        c = *fmt++;
        if (c == 'c') uputc(up,(char)*ap++);
        else if (c == 's') uputs(up,(char *)*ap++);
        else if (c == 'd' || c == 'u') {
            int n = *ap++;
            char buf[16]; int i = 0;
            if (n < 0 && c == 'd') { uputc(up,'-'); n = -n; }
            if (n == 0) buf[i++] = '0';
            while (n) { buf[i++] = '0' + (n % 10); n /= 10; }
            while (i--) uputc(up,buf[i]);
        } else if (c == 'x') {
            u32 n = (u32)*ap++;
            int i;
            uputs(up,"0x");
            for (i = 28; i >= 0; i -= 4)
                uputc(up,hex[(n >> i) & 0xF]);
        }
    }
}
