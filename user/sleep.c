#include "kernel/types.h"
#include "user/user.h"

inline uint8 check_number(char *argv[])
{
    uint8 flag = 1;
    char *ptr = argv[1];
    while (*ptr)
    {
        if (!('0' <= *ptr && *ptr <= '9'))
        {
            flag = 0;
            break;
        }
        ptr++;
    }
    return flag;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        printf("no argument.\n");
    else if (argc > 2)
        printf("Excessive argument.\n");
    else
    {
        if (check_number(argv) & 1)
            sleep(atoi(argv[1]));
        else
            printf("error");
    }
    exit(0);
}