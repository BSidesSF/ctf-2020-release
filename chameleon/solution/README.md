The solution for this is convoluted out of laziness. :)

There's a solve.cpp script that generates a bunch of keys. It takes about 5
seconds to do a day's worth of keys. Dump them to a file. solve.cpp is
compiled to solve.exe. It only generates keys for the current day, so after
the day it's generated it wno't be helpful.

keys-for-today.txt are the keys for today.

Then solve.rb will consume those and try to decrypt.

ruby ./solve.rb ../dist/flag.png.enc ./keys-for-today.txt > test.png

This just takes a couple seconds.
