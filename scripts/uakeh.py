#!/usr/bin/python
import serial
import sys

usage_str = "Usage: {:s} <serial_dev>"

def open(devname):
    ser = serial.Serial(port = devname, baudrate = 115200)
    write_waitok(ser, "echo 0")
    return ser

def read(ser):
    c = 0
    s = ""
    while (True):
        c = ser.read(1)
        if (c == '\x0a'):
            break
        s = s + c
    return s

def write(ser, s):
    ser.write(s + '\x0a')

def write_waitok(ser, s):
    write(ser, s)
    while (True):
        r = read(ser)
        if (r == "OK!"):
            break

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print (usage_str.format(sys.argv[0]))
        exit(-1)
    try:
        ser = open(sys.argv[1])
    except:
        print("cannot open serial dev")
        exit(-2)

    write(ser, "FWV")
    fwv = read(ser)
    print "UAKEH fw ver: " + fwv
