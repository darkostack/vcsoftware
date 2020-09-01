#include <stdio.h>

#include <vcdrivers/stdiobase.h>

int main(void)
{
    vcstdio_init(NULL);

    printf("vcdrivers-%s started\r\n", VCDRIVERS_VERSION);

    while (1)
    {

    }

    return 0;
}
