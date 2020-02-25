The basic solution is to enter your name as a file - eg, /home/ctf/flag.txt

Your name is stored at 0x08041140, the highscores filename is 0x0804d1f9

If you jump to level 101 using the following password:

```
  **
* * *
***
*     *
    ***
  * * *
   **
```

Level 101's pellet layout looks something like this:

```
        .  . .. . . ...  .
 .... .         .   .     .
      .           ....
    .      .    ....   .. .
  .................. . .
 . . .. .     .      .   ..
....        ......  ......
...
        1111100111010001000
0010000001000        .
                        .


 ..  .   .. .  ............
......   . .  . . ...
.      .          .
                 ..  .  ..
. .  ................ .. ..
  .. . .  ................
..  .  .. . .  ............
....                ....  .
```

In the middle, where I switched to binary, that's the filename pointer;
`0x0804d1f9` in little endian is `11111001110100010000010000001000`

To change the filename pointer (`0x0804d1f9`) to the name pointer(`0x08041140`)
requires flipping some bits to zero:

```
FILE: 0x0804d1f9 => 11111001110100010000010000001000
NAME: 0x08041140 => 01000000000100010000010000001000
            Flip => ^ ^^^  ^^^
```

So you have to set your name to the file you want to read, avoid the ghosts, and
gobble up those 7 bits. Then just quit with 'q'.

Simple! :)
