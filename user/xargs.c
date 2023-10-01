#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
    // 核心：构造exec的参数
    // argv：
    // 源：   上一个命令的输出 | xargs 需要运行的命令X X的参数...
    // 修改为：需要运行的命令X X的参数... 上一个命令的输出
    char *p[MAXARG];
    int i;
    for (i = 1; i < argc; i++)
    {
        p[i - 1] = argv[i];
    }
    p[argc - 1] = malloc(512);
    p[argc] = 0;
    while (gets(p[argc - 1], 512))
    { // gets函数一次读取一行
        if (p[argc - 1][0] == 0)
            break;                                        // 已读完
        if (p[argc - 1][strlen(p[argc - 1]) - 1] == '\n') // 该函数会将末尾换行保留，故需去掉换行符。
            p[argc - 1][strlen(p[argc - 1]) - 1] = 0;
        if (fork() == 0)
        {
            // printf("%s\n", argv[1]);
            exec(argv[1], p);
        }
        else
            wait(0);
    }
    exit(0);
}
