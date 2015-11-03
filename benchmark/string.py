#!/usr/bin/python

import re
import time
import sys

str = 'abcdefgh'+'efghefgh'
imax = 1024/len(str)*768

starttime = time.time();
sys.stdout.flush()

gstr = ''
i = 0

while (i < imax+1000):
  i = i + 1
  gstr += str
  gstr = re.sub('efgh','____',gstr)
  lngth = len(str) * i
  if (lngth % (1024*256) == 0):
    print int(time.time()-starttime),"sec\t\t",(lngth/1024),"kb"
    sys.stdout.flush()
