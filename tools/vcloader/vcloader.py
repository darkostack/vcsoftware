#!/usr/bin/env python3

#################################################################################
# Copyright (c) 2020, Vertexcom Technologies, Inc.
# All rights reserved.
#
# NOTICE: All information contained herein is, and remains
# the property of Vertexcom Technologies, Inc. and its suppliers,
# if any. The intellectual and technical concepts contained
# herein are proprietary to Vertexcom Technologies, Inc.
# and may be covered by U.S. and Foreign Patents, patents in process,
# and protected by trade secret or copyright law.
# Dissemination of this information or reproduction of this material
# is strictly forbidden unless prior written permission is obtained
# from Vertexcom Technologies, Inc.
#
# Authors: Darko Pancev <darko.pancev@vertexcom.com>
#
#################################################################################

from __future__ import print_function

import sys
import getopt
import serial
import time
import glob
import tempfile
import os
import subprocess

try:
    from progressbar import *
    usepbar = 1
except:
    usepbar = 0

# Verbose level
QUITE = 0

CHIP_ID_STRS = {0x18041901: 'VC7300 cortex-m3'}

def mdebug(level, message):
    if QUITE >= level:
        print(message, file=sys.stderr)

class CmdException(Exception):
    pass

class CommandInterface(object):
    def open(self, aport='/dev/tty.usbserial-1420', abaudrate=115200):
        self.sp = serial.Serial(
            port=aport,
            baudrate=abaudrate,
            bytesize=8,
            parity=serial.PARITY_EVEN,
            stopbits=1,
            xonxoff=0,
            rtscts=0,
            timeout=0.5
        )

    def _wait_for_ack(self, info="", timeout=0):
        stop = time.time() + timeout
        got = None
        while not got:
            got = self.sp.read(1)
            if time.time() > stop:
                break
        if not got:
            raise CmdException("No response to %s " % info)
        ask = ord(got)
        if ask == 0xff:
            # ACK OK
            return 1
        else:
            raise CmdException("Unrecognised response 0x%x to %s" % (ask, info))

    def _skip_remap_ack(self, info="", timeout=0):
        stop = time.time() + timeout
        got = None
        while not got:
            got = self.sp.read(1)
            if time.time() > stop:
                break
        if not got:
            raise CmdException("No response from %s device" % (info))
        ask = ord(got)
        if ask != 0xff:
            return 1
        else:
            raise CmdException("Device failed remap to %s" % (info))

    def _wait_for_bytes(self, nbyte, timeout=1):
        stop = time.time() + timeout
        got = bytearray()
        while len(got) < nbyte:
            got.extend(self.sp.read(1))
            if time.time() > stop:
                break
        if len(got) < nbyte:
            raise CmdException("Bytes received %d requested %d" % (len(got), nbyte))
        return list(got);

    def cmd_ping(self):
        ping_bytes = bytes([0x00,0x00,0x00,0x00])
        self.sp.write(ping_bytes)
        self._wait_for_ack("0x00 ping failed, make sure device in boot mode")
        self._wait_for_bytes(3) # read the rest of ack bytes

    def cmd_generic(self, cmd):
        cmd_bytes = bytes([cmd])
        self.sp.write(cmd_bytes)
        return self._wait_for_ack(hex(cmd))

    def _encode_size(self, size):
        byte1 = (size >> 8) & 0xff;
        byte0 = (size >> 0) & 0xff;
        return [byte0, byte1]

    def _encode_word(self, word):
        data = []
        data.append((word >> 0) & 0xFF)
        data.append((word >> 8) & 0xFF)
        data.append((word >> 16) & 0xFF)
        data.append((word >> 24) & 0xFF)
        return data

    def _get_checksum(self, data):
        checksum = 0
        for num in data:
            checksum = (num + checksum) & 0xFF
        return [checksum]

    def _decode_word(self, data):
        for i in range(len(data)):
            data[i] = str(hex(data[i]))
        word = '0x' + ''.join([format(int(c, 16), '02x') for c in reversed(data)])
        word = int(word, 16)
        return word

    def cmd_word_read(self, addr):
        send_bytes = []
        send_bytes.append(0x01)
        send_bytes.append(0x00)
        send_bytes += self._encode_size(0x1)
        addr_bytes = self._encode_word(addr)
        chcksum_byte = self._get_checksum(addr_bytes)
        send_bytes += (addr_bytes + chcksum_byte)
        self.sp.write(bytes(send_bytes))
        self._wait_for_ack("0x01 word read failed")
        recv_bytes = self._wait_for_bytes(8)
        recv_bytes = recv_bytes[3:-1] # remove flag, size and checksum bytes
        return self._decode_word(recv_bytes)

    def cmd_word_write(self, addr, word):
        send_bytes = []
        send_bytes.append(0x02)
        send_bytes.append(0x00)
        send_bytes += self._encode_size(0x1)
        addr_bytes = self._encode_word(addr)
        word_bytes = self._encode_word(word)
        checksum_byte = self._get_checksum(addr_bytes + word_bytes)
        send_bytes += (addr_bytes + word_bytes + checksum_byte)
        self.sp.write(bytes(send_bytes))
        self._wait_for_ack("0x02 word write failed")
        self._wait_for_bytes(3) # read the rest of ack bytes

    def cmd_remap_to_flash(self, chip_id):
        if chip_id == 0x18041901:
            PMU_REMAP = 0x40048068
            PMU_REMAPPASS = 0x4004806c
            self.cmd_word_write(PMU_REMAPPASS, 0xb4a59687)
            send_bytes = []
            send_bytes.append(0x02)
            send_bytes.append(0x00)
            send_bytes += self._encode_size(0x1)
            addr_bytes = self._encode_word(PMU_REMAP)
            word_bytes = self._encode_word(0x2)
            checksum_byte = self._get_checksum(addr_bytes + word_bytes)
            send_bytes += (addr_bytes + word_bytes + checksum_byte)
            self.sp.write(bytes(send_bytes))
            self._skip_remap_ack("FLASH")
        else:
            raise CmdException("Unrecognized chip id 0x%x for flash remap" % chip_id)

    def cmd_flash_erase(self, chip_id, keep_last_2_sector=0):
        if chip_id == 0x18041901:
            if keep_last_2_sector:
                # keep last 2 sector of the flash
                send_bytes = [0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00]
                self.sp.write(bytes(send_bytes))
                self._wait_for_ack("0x06 flash erase failed", 20)
                self._wait_for_bytes(3) # read the rest of ack bytes
            else:
                # erase entire flash
                send_bytes = [0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00]
                self.sp.write(bytes(send_bytes))
                self._wait_for_ack("0x80 flash erase failed", 20)
                self._wait_for_bytes(3) # read the rest of ack bytes
        else:
            raise CmdException("Unrecognized chip id 0x%x for flash erase" % chip_id)

    def cmd_flash_write(self, addr, data):
        assert((len(data) <= 256) and ((len(data) % 4) == 0))
        send_bytes = []
        send_bytes.append(0x05) # page program command
        send_bytes.append(0x00)
        lng = int(len(data)/4)
        send_bytes += self._encode_size(lng)
        data_bytes = []
        data_bytes += self._encode_word(addr)
        for c in data:
            data_bytes.append(c)
        checksum_byte = self._get_checksum(data_bytes)
        data_bytes += checksum_byte
        send_bytes += data_bytes
        self.sp.write(bytes(send_bytes))
        self._wait_for_ack("0x05 flash write failed", 5)
        self._wait_for_bytes(3) # read the rest of ack bytes

    def flash_write(self, chip_id, addr, data):
        lng = len(data)
        if chip_id == 0x18041901:
            mdebug(5, "Writing %(lng)d bytes to start address 0x%(addr)X" %
                { 'lng': lng, 'addr': addr})

            if usepbar:
                widgets = ['Writing: ', Percentage(),' ', ETA(), ' ', Bar()]
                pbar = ProgressBar(widgets=widgets, maxval=lng, term_width=79).start()

            offs = 0
            while lng > 256:
                if usepbar:
                    pbar.update(pbar.maxval-lng)
                else:
                    mdebug(5, "Write %(len)d bytes at 0x%(addr)X" % {'addr': addr, 'len': 256})
                self.cmd_flash_write(addr, data[offs:offs+256])
                offs = offs + 256
                addr = addr + 256
                lng = lng - 256
            if usepbar:
                pbar.update(pbar.maxval-lng)
                pbar.finish()
            else:
                mdebug(5, "Write %(len)d bytes at 0x%(addr)X" % {'addr': addr, 'len': 256})
            self.cmd_flash_write(addr, data[offs:offs+lng] + bytes([0xFF] * (256-lng)))
        else:
            raise CmdException("Unrecognized chip id 0x%x for flash write " % chip_id)

    def cmd_flash_read(self, addr, lng):
        assert((lng <= 256) and ((lng % 4) == 0))
        send_bytes = []
        send_bytes.append(0x01)
        send_bytes.append(0x00)
        nwords = int(lng/4)
        send_bytes += self._encode_size(nwords)
        addr_bytes = self._encode_word(addr)
        chcksum_byte = self._get_checksum(addr_bytes)
        send_bytes += (addr_bytes + chcksum_byte)
        self.sp.write(bytes(send_bytes))
        self._wait_for_ack("0x01 flash read failed", 5)
        recv_bytes = self._wait_for_bytes(lng + 4, 5)
        recv_bytes = recv_bytes[3:-1] # remove flag, size and checksum bytes
        return bytes(recv_bytes)

    def flash_read(self, chip_id, addr, lng):
        data = bytes([])
        if chip_id == 0x18041901:
            if usepbar:
                widgets = ['Reading: ', Percentage(),' ', ETA(), ' ', Bar()]
                pbar = ProgressBar(widgets=widgets,maxval=lng, term_width=79).start()
            while lng > 256:
                if usepbar:
                    pbar.update(pbar.maxval-lng)
                else:
                    mdebug(5, "Read %(len)d bytes at 0x%(addr)X" % {'addr': addr, 'len': 256})
                data += self.cmd_flash_read(addr, 256)
                addr = addr + 256
                lng = lng - 256
            if usepbar:
                pbar.update(pbar.maxval-lng)
                pbar.finish()
            else:
                mdebug(5, "Read %(len)d bytes at 0x%(addr)X" % {'addr': addr, 'len': 256})
            data += self.cmd_flash_read(addr, lng + (256-lng))
            return data
        else:
            raise CmdException("Unrecognized chip id 0x%x for flash read " % chip_id)

def usage():
    print("""Usage: %s [-hqVeEwvXru] [-l length] [-p port] [-b baud] [-a addr] [-s n] [file.bin]
    -h          This help
    -q          Quite
    -V          Verbose
    -e          Erase partial (keep last 2 sector)
    -E          Erase entire flash
    -w          Write
    -v          Verify
    -X          Remap to flash
    -r          Read
    -u          Use sector erase instead of global erase. You need to specify the amount of sectors with '-l'
    -l length   Length of read
    -p port     Serial port (default: first USB-like port in /dev)
    -b baud     Baudrate (default 115200)
    -a addr     Target address
    -s n        Skip writing N bytes from beginning of the binary (does not affect start address)

    Example: ./vcloader.py -E -w -v example/main.bin

    To use partial erase instead of global: ./vcloader.py -e -u -w -v example/main.bin

    """ % sys.argv[0])

def read(filename):
    """Read the file to be programmed and turn it into a binary"""
    with open(filename, 'rb') as f:
        bytes = f.read()

    if bytes.startswith(b'\x7FELF'):
        # Actually an ELF file.  Convert to binary
        handle, path = tempfile.mkstemp(suffix='.bin', prefix='stm32loader')

        try:
            os.close(handle)

            # Try a couple of options for objcopy
            for name in ['arm-none-eabi-objcopy', 'arm-linux-gnueabi-objcopy']:
                try:
                    code = subprocess.call([name, '-Obinary', filename, path])

                    if code == 0:
                        return read(path)
                except OSError:
                    pass
            else:
                raise Exception('Error %d while converting to a binary file' % code)
        finally:
            # Remove the temporary file
            os.unlink(path)
    else:
        return bytes

if __name__ == "__main__":

    had_error = False

    conf = {
        'port': 'auto',
        'baud': 115200,
        'address': 0x04000000,
        'skip': 0,
        'partial_erase': 0,
        'chip_erase': 0,
        'write': 0,
        'verify': 0,
        'read': 0,
        'remap': 0,
        'len': 16,
        'fname': '',
    }

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hqVeEwvXru:l:p:b:a:s")
    except getopt.GetoptError as err:
        # print help information and exit:
        print(str(err)) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    for o, a in opts:
        if o == '-V':
            QUITE = 10
        elif o == '-q':
            QUITE = 0
        elif o == '-h':
            usage()
            sys.exit(0)
        elif o == '-e':
            conf['partial_erase'] = 1
        elif o == '-E':
            conf['chip_erase'] = 1
        elif o == '-w':
            conf['write'] = 1
        elif o == '-v':
            conf['verify'] = 1
        elif o == '-r':
            conf['read'] = 1
        elif o == '-X':
            conf['remap'] = 1
        elif o == '-p':
            conf['port'] = a
        elif o == '-b':
            conf['baud'] = eval(a)
        elif o == '-a':
            conf['address'] = eval(a)
        elif o == '-s':
            conf['skip'] = eval(a)
        elif o == '-l':
            conf['len'] = eval(a)
        else:
            assert False, "unhandled option"

    # Try and find the port automatically
    if conf['port'] == 'auto':
        ports = []

        # Get a list of all USB-like names in /dev
        for name in ['tty.usbserial', 'ttyUSB']:
            ports.extend(glob.glob('/dev/%s*' % name))

        ports = sorted(ports)

        if ports:
            # Found something - take it
            conf['port'] = ports[0]

    cmd = CommandInterface()
    cmd.open(conf['port'], conf['baud'])
    mdebug(10, "Open port %(port)s, baud %(baud)d" % {'port':conf['port'],
                                                      'baud':conf['baud']})

    cmd.cmd_ping() # Make sure connection is OK

    chip_id_num = cmd.cmd_word_read(0x4004803c)
    chip_id_str = CHIP_ID_STRS.get(chip_id_num, None)

    if chip_id_str is None:
        mdebug(0, "Warning: unrecognized chip ID 0x%x" % chip_id_num)
    else:
        mdebug(0, "Chip id 0x%x, %s" % (chip_id_num, chip_id_str))

    try:
        if conf['write'] or conf['verify']:
            mdebug(5, "%s -- reading data from %s" % (chip_id_str, args[0]))
            data = read(args[0])

            if conf['skip']:
                mdebug(5, "%s -- skipping %d bytes" % (chip_id_str, conf['skip']))
                data = data[conf['skip']:]

        if conf['partial_erase']:
            cmd.cmd_flash_erase(chip_id_num, 1)
            mdebug(0, "%s -- flash erased - keep last 2 sector - DONE" % chip_id_str)
        elif conf['chip_erase']:
            cmd.cmd_flash_erase(chip_id_num)
            mdebug(0, "%s -- flash erased - all sector - DONE" % chip_id_str)

        if conf['write']:
            mdebug(5, "%s -- writing binary" % chip_id_str)
            cmd.flash_write(chip_id_num, conf['address'], data)

        if conf['verify']:
            mdebug(5, "%s -- verifying flash" % chip_id_str)
            verify = cmd.flash_read(chip_id_num, conf['address'], len(data))
            verify = verify[:len(data)]
            if data == verify:
                print("Verification OK")
            else:
                print("Verification FAILED")
                print(str(len(data)) + ' vs ' + str(len(verify)))
                for i in range(0, len(data)):
                    if data[i] != verify[i]:
                        print(hex(i) + ': ' + hex(data[i]) + ' vs ' + hex(verify[i]))
                had_error = True

        if not conf['write'] and conf['read']:
            addr = conf['address']
            # TODO: properly print memory data
            for _ in range(conf['len']):
                print(hex(cmd.cmd_word_read(addr)))
                addr += 4

        if conf['remap']:
            print("Remap to flash")
            cmd.cmd_remap_to_flash(chip_id_num)

    finally:
        cmd.sp.close()
        mdebug(10, "Close port %(port)s" % {'port':conf['port']})

    if had_error: exit(1)

