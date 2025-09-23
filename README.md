# 在控制台执行以下命令准备环境

## Ubuntu 22.04 为例
sudo apt install build-essential qemu-system-x86 nasm gdb \
                 xorriso grub-pc-bin grub2-common mtools
## 安装 64 位裸机交叉编译器（大多数发行版已自带）
x86_64-elf-gcc --version || sudo apt install gcc-mingw-w64-x86-64

# 32位模式 vs 64位模式的主要区别

1. 地址空间
32位模式：最大支持4GB内存（2³²字节）
64位模式：理论上支持2⁶⁴字节，实际实现中通常支持48位地址（256TB）
2. 寄存器
32位模式：使用EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP等32位寄存器
64位模式：扩展到64位寄存器RAX, RBX, RCX, RDX, RSI, RDI, RSP, RBP，并增加了R8-R15寄存器
3. 指令集
32位模式：使用32位指令集
64位模式：向后兼容32位指令，同时支持新的64位指令
4. 内存寻址
32位模式：使用32位地址，最多支持4GB内存
64位模式：使用64位地址，支持更大的内存空间
5. 页表结构
32位模式：使用二级或三级页表
64位模式：使用四级页表（PML4 → PDP → PD → PT）
6. 代码段
32位模式：代码段描述符设置为32位代码
64位模式：代码段描述符需要设置特定的标志位来标识64位代码