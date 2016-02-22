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
int notCorrectCount = 0;
int correctCount = 0;
int structIndex = 0;

//Struct
struct Entry {
    struct Entry *next;
    char *name;
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
            tableCount++;
        }
    }
}

//Renames functions to deal with reserved names in Assembly x86
char* funcRename(char * var) {
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
                    printf("    ld 3, %s@toc(2)\n", e -> varName);
                }
                else {
                    int offset = 8 * (inside + 1);
                    printf("    ld 3, %d(9)\n", offset);
                }
                break;
            }
            case eVAL : {
                printf("    stdu 4, -8(1)\n");
                printf("    convert64bit 4, %lu\n", e -> val);
                printf("    mr 3, 4\n");
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                break;
            }
            case ePLUS : {
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> left, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    mr 4, 3\n");
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> right, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    add 3, 3, 4\n");
                break;
            }
            case eMUL : {
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> left, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    mr 4, 3\n");
                printf("    stdu 4, -8(1)\n");
                myExpression(e -> right, p);
                printf("    ld 4, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    mulld 3, 3, 4\n");
                break;
            }
            case eEQ : {
                myExpression(e -> left, p);
                printf("    mr 4, 3\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                int notCorrectTemp = notCorrectCount;
                int correctTemp = correctCount;
                notCorrectCount++;
                correctCount++;
                printf("    bne notcorrect%d\n", notCorrectTemp);
                printf("    convert64bit 3, 1\n");
                printf("    b correct%d\n", correctTemp);
                printf("    notcorrect%d:\n", notCorrectTemp);
                printf("    convert64bit 3, 0\n");
                printf("    correct%d:\n", correctTemp);
                break;
            }
            case eNE : {
                myExpression(e -> left, p);
                printf("    mr 4, 3\n");
                myExpression(e -> right, p);
                printf("    cmpd 3, 4\n");
                int notCorrectTemp = notCorrectCount;
                int correctTemp = correctCount;
                notCorrectCount++;
                correctCount++;
                printf("    beq notcorrect%d\n", notCorrectTemp);
                printf("    convert64bit 3, 1\n");
                printf("    b correct%d\n", correctTemp);
                printf("    notcorrect%d:\n", notCorrectTemp);
                printf("    convert64bit 3, 0\n");
                printf("    correct%d:\n", correctTemp);
                break;
            } 
            case eLT : {
                myExpression(e -> left, p);
                printf("    mr 4, 3 \n");
                myExpression(e -> right, p);
                printf("    cmpd 4, 3\n");
                int notCorrectTemp = notCorrectCount;
                int correctTemp = correctCount;
                notCorrectCount++;
                correctCount++;
                printf("    bge notcorrect%d\n", notCorrectTemp);
                printf("    convert64bit 3, 1\n");
                printf("    b correct%d\n", correctTemp);
                printf("    notcorrect%d:\n", notCorrectTemp);
                printf("    convert64bit 3, 0\n");
                printf("    correct%d:\n", correctTemp);
                break;
            }
            case eGT : {
                myExpression(e -> left, p);
                printf("    mr 4, 3\n");
                myExpression(e -> right, p);
                printf("    cmpd 4, 3\n");
                int notCorrectTemp = notCorrectCount;
                int correctTemp = correctCount;
                notCorrectCount++;
                correctCount++;
                printf("    ble notcorrect%d\n", notCorrectTemp);
                printf("    convert64bit 3, 1\n");
                printf("    b correct%d\n", correctTemp);
                printf("    notcorrect%d:\n", notCorrectTemp);
                printf("    convert64bit 3, 0\n");
                printf("    correct%d:\n", correctTemp);
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
                        }
                    }
                }
                char *newName = funcRename(e -> callName);
                printf("    bl %s\n", newName);
                if (e != NULL) {
                    if (e -> callActuals != NULL) {
                        for (int i = 0; i < e -> callActuals -> n; i++) {
                            printf("    ld 5, 0(1)\n");
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
                    printf("    std 3, %s@toc(2)\n", s -> assignName);
                }
                else {
                    int offset = 8 * (inside + 1);
                    printf("    std 3, %d(9)\n", offset);
                }
                break;
            }
            case sPrint : {
                myExpression(s -> printValue, p);
                printf("    mr 15, 3\n");
                printf("    bl print\n");
                break;
            }
            case sIf : {
                myExpression(s -> ifCondition, p);
                int elseTemp = elseCount;
                int completeTemp = completeCount;
                elseCount++;
                completeCount++;
                printf("    cmpdi 3, 0\n");
                printf("%s%d\n", "    beq else", elseTemp);
                myStatement(s -> ifThen, p);
                printf("%s%d\n", "    b complete", completeTemp);
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
                printf("    cmpdi 3, 0\n");
                printf("%s%d\n", "    beq finished", finishedTemp);
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
                printf("    mr 1, 9\n");
                printf("    ld 9, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    ld 0, 0(1)\n");
                printf("    addi 1, 1, 8\n");
                printf("    mtlr 0\n");
                printf("    blr\n");
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
    printf("    mflr 0\n");
    printf("    stdu 0, -8(1)\n");
    //Move rbp to rsp
    printf("    stdu 9, -8(1)\n");
    printf("    mr 9, 1\n");
    myStatement(p -> body, p);
    //Move rsp back to rbp
    printf("    mr 1, 9\n");
    printf("    ld 9, 0(1)\n");
    printf("    addi 1, 1, 8\n");
    printf("    ld 0, 0(1)\n");
    printf("    addi 1, 1, 8\n");
    printf("    mtlr 0\n");
    printf("    blr\n");
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
    //Macro that takes care of numbers larger than 16 bits
    printf(".macro convert64bit register immediate\n");
    printf("    lis \\register,\\immediate@highest\n");
    printf("    ori \\register,\\register,\\immediate@higher\n");
    printf("    rldicr  \\register,\\register,32,31\n");
    printf("    oris    \\register,\\register,\\immediate@h\n");
    printf("    ori     \\register,\\register,\\immediate@l\n");
    printf(".endm\n");
    printf("    .global main\n");
    printf("main:\n");
    printf("    bl _main\n");
    printf("    b exit\n");
    genFuns(p);

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
    printf("    .global entry\n");
    printf("entry :\n");
    printf("    .quad main\n");
    printf("    .quad .TOC.@tocbase\n");
    printf("    .quad 0\n");

    return 0;
}
