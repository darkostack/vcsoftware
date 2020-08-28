#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <arduino/base.hpp>

SerialPort::SerialPort(unsigned int port)
    : _port(port)
{
}

int SerialPort::available(void)
{
    return vcstdio_read_available();
}

void SerialPort::begin(long baudrate)
{
    (void) baudrate; /* Uart serial already initialized in vcstdio_init */
}

void SerialPort::end(void)
{
    /* can't end the uart serial, it use in stdio */
}

size_t SerialPort::print(int val)
{
    return print(val, DEC);
}

size_t SerialPort::print(int val, SerialFormat format)
{
    char buf[SERIAL_BUFSIZE];
    size_t len;

    switch (format)
    {
    case BIN:
        /* TODO */
        return 0;

    case OCT:
        len = sprintf(buf, "%o", (unsigned)val);
        break;

    case DEC:
        len = sprintf(buf, "%i", val);
        break;

    case HEX:
        len = sprintf(buf, "%x", (unsigned)val);
        break;

    default:
        return 0;
    }

    write(buf, len);

    return len;
}

size_t SerialPort::print(float val)
{
    return print(val, 2);
}

size_t SerialPort::print(float val, int format)
{
    char buf[SERIAL_BUFSIZE];
    size_t len = sprintf(buf, "%.*f", format, (double)val);
    write(buf, len);
    return len;
}

size_t SerialPort::print(char val)
{
    return (size_t)write((int)val);
}

size_t SerialPort::print(const char *val)
{
    return (size_t)write(val);
}

size_t SerialPort::println(int val)
{
    size_t res = print(val);
    write("\r\n");
    return (res + 2);
}

size_t SerialPort::println(int val, SerialFormat format)
{
    size_t res = print(val, format);
    write("\r\n");
    return (res + 2);
}

size_t SerialPort::println(float val)
{
    size_t res = print(val);
    write("\r\n");
    return (res + 2);
}

size_t SerialPort::println(float val, int format)
{
    size_t res = print(val, format);
    write("\r\n");
    return (res + 2);
}

size_t SerialPort::println(char val)
{
    size_t res = print(val);
    write("\r\n");
    return (res + 2);
}

size_t SerialPort::println(const char *val)
{
    size_t res = print(val);
    write("\r\n");
    return (res + 2);
}

int SerialPort::read(void)
{
    int tmp;
    vcstdio_read(static_cast<void *>(&tmp), 1);
    return tmp;
}

int SerialPort::write(int val)
{
    vcstdio_write((const void *)&val, 1);
    return 1;
}

int SerialPort::write(const char *str)
{
    vcstdio_write((const void *)str, strlen(str));
    return strlen(str);
}

int SerialPort::write(char *buf, int len)
{
    vcstdio_write((const void *)buf, len);
    return len;
}
