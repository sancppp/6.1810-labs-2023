# Lab1
熟悉xv6&&xv6的系统调用
[xv6中文文档](https://th0ar.gitbooks.io/xv6-chinese/content/index.html)

xv6由三个文件夹组成
- kernel，里面是内核态代码
- user，用户态代码
- mkfs，文件系统

TODO：
- 第零章中对管道的描述
- 学习gdb™

lab1需要做什么？
编写几个用户态的UNIX程序，基于已经实现好的系统调用。

1. 实现`user/sleep.c`，就是写一个main函数处理参数，然后调用sleep系统调用就行。
通过阅读编译之后生成的汇编sleep.asm，能够看到自己的代码调用sleep是将程序转到00000000000003be位置。而00000000000003be位置是sleep函数的起始位置，其具体内容是将13（SYS_SLEEP宏）写到a7寄存器中，然后执行ECALL汇编指令。
在syscall的C实现中，`num = p->trapframe->a7;`提取其a7寄存器上的宏。
而寄存器结构，在`kernel/proc.h`中，这个文件定义了上下文数据结构、CPU数据结构、trapframe、Process。模拟CPU、进程...

2. 实现`user/pingpong.c`
fork一个子进程；然后使用pipe交互数据。
xv6只提供了单向管道。
read是阻塞的,所以自带同步

3. primes.c
思路是每次拿出第一个数，一定是质数，筛掉其倍数之后通过管道传递给下一个进程。
给的提示：文件描述符有限，记得关闭不用的；记得wait

4. find.c
递归查找文件
基本上是改已有的ls.c
使用open，fstat，memmove...

5. xargs.c
将上一个的输入转换成当前的输出