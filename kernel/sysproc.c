#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz; 
  // if(growproc(n) < 0)
  //   return -1;
  if (n < 0) {
    uvmdealloc(myproc()->pagetable, addr, addr + n); // n<0立即释放
  }
  myproc()->sz += n; // 懒分配
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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

// trace 系统调用
uint64
sys_trace(void)
{
  int mask;
  if (argint(0, &mask) < 0)  // 读取进程的trapframe 获得mask参数 a0寄存器
    return -1;
  myproc()->syscall_trace = mask;
  return 0;
}


// sysinfo 系统调用
uint64
sys_sysinfo(void) {
  // 从用户态读取一个指针，用来存放sysinfo结构缓冲区
  uint64 addr;
  if (argaddr(0, &addr) < 0) {
    return -1;
  }

  struct sysinfo syinfo;
  syinfo.freemem = count_free_mem(); // 在kalloc.c
  syinfo.nproc = count_process(); // 在proc.c
  
  if (copyout(myproc()->pagetable, addr, (char*)&syinfo, sizeof(syinfo)) < 0) {
    return -1;
  }
  return 0;
}