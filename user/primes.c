#include "kernel/types.h"
#include "user/user.h"
#define N 35

const int DEBUG = 0;

int check(char cur[])
{
    for (int i = 2; i <= N; i++)
        if (cur[i])
            return 1;
    return 0;
}

inline void myprint(char cur[])
{
    if (!DEBUG)
        return;
    for (int i = 2; i <= N; i++)
        printf("%d%c", cur[i], " \n"[i == N]);
}
inline void modify(char cur[])
{
    int prime = 0;
    for (int i = 2; i <= N; i++)
    {
        if (cur[i] == 1)
        {
            prime = i;
            break;
        }
    }
    if (prime)
    {
        if (DEBUG)
            printf("pid%d: prime %d\n", getpid(), prime);
        else
            printf("prime %d\n", prime);
        for (int i = prime; i <= N; i += prime)
            cur[i] = 0;
    }
}

void dfs(char cur[])
{
    if (check(cur) == 0)
        return;

    // 1.fork&pipeline
    // Bug:要先pipe(),再fork()!!!
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid == 0)
    {
        // child
        close(p[1]);
        char buff[N + 1] = {};
        read(p[0], buff, N + 1);
        close(p[0]);
        dfs(buff);
    }
    else
    {
        close(p[0]);
        modify(cur);
        write(p[1], cur, N + 1);
        close(p[1]);
        wait(0);
    }
    return;
}

int main()
{
    // 处理2~35的素数
    char flag[N + 1] = {};
    for (int i = 2; i <= N; i++)
        flag[i] = 1;

    dfs(flag);
    exit(0);
}
