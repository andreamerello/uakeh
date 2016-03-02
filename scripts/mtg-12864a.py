#!/usr/bin/python
#mtg 12864A (2x ks0108) display demo
from __future__ import print_function
from uakeh import Uakeh
import sys
import os
import struct
import time

gpio_config_str = "gp setcfg {:s} out pp 2"
gpio_write_str = "gp wr {:s} {:d}"
gpio_data = [ "a 1",
              "a 2",
              "a 3",
              "a 4",
              "a 5",
              "a 6",
              "a 7",
              "a 8"
]

gpio_cs = [ "b 1", "b 2" ]
gpio_e = "b 0"
gpio_di = "b 10"

usage_str = "Usage: {:s} <serial_dev>"

def init_gpio(uakeh):
    for g in gpio_data + gpio_cs + [gpio_e, gpio_di]:
        cmd = gpio_config_str.format(g)
        uakeh.write_waitok(cmd)

def set_gpio(uakeh, g, val):
    cmd = gpio_write_str.format(g, val)
    uakeh.write_waitok(cmd)

def write_cmd(uakeh, data):
    set_gpio(uakeh, gpio_di, 0)
    write_byte(uakeh, data)

def write_vram(uakeh, data):
    set_gpio(uekeh, gpio_di, 1)
    write_byte(uakeh, data)

def write_byte(uakeh, data):
    set_gpio(uakeh, gpio_e, 1)

    for i in range(8):
        set_gpio(uakeh, gpio_data[i], (data >> i) & 1)
    set_gpio(uakeh, gpio_e, 0)
    time.sleep(0.002)

def lcd_on(uakeh):
    set_gpio(uakeh, gpio_cs[0], 1)
    set_gpio(uakeh, gpio_cs[1], 1)
    write_cmd(uakeh, 0x3f) # Power on
    write_cmd(uakeh, 0xc0) # set vram index to 0

def set_y(uakeh, y):
    write_cmd(uakeh, 0x80 | y)

def set_x(uakeh, x):
    write_cmd(uakeh, 0xb8 | x)

def clear(uakeh):
    set_gpio(uakeh, gpio_cs[0], 1)
    set_gpio(uakeh, gpio_cs[1], 1)
    for x in range(64 / 8):
        set_x(uakeh, x)
        for y in range(64):
            write_vram(uakeh, 0)

def pattern(uakeh):
    for x in range(64 / 8):
        set_x(uakeh, x)
        for y in range(64):
            if y % 2:
                byte = 0x55
            else:
                byte = 0xaa
            write_vram(uakeh, byte)


def test_gpio(uakeh):
    for g in gpio_data + gpio_cs + [gpio_e, gpio_di]:
        print("going to move {:s}".format(g))
        raw_input("Press enter")
        for _ in range(2):
            set_gpio(uakeh, g, 1)
            time.sleep(0.5)
            set_gpio(uakeh, g, 0)

if len(sys.argv) != 2:
    print (usage_str.format(sys.argv[0]))
    exit(-1)

try:
    uakeh = Uakeh(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

init_gpio(uakeh)

#test_gpio(uakeh)
lcd_on(uakeh)
try:
    while True:
        clear(uakeh)
        pattern(uakeh)
except KeyboardInterrupt:
    pass

for g in gpio_data + gpio_cs + [gpio_e, gpio_di]:
    set_gpio(uakeh, g, 0)
