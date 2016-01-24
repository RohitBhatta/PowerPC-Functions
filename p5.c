#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int howManyBytes = 32;

int main(int argc, char *argv[]) {
    /*Funs *p = */ parse();

    printf("    .global main\n");
    printf("main:\n");
    printf("    addi 1,2,-8\n");        /* SP */
    printf("    li 15,42\n");
    printf("    bl print\n");
    printf("    b exit\n");

    printf("     .fill 8000\n");
    printf("stackBottom:\n");

    printf("    .data\n");
    printf("data : .fill %d\n", howManyBytes);

    printf("    .global entry\n");
    printf("entry :\n");
    printf("    .quad main\n");
    printf("    .quad data\n");
    printf("    .quad 0\n");

    return 0;
}
