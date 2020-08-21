#include <vcdrivers/periph/gpio.h>

int vcgpio_init(vcgpio_t pin, vcgpio_mode_t mode)
{
    (void) pin;
    (void) mode;
    return (mode >= GPIO_OUT) ? 0 : -1;
}

int vcgpio_read(vcgpio_t pin)
{
    (void) pin;
    return 0;
}

void vcgpio_set(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_clear(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_toggle(vcgpio_t pin)
{
    (void) pin;
}

void vcgpio_write(vcgpio_t pin, int value)
{
    (void) pin;
    (void) value;
}
