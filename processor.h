#ifndef _MAIN_HEADER_
#define _MAIN_HEADER_


#undef OUT
#undef IN


#define OPCODE_MASK 0x1F


#define FLAG_NUM 1 << 5
#define FLAG_REG 1 << 6
#define FLAG_MEM 1 << 7


enum commands
{
    HLT,
    PUSH,
    ADD,
    SUB,
    DIV,
    OUT,
    JMP,
    JA,
    JAE,
    JB,
    JBE,
    JE,
    POP,
    CALL,
    RET,
    IN,
    MUL,
    DRAW,
};

enum Registers
{
    AX,
    BX,
    CX,
    DX,
};

#endif
