#include <stdint.h>
#include <stdio.h>

#include <vcdrivers/stdiobase.h>

int main(void)
{
    vcstdio_init();

    printf("Hello world!\r\n");

    while(1)
    {

    }

    return 0;
}
