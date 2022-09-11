#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <hiredis/hiredis.h>

#include "destor.h"

int main(void) {

    int iret = 0;

    while (1)
    {
        iret = system("./destor");
        if(iret == -1) {
            printf("%d: %s\n", errno, strerror(errno));
        }
    }

    return 0;
}