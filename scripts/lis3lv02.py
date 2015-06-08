#!/usr/bin/python
#lis3lv02-SPI demo script (probe and 3-axis read)
from __future__ import print_function
import uakeh
import sys
import os

gp_config_str = ["gp setcfg a 4 af pp 10",
                 "gp setcfg a 5 af pp 10",
                 "gp setcfg a 7 af pp 10"]
spi_cfg = "spi cfg 100 b 8 1 1"
lis_start_cmd = "spi xfer 2 0x20 0xc7"
lis_id_cmd = "spi xfer 2 0x8f 0x00"
lis_read_cmd = "spi xfer 7 0xe8 0x00 0x00 0x00 0x00 0x00 0x00"
lis_id = 0x3a

usage_str = "Usage: {:s} <serial_dev>"

def init_uakeh(ser):
    for s in gp_config_str:
        uakeh.write_waitok(ser, s)
    uakeh.write_waitok(ser, spi_cfg)

def lis_probe(ser):
    uakeh.write(ser, lis_id_cmd)
    resp = uakeh.read(ser)
    resp_id = resp.split(' ')[1]
    return (int(resp_id, 16) == lis_id)

def lis_start(ser):
    uakeh.write(ser, lis_start_cmd)
    uakeh.read(ser)

def lis_read(ser):
    uakeh.write(ser, lis_read_cmd)
    data = uakeh.read(ser)
    data_s = data.split(' ')
    xl = int(data_s[1], 16)
    xh = int(data_s[2], 16)
    yl = int(data_s[3], 16)
    yh = int(data_s[4], 16)
    zl = int(data_s[5], 16)
    zh = int(data_s[6], 16)
    x = xl | (xh << 8)
    y = yl | (yh << 8)
    z = zl | (zh << 8)
    return (x, y, z)

def clear_screen():
#   cursor off = for on l -> h
    print("\x1B[?25l")
    print("\033[2J")

def print_loc(x,y,s):
    if((x is not None) and (y is not None)):
       print("\033[" + str(y) +";" + str(x)+"H",end="")
    print(s,end="")


if len(sys.argv) != 2:
    print (usage_str.format(sys.argv[0]))
    exit(-1)

try:
    ser = uakeh.open(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

init_uakeh(ser)
if False == lis_probe(ser):
    print("Lis3lv02 not detected!");
    exit(-3)

lis_start(ser)
clear_screen()
while(True):
    x, y, z = lis_read(ser)
    print_loc(1, 1, "X: {:05d}, Y: {:05d}, Z: {:05d}".format(x, y, z))

os.system("reset")
