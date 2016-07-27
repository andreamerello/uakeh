#!/usr/bin/python
#lmx2322 freq set
# CK PA5
# DI PA7
# LOAD PA4
# EN PA3

from __future__ import print_function
from uakeh import Uakeh
import sys
import os
import struct
import time

gp_config_str = ["gp setcfg a 4 out pp 10",
                 "gp setcfg a 5 out pp 10",
                 "gp setcfg a 7 out pp 10",
                 "gp setcfg a 3 out pp 2"]

gpio_write_str = "gp wr {:s} {:d}"
usage_str = "Usage: {:s} <serial_dev>"


def init_uakeh(uakeh):
    for s in gp_config_str:
        uakeh.write_waitok(s)

def set_gpio(uakeh, g, val):
    cmd = gpio_write_str.format(g, val)
    uakeh.write_waitok(cmd)

def write(uakeh, word):
    assert(len(word) == 18)
    set_gpio(uakeh, "a 5", 0) # ck low
    set_gpio(uakeh, "a 4", 1) # CS
    time.sleep(0.01)
    set_gpio(uakeh, "a 4", 0) # CS
    time.sleep(0.01)
    for bit in word:
        push_bit(uakeh, bit)

    set_gpio(uakeh, "a 4", 1) # CS
    time.sleep(0.01)

def push_bit(uakeh, bit):
    set_gpio(uakeh, "a 5", 0) # ck low
    time.sleep(0.01)
    set_gpio(uakeh, "a 7", bit) # data
    time.sleep(0.01)
    set_gpio(uakeh, "a 5", 1) # ck high
    time.sleep(0.01)

def enable(uakeh):
    set_gpio(uakeh, "a 3", 1) # enable

def calc_a_b(N):
    P = 32
    B = N / P;
    A = N - (B * P);
    return (A, B)

def set_N(uakeh, n):
    a, b = calc_a_b(n)
    word = []
    for i in reversed(range(10)):
        bit = (b >> i) & 1
        word.append(bit)
    for i in reversed(range(5)):
        bit = (a >> i) & 1
        word.append(bit)

    word.append(0) #cnt_rst
    word.append(0) #pwdn
    word.append(0) # select N

    write(uakeh, word)

def set_R(uakeh, r):
    #       X  X  X tst rs pol cptri
    word = [0, 0, 0, 0, 0, 0, 0]

    for i in reversed(range(10)):
        bit = (r >> i) & 1
        word.append(bit)
    word.append(1) # select R

    write(uakeh, word)

if len(sys.argv) != 2:
    print (usage_str.format(sys.argv[0]))
    exit(-1)

try:
    uakeh = Uakeh(sys.argv[1])
except:
    print("cannot open serial dev")
    exit(-2)

init_uakeh(uakeh)
set_R(uakeh, 317)
set_N(uakeh, 31800)
enable(uakeh)
