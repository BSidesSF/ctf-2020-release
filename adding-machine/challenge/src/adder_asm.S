.intel_syntax noprefix
.text
.global __libc_lookup_pwd
__libc_lookup_pwd:
  xchg rsp, rax
  ret
  xchg rsp, rdx
  ret
  xchg rsp, rbp
  ret
  xchg rsp, rdi
  ret
  xchg rsp, rsi
  ret
  pop rdi
  ret
  pop rsi
  ret
  pop rdx
  ret
  pop rax
  ret
  sub rsp, 0x400
  ret
