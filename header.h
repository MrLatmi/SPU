#undef OUT
#undef IN

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
