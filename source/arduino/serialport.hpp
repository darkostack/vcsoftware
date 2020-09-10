/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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
