#!/usr/bin/perl

use strict;
use warnings;

use bigint;

# d = a * x + b * y  where d is the GCD
sub euclid_gcd {
    my $a = shift;
    my $b = shift;

    if ($b == 0) {
        return ($a, 1, 0);
    }

    my ($d2, $x2, $y2) = euclid_gcd($b, $a % $b);

    my ($d, $x, $y) = ($d2, $y2, $x2 - (int($a / $b) * $y2));

    return ($d, $x, $y);

}


# a * x = b mod n
sub solvelin {
    my $a = shift;
    my $b = shift;
    my $n = shift;
    my $s = shift;

    my ($d, $x2, $y2) = euclid_gcd($a, $n);

    if ($b % $d == 0) {
        my $x0 = ($x2 * ($b / $d)) % $n;

        for my $i (0 .. $d) {
            #print 'Root: ', ($x0 + ($i * ($n / $d))) % $n, "\n";
            push @{$s}, (($x0 + ($i * ($n / $d))) % $n);
        }
    }
    else {
        print 'No solution', "\n";
    }
}


# 2x2x2 matrix det
sub matrix_det {
    my @matrix = @_;

    # ((a,b), (c, d)) = ad - bc

    return abs(($matrix[0] * $matrix[3]) - ($matrix[1] * $matrix[2]));
}


my @nums;


while (<STDIN>) {
    chomp;

    my $line = $_;

    if ($line =~ m/^\d+$/) {
        push @nums, ($line + 0);
    }
    else {
        warn 'Input did not look like a number: ', $line, "\n";
    }
}


my @dets;
my ($i, $j);

for ($i = 1; $i < scalar(@nums) - 2; $i++) {
    for ($j = $i + 1; $j < scalar(@nums) - 1; $j++) {

        my $det =
            matrix_det((($nums[$i] - $nums[0]), ($nums[$i + 1] - $nums[1]),
                        ($nums[$j] - $nums[0]), ($nums[$j + 1] - $nums[1])));

        #print 'Det: ', $det, "\n";

        push @dets, $det;
    }
}

# find gcd of all dets
my $gcd_sofar = shift @dets;
my $crap;
for ($i = 0; $i < scalar(@dets); $i++) {
    ($gcd_sofar, $crap, $crap) = euclid_gcd($gcd_sofar, $dets[$i]);

    #print 'GCD so far: ', $gcd_sofar, "\n";
}

my $modulus = $gcd_sofar;


my @a_sol;
for ($i = 0; $i < scalar(@nums) - 3; $i++) {

    my ($diff1, $diff2);

    $diff1 = $modulus - ($nums[$i + 3] - $nums[$i + 1]) % $modulus;
    $diff2 = ($nums[$i + 1] - $nums[$i + 3]) % $modulus;

    if (($nums[$i] - $nums[$i + 2]) > 0) {

        solvelin($nums[$i] - $nums[$i + 2], $diff1, $modulus, \@a_sol);
    }
    else {
        solvelin($modulus - ($nums[$i + 2] - $nums[$i]),
                 $diff2, $modulus, \@a_sol);
    }
}

# Solving for c using a's
foreach my $this_a (@a_sol) {
    for ($i = 0; $i < scalar(@nums) - 1; $i++) {
        my $mul = ($nums[$i] * $this_a) % $modulus;
        my $this_c;

        if ($nums[$i + 1] - $mul > 0) {
            $this_c = $nums[$i + 1] - $mul;
        }
        else {
            $this_c = $modulus - ($mul - $nums[$i + 1]);
        }


        my $r = check_params($this_a, $this_c, $modulus);
        if ($r == 1) {
            print 'Got ax + c = m; a: ',
            $this_a, ' c: ', $this_c, ' m: ', $modulus, "\n";
            my $x = lcg($this_a, $this_c, $modulus, $nums[-1]);
            print 'Next num 1: ', $x, "\n";
            $x = lcg($this_a, $this_c, $modulus, $x);
            print 'Next num 2: ', $x, "\n";
            exit;
        }

    }
}


sub check_params {
    my $a = shift;
    my $c = shift;
    my $m = shift;

    my $x = $nums[0];
    #print 'x0 = ', $x, "\n";
    for (my $i = 1; $i < scalar(@nums); $i++) {
        $x = lcg($a, $c, $m, $x);
        #print 'x', $i, ' = ', $x, "\n";
        if ($x != $nums[$i]) {
            #print 'Params fail at ', $i, "\n";
            return 0;
        }
    }

    print 'Params pass!', "\n";
    print 'a = ', $a, '; c = ', $c, '; m = ', $m, "\n";
    return 1;
}


sub lcg {
    my $a = shift;
    my $c = shift;
    my $m = shift;
    my $x = shift;

    return (($a * $x) + $c) % $m
}
