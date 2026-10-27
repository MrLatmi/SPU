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

static bool loadByteCode               (FILE* file, byteCode *code);
static bool anyJMPCommand              (processor *proc, const byteCode *code, const int command);

static int  bufferChecker              ();

static void addCommand                 (processor *proc);
static void subCommand                 (processor *proc);
static void divCommand                 (processor *proc);
static void mulCommand                 (processor *proc);
static void outCommand                 (processor *proc);
static void inCommand                  (processor *proc);
static void drawCommand                (processor *proc);
static bool jaCommand                  (processor *proc);
static bool jaeCommand                 (processor *proc);
static bool jbCommand                  (processor *proc);
static bool jbeCommand                 (processor *proc);
static bool jeCommand                  (processor *proc);

static bool callCommand                (processor *proc, const byteCode *code);
static bool retCommand                 (processor *proc);

static bool readArgument               (processor *proc, const byteCode *code, int *value, const char *name);
static void pushHandler                (processor *proc, const byteCode *code, const int flags);
static void popHandler                 (processor *proc, const byteCode *code, const int flags);

#endif
