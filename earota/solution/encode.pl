#!/usr/bin/perl

use strict;
use warnings;

use bigint;


my $data;
open (IN, $ARGV[0]) or die 'Unable to open file: ', $!, "\n";
{
    local $/ = undef;
    $data  = <IN>;
}
close IN;

my $bytes = 0;
my $dataint = 0;
for my $b (unpack('C*', $data)) {
    $dataint *= 256;
    $dataint += $b;

    $bytes++;
    if ($bytes % 100000 == 0) {
        warn 'Done ', $bytes, ' bytes', "\n";
    }
}

warn 'dataint found', "\n";

my @trits;

while ($dataint > 0) {
    my $r = $dataint % 3;
    $dataint = ($dataint - $r) / 3;
    if ($r == 0) {
        unshift @trits, '0';
    } elsif ($r == 1) {
        unshift @trits, '1';
    } else {
        unshift @trits, '-1';
    }
}

warn 'number of trits: ', scalar(@trits), "\n";
