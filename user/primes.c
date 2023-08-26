#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 递归筛子 每一次调用是一个筛子阶段，从pleft中获取输出一个素数p，筛除p的所有倍数
// 同时创建下一个stage 的进程以及相应输入管道pright 再传入下一个。
void sieve(int pleft[2]) {
    int p;
    read(pleft[0], &p, sizeof(p));// 读第一个数
    if (p == -1) { // 代表所有数字处理完毕，退出程序
        exit(0);
    }
    printf("prime %d\n", p);

    int pright[2];
    pipe(pright); // 创建下一输出管道pright

    if (fork() == 0) {
        close(pright[1]); // 子进程只需要对输入管道pright读，额不需要写。
        close(pleft[0]);
        sieve(pright);
    }
    else { // 当前进程 筛选过程
        close(pright[0]); // 父进程只要写不需要读，关掉父进程读文件描述符
        int buf;
        while (read(pleft[0], &buf, sizeof(buf)) && buf != -1) {
            if (buf % p != 0) {
                write(pright[1], &buf, sizeof(buf));
            }
        }
        buf = -1;
        write(pright[1], &buf, sizeof(buf));
        wait(0);
        exit(0);
    }

}


int main(int argc, char* argv[]) {
    int input_pipe[2];
    pipe(input_pipe);
    if (fork() == 0) {
        //第一个stage子进程
        close(input_pipe[1]); // 关闭子进程写文件描述符
        sieve(input_pipe); // 递归
        exit(0);
    }
    else {
        // 主进程
        close(input_pipe[0]);
        int i;
        for (i = 2; i < 35; i++) {
            write(input_pipe[1], &i, sizeof(i));
        }
        i = -1;
        write(input_pipe[1], &i, sizeof(i)); // 末尾输入-1 标识输入完成
        wait(0);// 等待第一个 stage 完成。注意：这里无法等待子进程的子进程，只能等待直接子进程，无法等待间接子进程。在 sieve() 中会为每个 stage 再各自执行 wait(0)，形成等待链。
        exit(0);
    }
}