#ifndef _EXECUTOR_
#define _EXECUTOR_

#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })

#define REG_COUNT 4



#define SCREEN_WIDTH  32



#define SCREEN_HEIGHT 32

#define RAM_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define ERROR_PRINTF(format, ...) printf("\033[1;31m" format " %s, %d \033[0m\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define GREEN_PRINTF(format, ...) printf("\033[1;32m" format "\033[0m\n" __VA_OPT__(,) __VA_ARGS__)

struct processor
{
    int reg[REG_COUNT];
    int ram[RAM_SIZE];
    int instructionPointer;
};

struct byteCode
{
    int    *words;
    size_t  size;
};


void commandFromFileExecutor    (stack_t *const stk, stack_t *const retStk, FILE* file, processor *proc);

bool loadByteCode               (FILE* file, byteCode *code);

bool anyJMPCommand              (stack_t *stk, processor *proc, byteCode *code, int command);

int  bufferChecker              ();

void addCommand                 (stack_t *stk);
void subCommand                 (stack_t *stk);
void divCommand                 (stack_t *stk);
void mulCommand                 (stack_t *stk);
void outCommand                 (stack_t *stk);
void inCommand                  (stack_t *stk);
void drawCommand                (const processor *proc);
bool jaCommand                  (stack_t *stk);
bool jaeCommand                 (stack_t *stk);
bool jbCommand                  (stack_t *stk);
bool jbeCommand                 (stack_t *stk);
bool jeCommand                  (stack_t *stk);

bool callCommand                (stack_t *retStk, processor *proc, byteCode *code);
bool retCommand                 (stack_t *retStk, processor *proc);

bool readArgument               (processor *proc, byteCode *code, int *value, const char *name);
void pushHandler                (stack_t *stk, processor *proc, byteCode *code, int flags);
void popHandler                 (stack_t *stk, processor *proc, byteCode *code, int flags);

#endif
