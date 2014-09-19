#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/io.h>

/* Programming the watchdog timer of Cool FrontRunner */

/* Run the program as root, otherwise you get "segmentation fault" */

#define N(arg) do { if((arg) < 0) { perror(#arg); exit(1); } } while(0)

volatile sig_atomic_t terminate_proc = 0;

void sig_handler(int sig)
  {
    terminate_proc = 1;
  }

int main()
  {
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = SA_RESTART;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGINT, &sa, NULL));
    N(sigaction(SIGTERM, &sa, NULL));

    sa.sa_handler = SIG_IGN;
    N(sigaction(SIGHUP, &sa, NULL));
    
    ioperm(0x4e, 2, 1);
     
    /* enter SuperIO configuration mode */
    outb(0x87, 0x4e);
    outb(0x87, 0x4e);

    /* enable logical device 9 */
    outb(0x07, 0x4e);
    outb(0x09, 0x4f);

    /* set GP33 as output in configuration register f0h -> Bit4 = 0 */
    outb(0xf0, 0x4e);
    outb(0xc7, 0x4f);

    /* set GP33 (Bit4) value in cofiguration register f1h to 1
       to enable watchdog function */
    outb(0xf1, 0x4e);
    outb((inb(0x4f) | 0x08), 0x4f);

    /* enable logical device 8 */
    outb(0x07, 0x4e);
    outb(0x08, 0x4f);

    /* trigger Watchdog by toggling Bit4 of logical Device 8 */
    outb(0xf1, 0x4e);

    while(!terminate_proc)
      {
        outb((inb(0x4f) ^ 0x08), 0x4f);
        usleep(100000);
      }

    /* enable logical device 9 */
    outb(0x07, 0x4e);
    outb(0x09, 0x4f);

    /* set GP33 (Bit4) value in cofiguration register f1h to 0
       to disable watchdog function */
    outb(0xf1, 0x4e);
    outb((inb(0x4f) & ~0x08), 0x4f);

    return 0;
  }

