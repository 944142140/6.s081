#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find(char *path, char *target_file);

int main (int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(2, "ERROR: canshutaishaole ");
        exit(1);
    }
    char *target_path = argv[1];
    char *target_file = argv[2];
    find(target_path, target_file);
    exit(0);
}

void find(char *path, char *target_file) {
    int fd;
    // stat.h stat结构体表示文件的各种属性信息，包括 dev（文件所属磁盘设备） inode编号，文件类型type
    struct stat st; 
    // 表示目录中条目的结构体，文件或者子目录
    struct dirent de;
    char buf[512], *p;

    if ((fd = open(path,0)) < 0) {
        fprintf(2, "ERROR: cannot open %s\n", path);
        return;
    }
    //  stat.h,fsta函数用于获取文件描述符相关的文件状态信息， 存储在stat结构体st中，信息有文件类型、大小、权限等
    if (fstat(fd, &st) < 0) {
        fprintf(2, "ERROR: cannot fstat %s\n", path);
        close(fd);
        return;
    }
    // 循环读取fd对应目录中文件、子目录， 每个大小de就是一个dirent结构体大小，表示目录中一个条目（文件或者子目录）。
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        strcpy(buf, path);
        // 将p指针移动到path最后并加/ 为后续连接下一个file或者folder做准备
        p = buf + strlen(buf);
        *p++ = '/';
        // inode number 为0表示该条目无效或者为空
        if (de.inum == 0) {
            continue;
        }
        // 将file或者folder名字复制到p即path/后面 加0表终止防止影响下一个字符串
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0) {
            fprintf(2, "ERROR: STAT ERROR %s\n", buf);
        }
        
        // 根据条目类型 是file就比对 是folder就递归查找
        switch (st.type) {
        case T_FILE:
            if (strcmp(target_file, de.name) == 0) {
                printf("%s\n", buf);
            }
            break;
        case T_DIR:
            if ((strcmp(de.name, ".") != 0) && (strcmp(de.name, "..") != 0)) {
                find(buf, target_file);
            }
        }

    }
    close(fd);
    return;  


}