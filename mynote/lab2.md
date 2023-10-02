# Lab2 syscall

> 实现一些系统调用

## 南京大学2022系统课的xv6入门
> 参考了[Title](https://www.bilibili.com/video/BV1DY4y1a7YD/?spm_id_from=333.337.search-card.all.click&vd_source=dc46a5fd1a28c80b30c5bba3cad6650b)

如何配置vscode : [知乎link](https://zhuanlan.zhihu.com/p/501901665)
bear工具`sudo apt install bear&&bear -- make qemu`，捕获编译中的命令&依赖...
xv6的进程内存状态相较于真正的unix很简单
内核启动的时候，运行initcode


##中断，上下文切换##:

•xv6 的系统调用实现：大家听得一头雾水，但留了个印象
• ecall 指令：跳转到trampoline 代码
• 保存所有寄存器到 trapframe
• 使内核代码能够继续执行

为什么死循环不会是OS挂&&如何写出让OS挂的代码？
OS会抢夺CPU执行权，强制syscall发生中断返回内核态执行调度。
这个过程是如何发生(上下文切换snapshot&taskswitch)的？！！！talk is cheap
**本节课的重头戏**：gdb调试上下文切换过程。
pmap命令`man pmap`
进程的组成：进程的结构，xv6中比较简单。内存结构+寄存器+文件描述符表...
最高位有两个特殊结构

（CPU虚拟化：通过将CPU划分为多个时间片，使其对外看起来有多个CPU）

如何保存现场，如何恢复现场。
保存寄存器，xv6将寄存器保存到其进程内存结构的最高位0x3f..ffe000 ，
然后，遍历proc*数组，找一个state==RUNNABLE的进程去运行

1. 使用gdb查看谁调用了syscall()，查看寄存器的值....
finished
配置好了使用vscode进行调试，本质是代理gdb
![Alt text](image.png)

## 添加一个syscall-trace

TODO：
1. 在`user/user.h`中添加函数原型。如此一来，用户态的程序可以调用trace函数
2. 在`user/usys.pl`中添加函数存根,perl语言基于这个文件生成usys.S，即用户态的汇编代码:
   ```
    .global trace
        trace:
        li a7, SYS_trace
        ecall
        ret 
   ```
3. 在`kernel/syscall.h`中添加SYS_trace宏。22
4. 在`kernel/syscall.c`中添加sys_trace函数声明
5. 在`kernel/sysproc.c`中实现sys_trace函数
此时全链路跑通

syscall的参数传递？
[Title](https://pdos.csail.mit.edu/6.828/2023/xv6/book-riscv-rev3.pdf)4.4有介绍
遵循RISC-V规范，将参数放在寄存器中。

如何实现trace？
修改proc结构，增加mask字段 + sys_trace()&&fork()维护mask + 在syscall函数中输出