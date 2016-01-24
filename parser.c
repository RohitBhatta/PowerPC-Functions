#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "parser.h"

#define NEW(typ) ((typ*)malloc(sizeof(typ)))

/* Kinds of tokens */
enum TKind {
    tNONE,
    tINT,
    tID,
    tEND,
    tLEFT,
    tRIGHT,
    tPLUS,
    tMUL,
    tLBRACE,
    tRBRACE,
    tEQ,
    tIF,
    tELSE,
    tSEMI,
    tWHILE,
    tEQEQ,
    tLT,
    tGT,
    tNE,
    tPRINT,
    tFUN,
    tCOMMA,
    tRETURN
};

struct Token {
    enum TKind kind;
    int value;
    char *ptr;
};

typedef struct Token Token;

static int line = 1;
static int pos = 0;

static Token current = { tNONE, 0, NULL };

static jmp_buf escape;

static void peek();

static void error() {
    fprintf(stderr, "===> error at %d:%d <===\n", line, pos);
    longjmp(escape, 1);
}

static int nextChar = -2;       // -2 => empty, -1 => EOF, ...

static int peekChar(void) {
    if (nextChar == -2) {
        nextChar = getchar();
        pos++;
        if (nextChar == 10) {
            line++;
            pos = 0;
        }
    }
    return nextChar;
}

static void consumeChar(void) {
    nextChar = -2;
}

static void peekInt(void) {
    current.kind = tINT;

    while (1) {
        const int c = peekChar();
        switch (c) {
        case '0' ... '9':
            consumeChar();
            current.value = current.value * 10 + (c - '0');
            break;
        case '_':
            consumeChar();
            break;
        default:
            return;
        }
    }
}

static void peekId(void) {
    current.kind = tID;

    int len = 0;

    while (1) {
        const int c = peekChar();
        switch (c) {
        case 'a' ... 'z':
        case '0' ... '9':
            consumeChar();
            len++;
            current.ptr = realloc(current.ptr, len + 1);
            current.ptr[len - 1] = c;
            current.ptr[len] = 0;
            break;
        default:
            if (strcmp(current.ptr, "if") == 0) {
                current.kind = tIF;
            } else if (strcmp(current.ptr, "else") == 0) {
                current.kind = tELSE;
            } else if (strcmp(current.ptr, "while") == 0) {
                current.kind = tWHILE;
            } else if (strcmp(current.ptr, "print") == 0) {
                current.kind = tPRINT;
            } else if (strcmp(current.ptr, "fun") == 0) {
                current.kind = tFUN;
            } else if (strcmp(current.ptr, "return") == 0) {
                current.kind = tRETURN;
            } else {
                current.kind = tID;
            }
            if (current.kind != tID) {
                free(current.ptr);
                current.ptr = 0;
            }
            return;
        }
    }
}

static void peek() {
    if (current.kind == tNONE) {
        while (1) {
            current.value = 0;
            current.ptr = 0;

            const int c = peekChar();
            switch (c) {
            case -1:
                current.kind = tEND;
                return;
            case ';':
                consumeChar();
                current.kind = tSEMI;
                return;
            case '0' ... '9':
                peekInt();
                return;
            case 'a' ... 'z':
                peekId();
                return;
            case '(':
                consumeChar();
                current.kind = tLEFT;
                return;
            case ')':
                consumeChar();
                current.kind = tRIGHT;
                return;
            case '{':
                consumeChar();
                current.kind = tLBRACE;
                return;
            case '}':
                consumeChar();
                current.kind = tRBRACE;
                return;
            case '+':
                consumeChar();
                current.kind = tPLUS;
                return;
            case '*':
                consumeChar();
                current.kind = tMUL;
                return;
            case '=':
                consumeChar();
                if (peekChar() == '=') {
                    consumeChar();
                    current.kind = tEQEQ;
                } else {
                    current.kind = tEQ;
                }
                return;
            case '<':
                consumeChar();
                if (peekChar() == '>') {
                    consumeChar();
                    current.kind = tNE;
                } else {
                    current.kind = tLT;
                }
                return;
            case '>':
                consumeChar();
                current.kind = tGT;
                return;
            case ',':
                consumeChar();
                current.kind = tCOMMA;
                return;
            case ' ':
            case 10:
                consumeChar();
                break;
            default:
                printf("undefined char %d\n", c);
                error();
            }
        }
    }
}

static void consume() {
    peek();
    if (current.ptr != 0) {
        free(current.ptr);
        current.ptr = 0;
    }
    current.kind = tNONE;
}

static int isReturn() {
    peek();
    return current.kind == tRETURN;
}

static int isComma() {
    peek();
    return current.kind == tCOMMA;
}

static int isFun() {
    peek();
    return current.kind == tFUN;
}

static int isWhile() {
    peek();
    return current.kind == tWHILE;
}

static int isPrint() {
    peek();
    return current.kind == tPRINT;
}

static int isIf() {
    peek();
    return current.kind == tIF;
}

static int isElse() {
    peek();
    return current.kind == tELSE;
}

static int isSemi() {
    peek();
    return current.kind == tSEMI;
}

static int isLeftBlock() {
    peek();
    return current.kind == tLBRACE;
}

static int isRightBlock() {
    peek();
    return current.kind == tRBRACE;
}

static int isEq() {
    peek();
    return current.kind == tEQ;
}

static int isEqEq() {
    peek();
    return current.kind == tEQEQ;
}

static int isLt() {
    peek();
    return current.kind == tLT;
}

static int isGt() {
    peek();
    return current.kind == tGT;
}

static int isNe() {
    peek();
    return current.kind == tNE;
}

static int isLeft() {
    peek();
    return current.kind == tLEFT;
}

static int isRight() {
    peek();
    return current.kind == tRIGHT;
}

static int isEnd() {
    peek();
    return current.kind == tEND;
}

static int isId() {
    peek();
    return current.kind == tID;
}

static int isMul() {
    peek();
    return current.kind == tMUL;
}

static int isPlus() {
    peek();
    return current.kind == tPLUS;
}

static char *getId() {
    return strdup(current.ptr);
}

static int isInt() {
    peek();
    return current.kind == tINT;
}

static int getInt() {
    return current.value;
}

static Expression *expression(void);
static Block *block(void);

/* [ <expression> [, <actuals> ]] */
static Actuals *actuals(void) {
    if (isRight())
        return 0;
    Actuals *p = NEW(Actuals);
    p->first = expression();
    p->rest = 0;
    p->n = 1;

    if (isComma()) {
        consume();
        p->rest = actuals();
        p->n = p->rest->n + 1;
    }

    return p;
}

/* handle id, literals, and (...) */
static Expression *e1(void) {
    if (isLeft()) {
        /* ( <expression> ) */
        consume();
        Expression *e = expression();
        if (!isRight()) {
            error();
        }
        consume();
        return e;
    } else if (isInt()) {
        /* 123 */
        int v = getInt();
        consume();
        Expression *e = NEW(Expression);
        e->kind = eVAL;
        e->val = v;
        return e;
    } else if (isId()) {
        /* xyz */
        char *id = getId();
        consume();
        if (isLeft()) {
            /* xyz ( <actuals> ) */
            consume();

            Expression *e = NEW(Expression);

            e->kind = eCALL;

            e->callName = id;
            e->callActuals = actuals();

            if (!isRight())
                error();
            consume();

            return e;
        } else {
            Expression *e = NEW(Expression);
            e->kind = eVAR;
            e->varName = id;
            return e;
        }
    } else {
        error();
        return 0;
    }
}

/* <e1> [ * <e2> ] */
static Expression *e2(void) {
    Expression *left = e1();
    enum EKind kind;

    if (isMul()) {
        kind = eMUL;
    } else {
        return left;
    }

    consume();

    Expression *e = NEW(Expression);

    e->kind = kind;
    e->left = left;
    e->right = e2();

    return e;
}

/* <e2> [ + <e3> ] */
static Expression *e3(void) {
    Expression *left = e2();
    enum EKind kind;

    if (isPlus()) {
        kind = ePLUS;
    } else {
        return left;
    }

    consume();

    Expression *e = NEW(Expression);

    e->kind = kind;

    e->left = left;
    e->right = e3();

    return e;
}

/* <e3> ( != | == | < | > ) <e4> */
static Expression *e4(void) {
    Expression *left = e3();
    enum EKind kind;

    if (isEqEq()) {
        kind = eEQ;
    } else if (isLt()) {
        kind = eLT;
    } else if (isGt()) {
        kind = eGT;
    } else if (isNe()) {
        kind = eNE;
    } else {
        return left;
    }

    consume();

    Expression *e = NEW(Expression);

    e->kind = kind;
    e->left = left;
    e->right = e4();
    return e;
}

static Expression *expression(void) {
    return e4();
}

static Statement *statement(void) {

    if (isId()) {
        Statement *p = NEW(Statement);
        p->kind = sAssignment;
        p->assignName = getId();
        consume();
        if (!isEq())
            error();
        consume();
        p->assignValue = expression();

        if (isSemi()) {
            consume();
        }

        return p;
    } else if (isReturn()) {
        Statement *p = NEW(Statement);
        p->kind = sReturn;

        consume();
        p->returnValue = expression();

        if (isSemi()) {
            consume();
        }

        return p;
    } else if (isLeftBlock()) {
        Statement *p = NEW(Statement);
        p->kind = sBlock;

        consume();

        p->block = block();
        if (!isRightBlock())
            error();
        consume();
        return p;
    } else if (isPrint()) {
        Statement *p = NEW(Statement);
        p->kind = sPrint;

        consume();

        p->printValue = expression();

        if (isSemi()) {
            consume();
        }

        return p;
    } else if (isIf()) {
        Statement *p = NEW(Statement);
        p->kind = sIf;
        consume();

        p->ifCondition = expression();

        p->ifThen = statement();

        if (isElse()) {
            consume();
            p->ifElse = statement();
        } else {
            p->ifElse = 0;
        }
        return p;
    } else if (isWhile()) {
        Statement *p = NEW(Statement);
        p->kind = sWhile;

        consume();              /* while */

        p->whileCondition = expression();
        p->whileBody = statement();
        return p;
    } else if (isSemi()) {
        Statement *p = NEW(Statement);
        p->kind = sBlock;
        p->block = 0;
        consume();
        return p;
    } else {
        return 0;
    }
}

static Block *block(void) {
    Block *p = 0;

    Statement *first = statement();
    if (first) {
        p = NEW(Block);
        p->first = first;
        p->rest = block();
        if (p->rest) {
            p->n = p->rest->n + 1;
        } else {
            p->n = 1;
        }
    }

    return p;
}

/* [<id> [, <formals>]] */
static Formals *formals() {
    Formals *p = 0;

    if (isId()) {
        p = NEW(Formals);
        p->first = getId();
        consume();
        p->n = 1;
        p->rest = 0;

        if (isComma()) {
            consume();
            p->rest = formals();
            if (p->rest) {
                p->n = p->rest->n + 1;
            }
        }
    }

    return p;
}

/* fun <id> ( <formals> ) <body> */
static Fun *fun() {

    if (!isFun())
        return 0;
    consume();

    Fun *p = NEW(Fun);

    if (!isId())
        error();
    p->name = getId();
    consume();

    if (!isLeft())
        error();
    consume();

    p->formals = formals();

    if (!isRight())
        error();
    consume();

    p->body = statement();

    return p;
}

/* <fun> <fun> <fun> ... */
static Funs *funs() {
    Funs *p = 0;
    Fun *first = fun();

    if (first) {
        p = NEW(Funs);
        p->first = first;
        p->rest = funs();
        if (p->rest) {
            p->n = p->rest->n + 1;
        } else {
            p->n = 1;
        }
    }

    if (!isEnd())
        error();

    return p;
}

Funs *parse() {
    current.kind = tNONE;
    int x = setjmp(escape);
    if (x == 0) {
        return funs();
    } else {
        return 0;
    }
}
