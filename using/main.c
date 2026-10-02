#include <stdio.h>
#include <uv.h>

int main()
{
    printf("uv_version: %s\n", uv_version());

    return 0;
}