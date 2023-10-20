// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[];  // first address after kernel.
                    // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

#define PGSUM (PHYSTOP / PGSIZE)
struct {
  struct spinlock lock;
  volatile int count[PGSUM];
} refer_counter;

void kinit() {
  initlock(&kmem.lock, "kmem");
  initlock(&refer_counter.lock, "refer_counter");
  freerange(end, (void *)PHYSTOP);
  printf("kinit done.\n");
}

void freerange(void *pa_start, void *pa_end) {
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE) kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// 因为不止一处调用了kfree，所以引用计数的维护应该在kfree内部
void kfree(void *pa) {
  struct run *r;
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  // printf("%d\n", refer_count((uint64)pa, -1));
  if (refer_count((uint64)pa, -1) > 0) {
    return;
  }
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;  // 为什么这样是对的？

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) {
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) {
    kmem.freelist = r->next;
    // 在这里将r的refer_count设为1，而不是+=1！！！！
    acquire(&refer_counter.lock);
    refer_counter.count[(uint64)r / PGSIZE] = 1;
    release(&refer_counter.lock);
  }
  release(&kmem.lock);

  if (r) memset((char *)r, 5, PGSIZE);  // fill with junk

  return (void *)r;
}
// 操作指针r指向的物理页引用计数。
// 返回更改后的引用计数，如果delte为0的话返回此时的引用计数
int refer_count(uint64 r, int delta) {
  int res = 0;
  acquire(&refer_counter.lock);
  refer_counter.count[r / PGSIZE] += delta;
  res = refer_counter.count[r / PGSIZE];
  release(&refer_counter.lock);
  return res;
}