#!/usr/bin/perl

use strict;
use warnings;

my @byte_index = (0 .. 0xff);
my @state = (0 .. 0xff);

# This is the desired state we will find a key for
my $state_hex = '31f65648bb2e62696e2f73680080cb0153545ff7eeb03b0f05';

# pad state_hex out to 256 bytes
# I'm using ff here because it isn't contained in the desired state
$state_hex = $state_hex . ('ff' x (256 - (length($state_hex) / 2)));


# RC4 init for reference
# for (int i = 0; i < 256; i++) {
#         rc4_state[i] = i;
# }

# int j = 0;
# for (int i = 0; i < 256; i++) {
#     j = (j + rc4_state[i] + key[i % keylen]) & 0xFF;
#     uint8_t tmp;
#     tmp = rc4_state[i];
#     rc4_state[i] = rc4_state[j];
#     rc4_state[j] = tmp;
# }

my @goal_bytes = unpack('C*', pack('H*', $state_hex));
my @key;
my $i = 0;
my $j = 0;
foreach my $b (@goal_bytes) {
    # We need cur_j to be the index for the byte $b that we want
    my $cur_j = $byte_index[$b];

    # To make cur_j that the key byte must be
    my $key_b = $cur_j - ($j + $state[$i]);
    $key_b = ($key_b + 1024) & 0xff; # Make positive and then mod
    push @key, $key_b;

    # Now swap the state accordingly
    ($state[$i], $state[$cur_j]) = ($state[$cur_j], $state[$i]);
    # And track the new indexes for this pair
    $byte_index[$state[$i]] = $i;
    $byte_index[$state[$cur_j]] = $cur_j;

    #warn sprintf('bytes %02x, %02x located at %02x, %02x, respectively',
    #             $state[$i], $state[$cur_j],
    #             $byte_index[$state[$i]], $byte_index[$state[$cur_j]]), "\n";

    $j = $cur_j;
    $i++;
}

print 'key: ', (map {sprintf("%02x", $_)} @key), "\n";
