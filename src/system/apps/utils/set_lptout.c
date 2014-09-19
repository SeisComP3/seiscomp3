#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

/* Sets LPT1 output pins, used for sensor recentering */

/* Run the program as root, otherwise you get "segmentation fault" */

const char *const usage = "Usage: set_lptout <n>\n";

int main(int argc, char **argv)
  {
    unsigned int n;
    char *tail;
    ioperm(0x378, 1, 1);

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

    outb(n, 0x378);
    return 0;
  }

