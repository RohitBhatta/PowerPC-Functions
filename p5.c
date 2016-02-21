#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int howManyBytes = 32;

//Global variables
int tableCount = 0;
int first = 0;
int elseCount = 0;
int completeCount = 0;
int againCount = 0;
int finishedCount = 0;
int structIndex = 0;

//Struct
struct Entry {
    struct Entry *next;
    char *name;
    //int index;
};

//Global struct
struct Entry *table;

//Adds variable to linked list
void set(char *id) {
    int same = 0;
    struct Entry *current = (struct Entry *) malloc(sizeof(struct Entry));
    if (first == 0) {
        table -> name = id;
        table -> next = NULL;
        //table -> index = structIndex++;
        first++;
        tableCount++;
    }
    else {
        current = table;
        while (current -> next != NULL) {
            if (strcmp(current -> name, id) == 0) {
                same = 1;
            }
            current = current -> next;
        }
        if (strcmp(current -> name, id) == 0) {
            same = 1;
        }
    }
    if (!same) {
        current -> next = malloc(sizeof(struct Entry));
        //No memory left
        if (current -> next == NULL) {
            printf("Out of memory\n");
        }
        //Assign and print values
        else {
            current = current -> next;
            current -> name = id;
            current -> next = NULL;
            //current -> index = structIndex++;
            tableCount++;
        }
    }
}

//Renames functions to deal with reserved names in Assembly x86
char* funcRename(char * var) {
    /*if (strcmp(var, "main") == 0) {
        return var;
    }*/
    size_t size = strlen(var) + 1;
    char *fullName = malloc(size);
    strcpy(fullName, "_");
    strcat(fullName, var);
    return fullName;
}    

//Returns the index of the variable on the stack if it's local, or -1 if it's global.
int formal(Fun * p, char * s) {
    if (p != NULL) {
        if (p -> formals != NULL) {
            Formals *myFormal = p -> formals;
            for (int i = 0; i < p -> formals -> n; i++) {
                if (strcmp(s, myFormal -> first) == 0) {
                    return (p -> formals -> n) - (myFormal -> n) + 1;
                }
                myFormal = myFormal -> rest;
            }
        }
    }
    return -1;
}

//Handles expression enums
void myExpression (Expression * e, Fun * p) {
    if (e != NULL) {
        switch (e -> kind) {
            case eVAR : {
                int inside = formal(p, e -> varName);
                if (inside == -1) {
                    set(e -> varName);
                    printf("    ld 3, %s@toc(2)\n", table -> name);
                    //printf("    mov %s, %%r15\n", e -> varName);
                }
                else {
                    int offset = 8 * (inside);
                    printf("    ld 3, %d(1)\n", offset);
                    //printf("    mov %d(%%rbp), %%r15\n", 8 * (inside + 1));
                }
                break;
            }
            case eVAL : {
                //Copy from register to memory
                printf("    stdu 4, -8(1)\n");
                //Check printf syntax
                printf("    li 4, %lu\n", e -> val);
                printf("    or 3, 4, 4\n");
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                /*printf("    push %%r13\n");
                printf("    mov $");
                printf("%lu", e -> val);
                printf(", %%r13\n");
                printf("    mov %%r13, %%r15\n");
                printf("    pop %%r13\n");*/
                break;
            }
            case ePLUS : {
                //Copy from register to memory
                printf("    stdu 4, -8(1)\n");
                //printf("    push %%r13\n");
                myExpression(e -> left, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    or 4, 3, 3\n");
                /*printf("    pop %%r13\n");
                printf("    mov %%r15, %%r13\n");
                printf("    push %%r13\n");*/
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> right, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    add 3, 3, 4\n");
                /*printf("    pop %%r13\n");
                printf("    add %%r13, %%r15\n");*/
                break;
            }
            case eMUL : {
                //Copy from register to memory
                printf("    stdu 4, -8(1)\n");
                //printf("    push %%r13\n");
                myExpression(e -> left, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    or 4, 3, 3\n");
                /*printf("    pop %%r13\n");
                printf("    mov %%r15, %%r13\n");
                printf("    push %%r13\n");*/
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> right, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    mulld 3, 3, 4\n");
                /*printf("    pop %%r13\n");
                printf("    imul %%r13, %%r15\n");*/
                break;
            }
            case eEQ : {
                myExpression(e -> left, p);
                printf("    or 4, 3, 3\n");
                //printf("    mov %%r15, %%r13\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                //printf("    beq correct");
                printf("    bne notcorrect\n");
                printf("    blt notcorrect\n");
                printf("    bgt notcorrect\n");
                /*printf("    cmp %%r13, %%r15\n");
                printf("    setz %%r15b\n");
                printf("    movzx %%r15b, %%r15\n");*/ 
                break;
            }
            case eNE : {
                myExpression(e -> left, p);
                printf("    or 4, 3, 3\n");
                //printf("    mov %%r15, %%r13\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                //printf("    bne correct");
                printf("    beq notcorrect\n");
                printf("    blt notcorrect\n");
                printf("    bgt notcorrect\n");
                /*printf("    cmp %%r13, %%r15\n");
                printf("    setnz %%r15b\n");
                printf("    movzx %%r15b, %%r15\n");*/ 
                break;
            } 
            case eLT : {
                myExpression(e -> left, p);
                printf("    or 4, 3, 3\n");
                //printf("    mov %%r15, %%r13\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                //printf("    blt correct");
                printf("    beq notcorrect\n");
                printf("    bne notcorrect\n");
                printf("    bgt notcorrect\n");
                /*printf("    cmp %%r15, %%r13\n");
                printf("    setl %%r15b\n");
                printf("    movzx %%r15b, %%r15\n");*/
                break;
            }
            case eGT : {
                myExpression(e -> left, p);
                printf("    or 4, 3, 3\n");
                //printf("    mov %%r15, %%r13\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                //printf("    bgt correct");
                printf("    beq notcorrect\n");
                printf("    bne notcorrect\n");
                printf("    blt notcorrect\n");
                /*printf("    cmp %%r15, %%r13\n");
                printf("    setg %%r15b\n");
                printf("    movzx %%r15b, %%r15\n");*/
                break;
            }
            case eCALL : {
                if (e != NULL) {
                    if (e -> callActuals != NULL) {
                        for (int i = e -> callActuals -> n - 1; i >= 0; i--) {
                            Actuals *actual = e -> callActuals; 
                            for (int a = 0; a < i; a++) {
                                actual = actual -> rest;
                            }
                            myExpression(actual -> first, p);
                            printf("    stdu 3, -8(1)\n");
                            //printf("    push %%r15\n");
                        }
                    }
                }
                char *newName = funcRename(e -> callName);
                printf("    bl %s\n", newName);
                //printf("    call ");
                //printf("%s\n", newName);
                if (e != NULL) {
                    if (e -> callActuals != NULL) {
                        for (int i = 0; i < e -> callActuals -> n; i++) {
                            //printf("    pop %%r14\n");
                            printf("    ld 4, 0(1)\n");
                            printf("    addi 1, 1, 8\n");
                        }
                    }
                }
                break;
            }
            default : {
                break;
            }
        }
    }
}

//Handles statement enums
void myStatement(Statement * s, Fun * p) {
    if (s != NULL) {
        switch (s -> kind) {
            case sAssignment : {
                myExpression(s -> assignValue, p);
                int inside = formal(p, s -> assignName);
                if (inside == -1) {
                    set(s -> assignName);
                    //int offset = 8 * (table -> index);
                    printf("    std 3, %s@toc(2)\n", table -> name);
                    //printf("    std 3, %d@toc(2)\n", 8 * (table -> index));
                    /*printf("    mov %%r15, ");
                    printf("%s\n", s -> assignName);*/
                }
                else {
                    int offset = 8 * (inside);
                    printf("    std 3, %d(1)\n", offset);
                    //printf("    std 3, %d@toc(2)\n", 8 * (inside + 1));
                    //printf("    mov %%r15, %d(%%rbp)\n", 8 * (inside + 1));
                }
                break;
            }
            case sPrint : {
                myExpression(s -> printValue, p);
                printf("    or 15, 3, 3\n");
                printf("    bl print\n");
                /*printf("    mov $p4_format, %%rdi\n");
                printf("    mov %%r15, %%rsi\n");
                printf("    mov $0, %%rax\n");
                printf("    call printf\n");*/
                break;
            }
            case sIf : {
                myExpression(s -> ifCondition, p);
                int elseTemp = elseCount;
                int completeTemp = completeCount;
                elseCount++;
                completeCount++;
                /*printf("    cmp $0, %%r15\n");
                printf("%s%d\n", "    je else", elseTemp);*/
                myStatement(s -> ifThen, p);
                printf("%s%d\n", "    b complete", completeTemp);
                printf("    notcorrect:\n");
                printf("%s%d%s\n", "    else", elseTemp, ":");
                myStatement(s -> ifElse, p);
                printf("%s%d%s\n", "    complete", completeTemp, ":");
                break;
            } 
            case sWhile : {
                int againTemp = againCount;
                int finishedTemp = finishedCount;
                againCount++;
                finishedCount++;
                printf("%s%d%s\n", "    again", againTemp, ":");
                myExpression(s -> whileCondition, p);
                //printf("    cmp $0, %%r15\n");
                printf("    notcorrect:\n");
                printf("%s%d\n", "    b finished", finishedTemp);
                myStatement(s -> whileBody, p);
                printf("%s%d\n", "    b again", againTemp);
                printf("%s%d%s\n", "    finished", finishedTemp, ":");
                break;
            }
            case sBlock : {
                Block *current = s -> block;
                while (current != NULL) {
                    myStatement(current -> first, p);
                    current = current -> rest;
                }
                break;
            } 
            case sReturn : {
                myExpression(s -> returnValue, p);
                printf("    ld 0, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    ld 0, 8(1)\n");
                printf("    mtlr 0\n");
                printf("    blr\n");
                /*printf("    mov %%rbp, %%rsp\n");
                printf("    pop %%rbp\n");
                printf("    mov $0,%%rax\n");
                printf("    ret\n");*/
                break;
            } 
            default : {
                break;
            }
        }
    }
}

void genFun(Fun * p) {
    char *rename = funcRename(p -> name);
    printf("    .global %s\n", rename);
    printf("%s:\n", rename);
    //Set up stack frame
    printf("    mflr 0\n");
    printf("    stdu 0, -8(1)\n");
    //printf("    stw 0, 8(1)\n");
    //printf("    stwu 1, -16(1)\n");
    /*printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");*/
    myStatement(p -> body, p);
    printf("    ld 0, 0(1)\n");
    printf("    addi 1, 1, 8\n");
    /*if (strcmp(p -> name, "main") == 0) {
        printf("    b exit\n");
    }*/
    printf("    ld 0, 8(1)\n");
    printf("    mtlr 0\n");
    printf("    blr\n");
    /*if (strcmp(p -> name, "main") == 0) {
        printf("    b exit\n");
    }*/
    /*printf("    mov %%rbp, %%rsp\n");
    printf("    pop %%rbp\n");
    printf("    mov $0,%%rax\n");
    printf("    ret\n");*/
}

void genFuns(Funs * p) {
    if (p == 0)
        return;
    genFun(p->first);
    genFuns(p->rest);
}

int main(int argc, char *argv[]) {
    Funs *p = parse();
    table = (struct Entry *) malloc(sizeof(struct Entry));
    //printf("    addi 1,2,-8\n");       /* SP */
    printf("    .global main\n");
    printf("main:\n");
    printf("    bl _main\n");
    printf("    b exit\n");
    genFuns(p);

    //This part is weird
    //Copy p4 code because this prints 42
    /*printf("    .global main\n");
    printf("main:\n");
    printf("    addi 1,2,-8\n");*/        /* SP */
    /*printf("    li 15,42\n");
    printf("    bl print\n");
    printf("    b exit\n");*/

    //Moved data section in front because of segaults mentioned on piazza
    /*printf("    .data\n");
    printf("data : .fill %d\n", howManyBytes);
    
    printf("     .fill 8000\n");*/
    //printf("stackBottom:\n");

    printf("    .section \".toc\", \"aw\"\n");
    if (tableCount != 0) {
        while (table -> next != NULL) {
            printf("    ");
            printf("%s", table -> name);
            printf(": .quad 0\n");
            table = table -> next;
        }
        printf("    ");
        printf("%s", table -> name);
        printf(": .quad 0\n");
        table = table -> next;
    }
    /*printf("    .data\n");
    printf("data : .fill %d\n", howManyBytes);*/

    //printf("    b exit\n");
    //printf(".section \".opd\", \"aw\"\n");
    printf("    .global entry\n");
    printf("entry :\n");
    printf("    .quad main\n");
    printf("    .quad .TOC.@tocbase\n");
    //printf("    .quad data\n");
    printf("    .quad 0\n");

    return 0;
}
