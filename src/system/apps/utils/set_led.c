#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

/* Sets digital output pins, used by the status LED */

/* Run the program as root, otherwise you get "segmentation fault" */

const char *const usage = "Usage: set_led <n>\n";

int main(int argc, char **argv)
  {
    unsigned int n;
    char *tail;
    ioperm(0x300, 4, 1);

    if(argc != 2)
      {
        printf(usage);
        return 1;
      }

    n = strtoul(argv[1], &tail, 0);
    if(*tail || n > 0xff)
      {
        printf(usage);
        return 1;
      }

    outb(0x82, 0x303);
    outb(n, 0x300);
    return 0;
  }

