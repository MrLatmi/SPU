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
    PUSHr,
    POPr,
    CALL,
    RET,
    IN,
    MUL,
};

enum Registers
{
    AX,
    BX,
    CX,
    DX,
};
