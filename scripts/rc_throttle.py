#!/usr/bin/python
#interactive R/C PWM throttle for UAKEH
from __future__ import print_function
import serial
import tty
import sys
import os

throttle = 0.0
gp_config_str = "gp setcfg a 8 af pp 50"
pwm_rc_str = "pwm freq rc"
pwm_enable_str = "pwm en 1"
pwm_disable_str = "pwm en 0"
pwm_throttle_str = "pwm perc {:f}"
usage_str = "Usage: {:s} <serial_dev>"
bar_div = 2
step = 0.5

def open_ser(devname):
    ser = serial.Serial(port = devname, baudrate = 115200)
    return ser

def read_ser():
    c = 0
    s = ""
    while (True):
        c = ser.read(1)
        if (c == '\x0a'):
            break
        s = s + c

    return s

def write_ser(s):
    ser.write(s + '\x0a')
    while (True):
        r = read_ser()
        if (r == "OK!"):
            break

def init_uakeh(ser):
    write_ser(gp_config_str)
    write_ser(pwm_rc_str)
    write_ser(pwm_enable_str)

def update_throttle(t):
    global throttle
    if (throttle != t):
        write_ser(pwm_throttle_str.format(t))
    throttle = t

def clear_screen():
#   cursor off = for on l -> h
    print("\x1B[?25l")
    print("\033[2J")

def print_loc(x,y,s):
    if((x is not None) and (y is not None)):
       print("\033[" + str(y) +";" + str(x)+"H",end="")
    print(s,end="")

def ui_throttle():
    print_loc (1, 2, "Throttle {:3.2f}".format(throttle))
    bars = int(throttle / bar_div);
    spaces = 100 / bar_div - bars
    print_loc (1, 3, ":" + "=" * bars + ">" + " " * spaces)
    t = throttle
    i = sys.stdin.read(1)
    if (i == 'u') and (throttle <= (100.0 - step)):
        t = throttle + step
    elif (i == 'd') and (throttle >= step):
        t = throttle - step
    elif (i == 'q'):
        return None
    return t

if len(sys.argv) != 2:
    print (usage_str.format(sys.argv[0]))
    exit(-1)

try:
    ser = open_ser(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

tty.setraw(sys.stdin.fileno())
init_uakeh(ser)
clear_screen()
print("Use 'u' and 'd' keys, or q to quit")
while(True):
    t = ui_throttle()
    if (t == None):
        break
    update_throttle(t)
write_ser(pwm_disable_str)
os.system("reset")
