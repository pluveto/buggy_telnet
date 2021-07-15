#include <stdio.h>
#include <string.h>
int main(int argc, char const *argv[])
{
    printf("hello");
    char s[100];
    for (;;)
    {
        scanf("%s", s);
        printf("%s\n", s);

        if (strcmp(s, "q") == 0)
        {
            break;
        }
    }
    return 0;
}
