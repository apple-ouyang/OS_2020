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

#define hashsize 13

// struct hash_head
// {
//   struct buf* next;
//   struct buf* prev;
//   struct spinlock lock;
// };


struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf bucket[hashsize];
  struct spinlock bucket_lock[hashsize];
} bcache;

inline int hash(uint blockno){
  return blockno % hashsize;
}

void insert(struct buf* a, struct buf* c){
  struct buf* b = a->next;
  c->next = b;
  c->prev = a;
  a->next = c;
  b->prev = c;
}

inline void del(struct buf* a){
  a->next->prev = a->prev;
  a->prev->next = a->next;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for (uint i = 0; i < hashsize; i++)
  {
    initlock(&bcache.bucket_lock[i], "bcache.bucket");
    bcache.bucket[i].prev = &bcache.bucket[i];
    bcache.bucket[i].next = &bcache.bucket[i];
  }
  
  for (uint i = 0; i < NBUF; i++)
  {
    b = bcache.buf + i;
    initsleeplock(&b->lock, "buffer");
    int ha = hash(i);
    insert(&bcache.bucket[ha], b);
  }
  printf("binit done\n");
  
  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // initsleeplock(&b->lock, "buffer");
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  // }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int ha = hash(blockno);
  struct buf* headha = &bcache.bucket[ha];
  struct buf* headi;
  
  acquire(&bcache.bucket_lock[ha]);
  for (b = headha->next; b != headha; b = b->next)
  {
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket_lock[ha]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.bucket_lock[ha]);
  // Not cached; recycle an unused buffer.
  // acquire(&bcache.lock);
  for (uint i = 0; i < hashsize; i++)
  {
    headi = &bcache.bucket[i];
    acquire(&bcache.bucket_lock[i]);
    for (b = headi->next; b != headi; b = b->next)
    {
      if(b->refcnt == 0){
        if(ha == i){
          del(b);
          insert(headha, b);
        }else{
          del(b);
          release(&bcache.bucket_lock[i]);

          acquire(&bcache.bucket_lock[ha]);
          insert(headha, b);
        }

        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&bcache.bucket_lock[ha]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.bucket_lock[i]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int ha = hash(b->blockno);

  acquire(&bcache.bucket_lock[ha]);
  b->refcnt--;
  // if (b->refcnt == 0) {
    // no one is waiting for it.
    // b->tick = ticks;
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  // }
  
  release(&bcache.bucket_lock[ha]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


