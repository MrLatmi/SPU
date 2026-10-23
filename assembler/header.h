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


int    getCommands      (char *readFileName, FILE *fileRead, FILE *fileText, FILE *fileBinary);
bool   commandCompound  (size_t lineCount, output *out, char **index);

void   createLabel      (int *pointerLabels);

bool   firstEntrance    (size_t lineCount, char **index, int *pointerLabels);
bool   secondEntrance   (size_t lineCount, output *out, char **index, int *pointerLabels);

bool   writeWords       (output *out, const int *words, int n);

bool   parseOperand     (const char *args, operand *a);
int    operandWords     (const operand *a);
bool   writeOperand     (output *out, int opCode, const operand *a);

bool   commandPush      (const char *command, const char *args, size_t lineNum, output *out);
bool   commandPop       (const char *command, const char *args, size_t lineNum, output *out);
bool   commandJump      (int opCode, const char *args, size_t lineNum, const int *pointerLabels, output *out);

int    registerCode     (const char *name);
bool   jumpOpCode       (const char *command, int *opCode);
bool   simpleOpCode     (const char *command, int *opCode);

#endif
