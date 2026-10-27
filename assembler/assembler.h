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


int           getCommands      (const char *readFileName, FILE *fileRead, FILE *fileText, FILE *fileBinary);
static bool   commandCompound  (const size_t lineCount, output *out, char *const*index);

static void   createLabel      (int *pointerLabels);

static bool   firstEntrance    (const size_t lineCount, char *const*index, int *pointerLabels);
static bool   secondEntrance   (const size_t lineCount, output *out, char *const*index, int *pointerLabels);

static bool   writeWords       (output *out, const int *words, const size_t n);

static bool   parseOperand     (const char *args, operand *oper);
static int    operandWords     (const operand *oper);
static bool   writeOperand     (output *out, const int opCode, const operand *oper);

static bool   commandPush      (const char *command, const char *args, const size_t lineNum, output *out);
static bool   commandPop       (const char *command, const char *args, const size_t lineNum, output *out);
static bool   commandJump      (const int opCode, const char *args, const size_t lineNum, const int *pointerLabels, output *out);

static int    registerCode     (const char *name);
static bool   jumpOpCode       (const char *command, int *opCode);
static bool   simpleOpCode     (const char *command, int *opCode);

#endif
