You can get the hex like this:

```
$ strace ./strace-me 2>&1 | cut -d'(' -f2 | cut -c3-10 | grep -v 'exited' | sed '$d' | tail -n24 | tr -d '\n'; echo ''
4354467b7468655f6c6f6e6765725f7468655f666c61675f69735f7468655f6d6f72655f696e746572657374696e675f6f75747075745f796f755f6172655f736565696e675f736f5f7468655f6c6f6e675f666c61675f69735f6c6f6e67217d
```

That decodes to the flag:

```
irb(main):002:0> ['4354467b7468655f6c6f6e6765725f7468655f666c61675f69735f7468655f6d6f72655f696e746572657374696e675f6f75747075745f796f755f6172655f736565696e675f736f5f7468655f6c6f6e675f666c61675f69735f6c6f6e67217d'].pack('H*')
=> "CTF{the_longer_the_flag_is_the_more_interesting_output_you_are_seeing_so_the_long_flag_is_long!}"
```
