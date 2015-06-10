#!/usr/bin/python
#AS5045 demo script (MOSI not connected!)
from __future__ import print_function
import uakeh
import sys
import os
import struct
import time

gp_config_str = ["gp setcfg a 4 af pp 10",
                 "gp setcfg a 5 af pp 10"]

spi_cfg = "spi cfg 100 b 8 1 0"
as_read_cmd = "spi xfer 3 0x00 0x00 0x00"

period = 0.05

usage_str = "Usage: {:s} <serial_dev>"

def init_uakeh(ser):
    for s in gp_config_str:
        uakeh.write_waitok(ser, s)
    uakeh.write_waitok(ser, spi_cfg)

def as_2_py(sh, sm, sl):
    l = int(sl, 16)
    h = int(sh, 16)
    m = int(sm, 16)
    val = l | (m << 8) | (h << 16)

    val = (val >> 5) & 0x3ffff
    ang = (val >> 6)
    even = 0

    for b in range(0, 18):
        if 1 & (val >> b):
            even = even ^ 1
    if even == 1:
        return -1, -1
    err = (val >> 1) & 0x1f
    return ang, err

def as_read(ser):
    uakeh.write(ser, as_read_cmd)
    data = uakeh.read(ser)
    data_s = data.split(' ')
    ang = as_2_py(data_s[0], data_s[1], data_s[2])

    return ang

def clear_screen():
#   cursor off = for on l -> h
    print("\x1B[?25l")
    print("\033[2J")

def print_loc(x,y,s):
    if((x is not None) and (y is not None)):
       print("\033[" + str(y) +";" + str(x)+"H",end="")
    print(s,end="")

try:
    ser = uakeh.open(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

init_uakeh(ser)
clear_screen()

par_err = 0
while(True):
    ang, err = as_read(ser)
    if (ang == -1):
        par_err = par_err + 1
    print_loc(1, 1, "ang: {:05d} err:{:x}".format(ang, err))
    print_loc(1, 2, "par err: {:d}".format(par_err))

os.system("reset")
