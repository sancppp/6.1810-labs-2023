// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define NHASH (13)
struct {
  struct spinlock lock;  // 这个锁粒度太粗了，一把大锁保护一个双向链表
  // 如何优化？哈希，分桶
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} _bcache;
struct {
  // 将双向链表改成桶哈希
  struct spinlock lock[NHASH];  // 每个桶一个锁
  struct buf buf[NHASH][NBUF];
  struct buf bucket[NHASH];

} bcache;

inline uint hash(uint dev, uint blockno) {
  return (dev * 1000 + blockno) % NHASH;
}

void binit(void) {
  struct buf *b;

  for (int i = 0; i < NHASH; i++) initlock(&bcache.lock[i], "bcache");

  // 每个桶都是一个双向链表
  for (int i = 0; i < NHASH; i++) {
    bcache.bucket[i].prev = &bcache.bucket[i];
    bcache.bucket[i].next = &bcache.bucket[i];
    for (b = bcache.buf[i]; b < bcache.buf[i] + NBUF; b++) {
      b->next = bcache.bucket[i].next;
      b->prev = &bcache.bucket[i];
      initsleeplock(&b->lock, "buffer");
      bcache.bucket[i].next->prev = b;
      bcache.bucket[i].next = b;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf *bget(uint dev, uint blockno) {
  struct buf *b;
  uint id = hash(dev, blockno);
  acquire(&bcache.lock[id]);

  // Is the block already cached?
  for (b = bcache.bucket[id].next; b != &bcache.bucket[id]; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (b = bcache.bucket[id].prev; b != &bcache.bucket[id]; b = b->prev) {
    if (b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *bread(uint dev, uint blockno) {
  struct buf *b;

  b = bget(dev, blockno);
  if (!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b) {
  if (!holdingsleep(&b->lock)) panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
  if (!holdingsleep(&b->lock)) panic("brelse");
  uint id = hash(b->dev, b->blockno);
  releasesleep(&b->lock);

  acquire(&bcache.lock[id]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.bucket[id].next;
    b->prev = &bcache.bucket[id];
    bcache.bucket[id].next->prev = b;
    bcache.bucket[id].next = b;
  }

  release(&bcache.lock[id]);
}

void bpin(struct buf *b) {
  uint id = hash(b->dev, b->blockno);
  acquire(&bcache.lock[id]);
  b->refcnt++;
  release(&bcache.lock[id]);
}

void bunpin(struct buf *b) {
  uint id = hash(b->dev, b->blockno);
  acquire(&bcache.lock[id]);
  b->refcnt--;
  release(&bcache.lock[id]);
}
