// #include "kernel/types.h"
// #include "kernel/param.h"
// #include "user/user.h"

// // getline函数读取每行命令 
// char *getline(int fd) {
//     // 静态变量全局 访问
//     static char buf[MAXARG];
//     char *p = buf;
//     for (;;) {
//         if (read(fd, p, sizeof(char)) <= 0) {
//             return 0;
//         }
//         if (*p == '\n') break;
//         ++p;
//     }
//     *p = 0;
//     return buf;
// }

// int main(int argc, char *argv[]) {
//     if (argc < 1) {
//         fprintf(2, "xargs: missing operand");
//         exit(1);
//     }

//     for (;;) {
//         char *p = getline(0);
//         if (p == 0) break;

//         if (fork() == 0) {
//             char *argv[MAXARG];
//             // argv往前移一位 把xargs移掉 将getline得到的p接到argv后面加一个0
//             for (int i = 0; i + 1 < argc; i++) 
//                 argv[i] = argv[i + 1];
//             argv[argc - 1] = p;
//             argv[argc] = 0;

//             exec(argv[0], argv);
//             fprintf(2, "exec %s failed", argv[1]);
//             exit(1);
            
//         }
//         else {
//             wait(0);
//         }
//     }
//     exit(0);
// }
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 带参数列表，执行某个程序
void run(char *program, char **args) {
	if(fork() == 0) { // child exec
		exec(program, args);
		exit(0);
	}
	return; // parent return
}

int main(int argc, char *argv[]){
	char buf[2048]; // 读入时使用的内存池
	char *p = buf, *last_p = buf; // 当前参数的结束、开始指针
	char *argsbuf[128]; // 全部参数列表，字符串指针数组，包含 argv 传进来的参数和 stdin 读入的参数
	char **args = argsbuf; // 指向 argsbuf 中第一个从 stdin 读入的参数
	for(int i=1;i<argc;i++) {
		// 将 argv 提供的参数加入到最终的参数列表中
		*args = argv[i];
		args++;
	}
	char **pa = args; // 开始读入参数
	while(read(0, p, 1) != 0) {
		if(*p == ' ' || *p == '\n') {
			// 读入一个参数完成（以空格分隔，如 `echo hello world`，则 hello 和 world 各为一个参数）
			*p = '\0';	// 将空格替换为 \0 分割开各个参数，这样可以直接使用内存池中的字符串作为参数字符串
						// 而不用额外开辟空间
			*(pa++) = last_p;
			last_p = p+1;

			if(*p == '\n') {
				// 读入一行完成
				*pa = 0; // 参数列表末尾用 null 标识列表结束
				run(argv[1], argsbuf); // 执行最后一行指令
				pa = args; // 重置读入参数指针，准备读入下一行
			}
		}
		p++;
	}
	if(pa != args) { // 如果最后一行不是空行
		// 收尾最后一个参数
		*p = '\0';
		*(pa++) = last_p;
		// 收尾最后一行
		*pa = 0; // 参数列表末尾用 null 标识列表结束
		// 执行最后一行指令
		run(argv[1], argsbuf);
	}
	while(wait(0) != -1) {}; // 循环等待所有子进程完成，每一次 wait(0) 等待一个
	exit(0);
}