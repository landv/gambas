#!/usr/bin/perl -w

use strict;

sub poly($)
{
  my $n = 500000;
  my $x = $_[0];

  my $mu = 10;
  my $pu = 0;

  my @pol;

  foreach (0 .. $n - 1) {
      foreach (0 .. 99) {
	  $pol[$_] = $mu = ($mu + 2) / 2;
      }

      my $s = 0;
      foreach (0 .. 99) {
	  $s = $x * $s + $pol[$_];
      }

      $pu += $s;
  }

  return $pu;
} 

my $res;
for (1..4) {
    $res = poly(0.2);
    print "$res\n";
} 
