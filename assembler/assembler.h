#ifndef _ASSEMBLER_
#define _ASSEMBLER_



struct output
{
    FILE *text;
    FILE *binary;
};

struct operand
{
    bool mem;
    bool hasReg;
    bool hasNum;
    int  reg;
    int  value;
};


int    getCommands      (const char *readFileName, FILE *fileRead, FILE *fileText, FILE *fileBinary);
bool   commandCompound  (const size_t lineCount, output *out, char **index);

void   createLabel      (int *pointerLabels);

bool   firstEntrance    (const size_t lineCount, char *const*index, int *pointerLabels);
bool   secondEntrance   (const size_t lineCount, output *out, char *const*index, int *pointerLabels);

bool   writeWords       (output *out, const int *words, size_t n);

bool   parseOperand     (const char *args, operand *oper);
int    operandWords     (const operand *oper);
bool   writeOperand     (output *out, const int opCode, const operand *oper);

bool   commandPush      (const char *command, const char *args, const size_t lineNum, output *out);
bool   commandPop       (const char *command, const char *args, const size_t lineNum, output *out);
bool   commandJump      (const int opCode, const char *args, const size_t lineNum, const int *pointerLabels, output *out);

int    registerCode     (const char *name);
bool   jumpOpCode       (const char *command, int *opCode);
bool   simpleOpCode     (const char *command, int *opCode);

#endif
