#!/usr/bin/perl -w

my $str='abcdefgh'.'efghefgh';
my $imax = 1024 / length($str) * 512;

my $starttime=time();

my $gstr='';
my $i=0;

while($i++ < $imax+1000)
{

        $gstr.=$str;
        $gstr=~s/efgh/____/g;
        my $lngth=length($gstr);   ##     my $lngth=length($gstr);        # Perhaps that would be a slower way
        print time()-$starttime,"sec\t\t",$lngth/1024,"kb\n" unless $lngth % (1024*64); #print out every 256kb
}
