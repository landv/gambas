#!/usr/bin/python

import sys

MAXITER = 50
LIMIT = 4
out = sys.stdout;

def mandelbrot(w, h) :

  xmin = -1.5
  ymin = -1
  invN = 2.0 / w

  checknext = True

  for y in range(h) :
    Ci = y * invN + ymin
    
    for x in range(w) :
      Zr = 0.0
      Zi = 0.0
      Tr = 0.0
      Ti = 0.0
      Cr = x * invN + xmin  
      if (checknext) :
	for k in range(MAXITER) :
	  Zi = 2 * Zr * Zi + Ci
	  Zr = Tr - Ti + Cr
	  Ti = Zi * Zi
	  Tr = Zr * Zr
	  if (Tr + Ti) > LIMIT :
	    break
	if k == MAXITER :
	  out.write("1")
	else :
	  out.write("0")
	  checknext = False
      else :
	for k in range(MAXITER) :
	  Zi = 2 * Zr * Zi + Ci
	  Zr = Tr - Ti + Cr
	  Ti = Zi * Zi
	  Tr = Zr * Zr
	if (Tr + Ti) < LIMIT :
	  out.write("1")
	else :
	  out.write("0")
	  checknext = True
	  
    out.write("\n")

for i in range(50) :
  mandelbrot(200, 200)
