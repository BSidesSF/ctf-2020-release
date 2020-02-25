#!/usr/bin/perl

use strict;
use warnings;

# Disable output buffering
$| = 1;

my $prompt =  'R? (enter in form [1234,5678])> ';

my $banner;
open (my $bfh, 'banner.txt') or die 'Unable to open banner file: ', $!, "\n";
{
    local $/ = undef;
    $banner  = <$bfh>;
}
close $bfh;

my $flag;
open (my $ffh, 'flag.txt') or die 'Unable to open flag file: ', $!, "\n";
{
    local $/ = undef;
    $flag  = <$ffh>;
}
close $ffh;


my $params = `gp -q gen_curve.gp`;

my ($d, $a, $b, $s, $px, $py, $rx, $ry);
if ($params =~ m/^d=(\d+); a=(\d+); b=(\d+); s=(\d+); px=(\d+); py=(\d+); rx=(\d+); ry=(\d+);\s*$/) {
    ($d, $a, $b, $s, $px, $py, $rx, $ry) = ($1, $2, $3, $4, $5, $6, $7, $8);
} else {
    die 'Unable to generate params!', "\n";
}

print $banner, "\n";
print sprintf('Curve Generated: y^2 = x^3 + %s*x + %s mod %s', $a, $b, $d), "\n";
print sprintf('Point `P` on curve: [%s,%s]', $px, $py), "\n";
print sprintf('Scalar `s`: %s', $s), "\n";
print 'Please compute `R` = `s*P`', "\n";
#print sprintf('Answer: [%s,%s]', $rx, $ry), "\n";

print "\n", $prompt;
while (<STDIN>) {
    chomp;
    my $line = $_;

    my ($ux, $uy);
    if ($line =~ m/^\s*\[\s*(\d+)\s*,\s*(\d+)\s*\]\s*$/) {
        ($ux, $uy) = ($1, $2);
    } else {
        print 'Invalid input!', "\n";
        print "\n", $prompt;
        next;
    }

    if (($ux eq $rx) && ($uy eq $ry)) {
        print 'Great!', "\n";
        print $flag, "\n";
        exit(0);
    } else {
        print 'Incorrect.', "\n";
        print "\n", $prompt;
        next;
    }

    print "\n", $prompt; # should not actually gete here
}

print "\n";
