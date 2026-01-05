# ---------- 基本连接 ----------
target remote :1234
file ./build/grub/kernel.elf
set confirm off
set architecture i386:x86-64
handle SIGTRAP stop print

# ---------- 关键断点 ----------
# 1. 向量 32（8254 中断）汇编入口
b irq0                      # 你的 idt.S 里 IRQ   0,32 对应的符号
commands
  echo >>> IRQ0 entered <<<\n
  info registers
  x/10i $rip
  c
end

# 2. （可选）C 层 handler
b timer_intr                # 如果 1 能断到而 2 不能，说明汇编→C 传参崩了
commands
  echo >>> timer_intr() <<<\n
  bt
  c
end

b isr6
commands
  echo >>> #UD at RIP=0x
  p/x $rip
  bt
  c
end

# ---------- 让机器跑起来 ----------
continue