#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

const int DEBUG = 0;

inline uint8 check_name(char *p, char *q)
{
    while (*p && *p == *q)
        p++, q++;
    return *p != 0;
}

char *fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}
void find(char *path, char *target)
{
    if (DEBUG)
        printf("current : %s %s\n", path, target);
    // 1.找当前路径下的文件
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        goto cleanup;
    }
    if (st.type != T_DIR)
    {
        // 递归到了非路径，终止
        if (!check_name(target, fmtname(path)))
            printf("%s\n", path);
    }
    else
    {
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            goto cleanup;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            char *name = fmtname(buf);
            if (strcmp(name, ".             ") && strcmp(name, "..            "))
            {
                if (DEBUG)
                    printf("%s %d %d %d\n", (buf), st.type, st.ino, st.size);
                find(buf, target);
            }
        }
    }
cleanup:
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        printf("Excessive argument.\n");
    else
        find(argv[1], argv[2]);
    exit(0);
}
