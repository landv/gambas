#!/usr/bin/python

n = 500000
x = 0.2

def t(x):
    mu = 10.0
    pu = 0.0
    pol = [0] * 100
    r = range(0,100)

    for i in range(0,n):
        for j in r:
            pol[j] = mu = (mu + 2.0) / 2.0
        su = 0.0
        for j in r:
            su = x * su + pol[j]
        pu = pu + su
    return pu

for i in range(2):
  print t(x)

