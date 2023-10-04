# Lab3

## 读xv6手册的第三组
最重要的概念，在课上讲了，着重看手册中的代码介绍。
3.4：xv6维护一个可用分配内存的链表，每次申请/释放4096b，申请时将这块内存指针从链表中移除。
3.5：3.4的代码实现，kalloc.c中的函数。先忽略锁的存在，第六章会讲。

PTE：page table entries，页表条目，每一个PTE包含物理页码以及flag

## 1.加速getpid
通过用户空间、内核空间的共享只读区域，与进程绑定
修改proc结构体，维护pid。在虚拟地址空间的最高位第三页增加一个usyscall
![Alt text](image.png)
1. 在proc结构体中引入usyscall指针
2. 在allocproc函数中为usysall申请一个页
3. 在proc_pagetable函数中，建立内存映射
4. 完善free的逻辑

## 2.Print a page table
递归打印
PTE2PA宏，将一个PTE转换成PA