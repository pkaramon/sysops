#include <stdio.h>

int main(int argc, char* args[])
{
    if(argc > 1) {
        printf("arg: %s\n", args[1]);
    }

    for (int i = 10; i >= 0; i--) {
        printf("%d\n", i);
    }
    return 0;
}
