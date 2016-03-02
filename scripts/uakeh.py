#!/usr/bin/python
import serial
import sys

usage_str = "Usage: {:s} <serial_dev>"

class Uakeh:
    def __init__(self, devname):
        self.ser = serial.Serial(port = devname, baudrate = 115200)
        self.write_waitok("echo 0")

    def read(self):
        c = 0
        s = ""
        while (True):
            c = self.ser.read(1)
            if (c == '\x0a'):
                break
            s = s + c
        return s

    def write(self, s):
        self.ser.write(s + '\x0a')

    def write_waitok(self, s):
        self.write(s)
        while (True):
            r = self.read()
            if (r == "OK!"):
                break

    def close(self):
        self.ser.close()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print (usage_str.format(sys.argv[0]))
        exit(-1)
    try:
        uakeh = Uakeh(sys.argv[1])
    except IOError:
        print("cannot open serial dev")
        exit(-2)

    uakeh.write("FWV")
    fwv = uakeh.read()
    print "UAKEH fw ver: " + fwv
