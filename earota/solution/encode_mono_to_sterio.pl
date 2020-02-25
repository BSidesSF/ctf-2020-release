#!/usr/bin/perl

use strict;
use warnings;

use bigint;


my $audio;
open (my $data, $ARGV[0]) or die 'Can not open mono audio samples file ', $ARGV[0], ': ', $!, "\n";
{
    local $/ = undef;
    $audio  = <$data>;
}
close $data;
my @samples = unpack('s*', $audio);


my $trinary;
open (my $tri, $ARGV[1]) or die 'Can not open trinary file ', $ARGV[1], ': ', $!, "\n";
{
    local $/ = undef;
    $trinary  = <$tri>;
}
close $tri;

$trinary =~ s/[^012]//g;
my @trits = split(//, $trinary);


open (my $left, '>', 'left.bin') or die 'Unable to open left output: ', $?, ' ', $!, "\n";
open (my $right, '>', 'right.bin') or die 'Unable to open left output: ', $?, ' ', $!, "\n";

if (scalar(@trits) > scalar(@samples)) {
    die 'Not enough samples to fit all the trits', "\n";
}


for (my $i = 0; $i < scalar(@samples); $i++) {

    my $sl = $samples[$i];
    my $sr = $sl;

    if ($i < scalar(@trits)) {

        my $t = $trits[$i];

        if ($t eq '0') {
            # do nothing
        } elsif ($t eq '1') {
            # we want to add 1 to r if we can
            if ($sr < 0x7FFF) {
                $sr += 1;
            } else {
                $sl -= 1;
            }
        } elsif ($t eq '2') {
            # we want to subtract 1 from r if we can
            if ($sr > -0x8000) {
                $sr -= 1;
            } else {
                $sl += 1;
            }
        } else {
            die 'Not a trit: ', $t, "\n";
        }
    }

    print $left pack('s', $sl);
    print $right pack('s', $sr);
}

close $left;
close $right;
