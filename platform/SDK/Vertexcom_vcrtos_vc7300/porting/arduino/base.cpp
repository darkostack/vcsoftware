#include <arduino/base.hpp>

void pinMode(int pin, int mode)
{
    vcgpio_mode_t m = GPIO_OUT;

    if (mode == INPUT)
    {
        m = GPIO_IN;
    }
    else if (mode == INPUT_PULLUP)
    {
        m = GPIO_IN_PU;
    }

    vcgpio_init(arduino_pin_map[pin], m);
}

void digitalWrite(int pin, int state)
{
    vcgpio_write(arduino_pin_map[pin], state);
}

int digitalRead(int pin)
{
    if (vcgpio_read(arduino_pin_map[pin]))
    {
        return HIGH;
    }
    else
    {
        return LOW;
    }
}

void delay(unsigned long msec)
{
    (void) msec;
}

void delayMicroseconds(unsigned long usec)
{
    (void) usec;
}

unsigned long micros()
{
    return 0;
}

int analogRead(int pin)
{
    (void) pin;
    return 0;
}
