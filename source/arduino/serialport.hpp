#ifndef ARDUINO_SERIALPORT_HPP
#define ARDUINO_SERIALPORT_HPP

#define SERIAL_BUFSIZE (128)

enum SerialFormat
{
    BIN,
    OCT,
    DEC,
    HEX
};

class SerialPort
{
public:
    explicit SerialPort(unsigned int port);

    int available(void);

    void begin(long baudrate);

    void end(void);

    size_t print(int val);

    size_t print(int val, SerialFormat format);

    size_t print(float val);

    size_t print(float val, int format);

    size_t print(char val);

    size_t print(const char *val);

    size_t println(int val);

    size_t println(int val, SerialFormat format);

    size_t println(float val);

    size_t println(float val, int format);

    size_t println(char val);

    size_t println(const char *val);

    int read(void);

    int write(int val);

    int write(const char *str);

    int write(char *buf, int len);

private:
    unsigned int _port;
};

#endif /* ARDUINO_SERIALPORT_HPP */
