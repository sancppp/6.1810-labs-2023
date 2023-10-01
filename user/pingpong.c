#include "kernel/types.h"
#include "user/user.h"

// pingpong.c

int main(int argc, char *argv[])
{
    int p2c[2], c2p[2];
    pipe(p2c); // [0]读，[1]写。注意这里是单向管道
    pipe(c2p);
    int pid = fork();
    if (!pid)
    {
        // child process
        // 关掉两个pipeline的对方一端
        close(p2c[1]);
        close(c2p[0]);

        // 接收父进程的"ping"
        char buff[32];
        read(p2c[0], buff, sizeof buff);
        printf("%d: received %s\n", getpid(), buff);

        // 向parent发信息
        write(c2p[1], "pong", 5);

        close(p2c[0]);
        close(c2p[1]);
    }
    else
    {
        // parent process
        // 关掉两个pipeline的对方一端
        close(p2c[0]);
        close(c2p[1]);

        write(p2c[1], "ping", 5);

        // 接收子进程的"pong"
        char buff[32];
        read(c2p[0], buff, sizeof buff);
        printf("%d: received %s\n", getpid(), buff);

        close(p2c[1]);
        close(c2p[0]);
    }
    exit(0);
}