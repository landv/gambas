#!/bin/bash

# System Report script for Gambas
# Joshua Higgins
# GPL'd

# Revision 11 15/01/10

# Changed detection of distros using /etc/lsb-release, because before we were only expecting Ubuntu to be using it. Silly me - Josh
# 20/01/10 - Benoît: Integration into Gambas IDE

# echo "System Report for Gambas"

# ---------------- DETECT DISTRIBUTION VERSION

# Detection of following distro's is supported
# To add dectection of your favourite distro, format as follows:
# distroshortname:/path/to/version/file
# (generic:/etc/issue should be the LAST entry in the list, as this is the fallback)

distros="lsb-release:/etc/lsb-release vector:/etc/vector-version slackware:/etc/slackware-version debian:/etc/debian_version redhat:/etc/redhat-release arch:/etc/arch-release SuSE:/etc/SuSE-release gentoo:/etc/gentoo-release conectiva:/etc/conectiva-release mandriva:/etc/mandriva-release mandrake:/etc/mandrake-release pardus:/etc/pardus-release kanotix:/etc/kanotix-release generic-undetected:/etc/issue"

for distro in $distros
do

  path="`echo $distro  | awk -F: '{print $2}'`"
  vendor="`echo $distro | awk -F: '{print $1}'`"
  
  # Ubuntu and Mandriva now give lsb-release files, which needs the info extracting from
  
  if [ "$vendor" = "lsb-release" ]; then
    release="`cat $path 2>/dev/null | grep DESCRIPTION | awk -F= '{print $2}'`"
    # this is a bit ugly, because we overwrite the vendor variable, but I can't see any other way
    vendor="`cat $path 2>/dev/null | grep DISTRIB_ID | awk -F= '{print $2}'`"
  else
    release="`cat $path 2>/dev/null`"
  fi
  
  if [ "$release" = "" ]; then
    message="Still not here..."
    # Check if we've missed Arch
    if [ -e /etc/arch-release ]; then
      vendor="arch"
      release="n/a"
      #echo "Detected distro: $vendor"
      break
    fi
  else
    #echo "Found distro information at $path!"
    #echo "Detected distro: $vendor"
    break
  fi
  
done

# ---------------- DETECT OS DETAILS

OS=$(uname)
KERNEL=$(uname -r)

# ---------------- DETECT SYSTEM DETAILS


if [ "$OS" = "FreeBSD" ]; then
  # Added for FreeBSD RAM detection 
  RAM=$(echo `sysctl -n hw.physmem` / 1024 | bc -l | cut -d . -f1)" Kb"
else
  RAM="`cat /proc/meminfo | grep MemTotal | awk -F: '{print $2}' | sed -e 's/^[ \t]*//'`"
fi

ARCH=$(uname -m)

# ---------------- DETECT GAMBAS DETAILS

GAMBAS=$(gbx -V 2>/dev/null)
GAMBAS2=$(gbx2 -V 2>/dev/null)
GAMBAS3=$(gbx3 -V 2>/dev/null)

GAMBASPATH=$(which gbx 2>/dev/null)
GAMBAS2PATH=$(which gbx2 2>/dev/null)
GAMBAS3PATH=$(which gbx3 2>/dev/null)

# ---------------- PRINT ALL TO FILE

echo "[OperatingSystem]"
echo "OperatingSystem=$OS"
echo "KernelRelease=$KERNEL"
echo "DistributionVendor=$vendor"
echo "DistributionRelease=$release"
echo ""
echo "[System]"
echo "CPUArchitecture=$ARCH"
echo "TotalRam=$RAM"
echo ""
echo "[Gambas]"

if [ "$GAMBAS" = "" ]; then
  echo "Gambas1=Not Installed"
else
  echo "Gambas1=$GAMBAS"
  echo "Gambas1Path=$GAMBASPATH"
fi

if [ "$GAMBAS2" = "" ]; then
  echo "Gambas2=Not Installed"
else
  echo "Gambas2=$GAMBAS2"
  echo "Gambas2Path=$GAMBAS2PATH"
fi

if [ "$GAMBAS3" = "" ]; then
  echo "Gambas3=Not Installed"
else
  echo "Gambas3=$GAMBAS3" 
  echo "Gambas3Path=$GAMBAS3PATH" 
fi
