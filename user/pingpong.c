#include "kernel/types.h"
#include "user/user.h"

int main (int argc, char* argv[]) {
   int p1[2]; // pipe会得到一个长度2的数组，一般p[0]为用于读数据的fd p[1] 为写数据fd。
   int p2[2]; // 定义两个管道数组，用于父子进程通信
   pipe(p1); // 创建父进程向子进程通信的管道
   pipe(p2); // 创建子进程向父进程通信的管道
   char buf[1];
   int pid = fork();

   if (pid == 0) { // 子进程通信
      close(p1[1]); // 关闭父进程向子进程写通道，要读数据
      close(p2[0]); // 关闭子进程向父进程读通道

      read(p1[0], buf, 1);
      printf("%d: received ping\n", getpid());
      write(p2[1], "!", 1);
      close(p1[0]);
      close(p2[1]);
      exit(0);

   }
   else {
      close(p1[0]); // 关闭父进程向子进程写通道，要读数据
      close(p2[1]); // 关闭子进程向父进程读通道

      write(p1[1], "!", 1);
      read(p2[0], buf, 1);
      printf("%d: received pong\n", getpid());
      
      close(p1[1]);
      close(p2[0]);
      exit(0);
   }
   
}