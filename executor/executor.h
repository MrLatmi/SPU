#ifndef _EXECUTOR_
#define _EXECUTOR_


#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })

#define REG_COUNT 4



#define SCREEN_WIDTH  32

#define SCREEN_HEIGHT 32

#define RAM_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)


struct processor
{
    int      reg[REG_COUNT];
    int      ram[RAM_SIZE];
    int      instructionPointer;
    stack_t* stk;
    stack_t* retStk;
};

struct byteCode
{
    int*   words;
    int    size;
};


void commandFromFileExecutor    (FILE* file, processor *proc);

bool loadByteCode               (FILE* file, byteCode *code);

bool anyJMPCommand              (processor *proc, const byteCode *code, const int command);

int  bufferChecker              ();

void addCommand                 (processor *proc);
void subCommand                 (processor *proc);
void divCommand                 (processor *proc);
void mulCommand                 (processor *proc);
void outCommand                 (processor *proc);
void inCommand                  (processor *proc);
void drawCommand                (processor *proc);
bool jaCommand                  (processor *proc);
bool jaeCommand                 (processor *proc);
bool jbCommand                  (processor *proc);
bool jbeCommand                 (processor *proc);
bool jeCommand                  (processor *proc);

bool callCommand                (processor *proc, const byteCode *code);
bool retCommand                 (processor *proc);

bool readArgument               (processor *proc, const byteCode *code, int *value, const char *name);
void pushHandler                (processor *proc, const byteCode *code, const int flags);
void popHandler                 (processor *proc, const byteCode *code, const int flags);

#endif
