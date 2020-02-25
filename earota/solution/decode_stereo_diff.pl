#!/usr/bin/perl

use warnings;
use strict;

my $l_samp;
open (my $ldata, $ARGV[0]) or die 'Can not open mono audio samples file ', $ARGV[0], ': ', $!, "\n";
{
    local $/ = undef;
    $l_samp  = <$ldata>;
}
close $ldata;


my $r_samp;
open (my $rdata, $ARGV[1]) or die 'Can not open mono audio samples file ', $ARGV[1], ': ', $!, "\n";
{
    local $/ = undef;
    $r_samp  = <$rdata>;
}
close $rdata;

warn 'length of l_samp file: ', length($l_samp), "\n";
warn 'length of r_samp file: ', length($r_samp), "\n";

my @r_samps = unpack('s*', $r_samp);
my @l_samps = unpack('s*', $l_samp);

for (my $i = 0; $i < scalar(@r_samps); $i++) {

    warn 'got ', $i, ' sample pair: ', $l_samps[$i], ', ', $r_samps[$i], "\n";

    my $d = $l_samps[$i] - $r_samps[$i];

    if ($d == 0) {
        print '0';
    } elsif ($d == 1) {
        print '1';
    } elsif ($d == -1) {
        print '2';
    } else {
        die 'got bogus diff: ', $d, "\n";
    }

}
print "\n";
