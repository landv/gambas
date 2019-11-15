#! /bin/bash

gbc3 -g ./ 2>&1
gba3 ./
gbr3 -s TestMe ./gb.test.gambas
