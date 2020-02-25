This is just a one-byte-shift, so add one to each byte in your command. This command will run `cat /home/ctf/flag.txt`:

```
dbu!0ipnf0dug0gmbh/uyu
```

And it responds the same way:

```
BSEzxd`g^ats^xnt^g`c^sgd^rntqbd|
```

Which decodes to the flag:

```
irb(main):007:0> 'BSEzxd`g^ats^xnt^g`c^sgd^rntqbd|'.bytes.map {|b| (b+1).chr}.join
=> "CTF{yeah_but_you_had_the_source}"
```
