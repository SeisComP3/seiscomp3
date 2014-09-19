#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

/* This program shuts down SeisComP when power failure occurs */

/* Run the program as root, otherwise you get "segmentation fault" */

#define N(arg) do { if((arg) < 0) { perror(#arg); exit(1); } } while(0)

int main()
  {
    int a, b, c;
    ioperm(0x320, 1, 1);

    while(1)
      {
        outb(0x13, 0x320);
        usleep(100000);
        a = inb(0x320);

        outb(0x99, 0x320);
        usleep(100000);
        b = inb(0x320);

        outb(0x02, 0x320);
        usleep(100000);
        c = inb(0x320);
        
        outb(0xff, 0x320);

        if((c & 0x0d) == 1)
          {
            system("halt");
            break;
          }

        sleep(1);
      }

    return 0;
  }
 
