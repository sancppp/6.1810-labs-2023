# Lab4

## 熟悉RISC_V汇编
看看.asm带符号的汇编，回答几个问题。
编译器的内联优化

## 回溯栈
打印出函数调用栈
难点是如何判断重点，xv6中栈帧都在一个page上，给了PGROUNDDOWN宏。
坑：指针-1实际上是移动到下一位，等价于uint64的-=8

## alarm
实现功能：限制进程使用CPU时间的基础：进程在使用CPU时定期向内核汇报。
需要实现的功能：
1. 系统调用`sigalarm(n,handler)`，进程在执行了n个CPU时钟周期后执行handler。
2. 系统调用`sigreturn()`，恢复寄存器，由handler函数调用。注意函数的返回值会传到trapframe->a0上。

时钟中断由硬件发生，可在usertrap函数中判断中断的来源。如果trap来自硬件则执行逻辑。
需要自己根据trap的调用链路，实现自己的保存栈帧，恢复栈帧...
寄存器现场保存在proc结构体中，需要自己加&&维护字段。

本质：系统调用只是简单的维护proc对象的相关字段；真正的运行机制在usertrap函数中实现。
调用链：
- 用户态程序调用sigalarm注册
- 时钟中断的发生会触发usertrap函数的对应逻辑，如果符合要求则保存现场、将epc寄存器中写入handler的地址、清空counter...维护相关变量。
- 返回到用户态，此时直接去执行handler函数，handler函数必须调用sigreturn系统调用；
- sigreturn恢复寄存器&pc。