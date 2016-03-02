#!/usr/bin/python
#lis3lv02-SPI demo script (probe and 3-axis read)
from __future__ import print_function
from uakeh import Uakeh
import sys
import os
import struct
import time

gp_config_str = ["gp setcfg a 4 af pp 10",
                 "gp setcfg a 5 af pp 10",
                 "gp setcfg a 7 af pp 10"]
spi_cfg = "spi cfg 100 b 8 1 1"
lis_start_cmd = "spi xfer 2 0x20 0xc7"
lis_id_cmd = "spi xfer 2 0x8f 0x00"
lis_read_cmd = "spi xfer 7 0xe8 0x00 0x00 0x00 0x00 0x00 0x00"
lis_id = 0x3a

max_history = 80
scale = 10
period = 0.005

usage_str = "Usage: {:s} <serial_dev>"

def init_uakeh(uakeh):
    for s in gp_config_str:
        uakeh.write_waitok(s)
    uakeh.write_waitok(spi_cfg)

def lis_probe(uakeh):
    uakeh.write(lis_id_cmd)
    resp = uakeh.read()
    resp_id = resp.split(' ')[1]
    return (int(resp_id, 16) == lis_id)

def lis_start(uakeh):
    uakeh.write(lis_start_cmd)
    uakeh.read()

def lis_2_py(sl, sh):
    l = int(sl, 16)
    h = int(sh, 16)
    val = l | (h << 8)
    return struct.unpack("h",struct.pack("H", val))[0]

def lis_read(uakeh):
    uakeh.write(lis_read_cmd)
    data = uakeh.read()
    data_s = data.split(' ')

    x = lis_2_py(data_s[1], data_s[2])
    y = lis_2_py(data_s[3], data_s[4])
    z = lis_2_py(data_s[5], data_s[6])

    return (x, y, z)

def clear_screen():
#   cursor off = for on l -> h
    print("\x1B[?25l")
    print("\033[2J")

def print_loc(x,y,s):
    if((x is not None) and (y is not None)):
       print("\033[" + str(y) +";" + str(x)+"H",end="")
    print(s,end="")

def plot_trace(history, ybase):
    for i, d in enumerate(history):
        y = 1 + scale / 2 + d * scale / 4096
        for j in range(0, scale + 1):
            if (j == y):
                continue
            print_loc(i, j + ybase, " ")
        print_loc(i, y + ybase, "*")

if len(sys.argv) != 2:
    print (usage_str.format(sys.argv[0]))
    exit(-1)

try:
    uakeh = Uakeh(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

init_uakeh(uakeh)
if False == lis_probe(uakeh):
    print("Lis3lv02 not detected!");
    exit(-3)

lis_start(uakeh)
clear_screen()

history_x = []
history_y = []
history_z = []

while(True):
    x, y, z = lis_read(uakeh)
    print_loc(1, 1, "X: {:05d}, Y: {:05d}, Z: {:05d}".format(x, y, z))

    if (len(history_x) > max_history):
        history_x.pop(0)
        history_y.pop(0)
        history_z.pop(0)

    history_x.append(x)
    history_y.append(y)
    history_z.append(z)
    plot_trace(history_x, 2 )
    plot_trace(history_y, 4 + scale)
    plot_trace(history_z, 8 + scale * 2)

    time.sleep(period)

os.system("reset")
