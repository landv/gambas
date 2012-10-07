#!/usr/bin/perl -w

use strict;

my $MAXITER = 50;
my $LIMIT = 4;

sub mandelbrot($$)
{
  my $w = $_[0];
  my $h = $_[1];
  my $xmin = -1.5;
  my $ymin = -1;
  my $invN = 2/$w;

  my $checknext=1;

  for my $y (0..$h-1) {

    my $Ci = $y * $invN + $ymin;

    X:
    for my $x (0..$w-1) {

      my $Zr = 0;
      my $Zi = 0;
      my $Tr = 0;
      my $Ti = 0;

      my $Cr = $x * $invN + $xmin;

      if ($checknext) {

	  for (1..$MAXITER) {
	    $Zi = 2 * $Zr * $Zi + $Ci;
	    $Zr = $Tr - $Ti + $Cr;
	    $Ti = $Zi * $Zi;
	    $Tr = $Zr * $Zr;

	    if ($Tr + $Ti > $LIMIT) {
	      print "0";
	      next X;
	    }
	  }

	  print "1";
	  $checknext = 0;

      } else {

	for (1..$MAXITER) {
	  $Zi = 2 * $Zr * $Zi + $Ci;
	  $Zr = $Tr - $Ti + $Cr;
	  $Ti = $Zi * $Zi;
	  $Tr = $Zr * $Zr;
	}

	if ($Tr+$Ti < $LIMIT) {
	  print "1";
	} else { # $Tr+$Ti is a large number or overflow ('nan' or 'inf')
	  print "0";
	  $checknext = 1;
	}

      }

    }

    print "\n";
  } 
}

for(1..50) {
  mandelbrot(200, 200);
}
