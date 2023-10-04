#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
int sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // int pgaccess(void *base, int len, void *mask);
  struct proc *proc = myproc();
  uint64 st;
  argaddr(0, &st); // 获取user page虚拟地址
  int len = 0;
  argint(1, &len); // 获取要check的页数
  if (len > 32)
    len = 32; // 限制check最大页数

  unsigned int kmask = 0;
  for (int i = 0; i < len; i++)
  {
    pte_t *pte = walk(proc->pagetable, st + i * PGSIZE, 0);
    if (*pte & PTE_A)
    {
      *pte &= ~PTE_A; // 每次调用是返回自从上次调用之后是否被访问。所以需要置零。
      kmask |= (1 << i);
    }
  }
  uint64 mask;
  argaddr(2, &mask);
  return copyout(proc->pagetable, mask, (char *)&kmask, sizeof kmask);
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
