#!/bin/bash

# Translate C/C++ function name to Gambas symbol name

if [ $# -ne 1 ]
   then cat >&2 <<EOF

Usage: $0 CLASSDEF

Translate C/C++ function names (from stdin) to Gambas symbol
names using the class definition in the CLASSDEF file.

EOF
    exit 1
fi

while read line
do if [ -z "$line" -o "$line" == "NULL" ]
      then echo $line
      continue
   fi
   if [[ $line == \'* ]]
      # Quoting is important here to not lose whitespace
      then echo "$line"
      continue
   fi

   # It's the symbol name already (a syntax-2 help block)?
   if [[ $line =~ ^G[\ ] ]]
      then echo "$line" | sed 's/^G //'
      continue
   fi
   # Now, we have a C/C++ function name in $line. First search the class.
   $(dirname $0)/prefix.awk -v funcname=$line <$1
   # Put all the synonyms behind.
   egrep "GB_.*$line[,)]" $1 | sed 's/^[\t]*GB_[^(]\+("\([^"]\+\)".*$/\1/' |
   tr '\n' ' '
done
