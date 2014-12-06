#!/bin/bash

if [ $# -lt 1 ]
   then cat >&2 <<EOF

Usage: $0 SOURCES...

Extract Gambas help comments from all SOURCES and output a .help file.
If SOURCES contains directories, they are searched recursively for .c
and .cpp files.

EOF
   exit 1
fi

function doit {
  for arg in ${@:1}
  do echo Processing $arg... >&2
     if [ -d "$arg" ]
        then doit $(find -H $arg -name \*.c -or -name \*.cpp)
        else $(dirname $0)/extract.awk <$arg | sed "s/^[ ]*\*/\'/" |
             $(dirname $0)/xlate.sh $arg | $(dirname $0)/mkhelp.gbs3
     fi
  done
}

doit ${@:1}
