#!/usr/bin/perl -w

my $str='abcdefgh'.'efghefgh';
my $imax=1024/length($str)*768;

my $starttime=time();

my $gstr='';
my $i=0;

while($i++ < $imax+1000){   #adding 1000 iterations to delay exit. This will allow to capture memory usage on last step

        $gstr.=$str;
        $gstr=~s/efgh/____/g;
        my $lngth=length($str)*$i;   ##     my $lngth=length($gstr);        # Perhaps that would be a slower way
        print time()-$starttime,"sec\t\t",$lngth/1024,"kb\n" unless $lngth % (1024*256); #print out every 256kb
}

