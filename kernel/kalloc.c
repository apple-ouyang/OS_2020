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
void myKfree(void *pa, int cpuid);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++)
  {
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
  printf("kinit done\n");
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  int all = ((uint64)pa_end-(uint64)pa_start)/PGSIZE;
  printf("all pages are %d\n", all);
  int aver = all/NCPU;
  int i = 0, cnt = 0;
  while (1)
  {
    if(cnt == aver) ++i, cnt = 0;
    if(i == NCPU-1) break;
    myKfree(p, i);
    p += PGSIZE;
    ++cnt;
  }
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    myKfree(p, NCPU-1);
}

void
myKfree(void *pa, int cpuid)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("myKfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpuid].lock);
  r->next = kmem[cpuid].freelist;
  kmem[cpuid].freelist = r;
  release(&kmem[cpuid].lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int i = cpuid();
  acquire(&kmem[i].lock);
  // printf("get lock%d\n",i);
  r->next = kmem[i].freelist;
  kmem[i].freelist = r;
  release(&kmem[i].lock);
  // printf("free lock%d\n",i);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int i = cpuid();
  acquire(&kmem[i].lock);
  r = kmem[i].freelist;
  if(r)
    kmem[i].freelist = r->next;
  else{
    // printf("find i:%d\n",i);
    for (int j = 0; j < NCPU; j++)
      if(j!=i && kmem[j].lock.locked == 0){
        // printf("find in j:%d\n", j);
        acquire(&kmem[j].lock);
        // printf("get lock j:%d\n", j);
        r = kmem[j].freelist;
        if(r){
          // printf("get mem i:%d in j:%d\n", i, j);
          kmem[j].freelist = r->next;
          release(&kmem[j].lock);
          // printf("releas lock j:%d\n", j);
          break;
        }
        release(&kmem[j].lock);
        // printf("releas lock j:%d\n", j);
    } 
  }
  release(&kmem[i].lock);
  // printf("free lock i:%d\n",i);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
