#include <unistd.h>
#include <sys/io.h>

/* Programming the long time watchdog timer of the Super I/O chip
   (SMSC FDC37B72x) */

/* Run the program as root, otherwise you get "segmentation fault" */

void smcw(int regidx, int data)
  {
    outb(0x55, 0x370);
    outb(regidx, 0x370);
    outb(data, 0x371);
    outb(0xaa, 0x370);
  }

int main()
  {
    ioperm(0x370, 2, 1);   /* ask for access to 0x340 and 0x341 */
    ioperm(0x201, 1, 1);   /* ask for access to 0x201 */
    
    smcw(0x07, 0x08);      /* select functional group 8 */
    smcw(0xf1, 0x80);      /* timer unit in seconds */
    smcw(0xf2, 0x20);      /* time-out in 32 units (seconds) */
    smcw(0xf3, 0x01);      /* WDT is reset upon I/O read/write of port 0x201 */
    smcw(0xe2, 0x8a);      /* hardware reset upon WDT time-out */

    while(1)
      {
        outb(0x01, 0x201); /* reset WDT */
        sleep(1);
      }

    return 0;
  }

