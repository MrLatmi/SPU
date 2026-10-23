#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int stack_element_t;


#define DEBUG_FILE   "DEBUG.txt"

#define CANARY_COUNT 2

#define CANARY_LEFT         0xCAFEBABE
#define CANARY_RIGHT        0x8BADF00D
#define CANARY_STUCT_UP     0xC011
#define CANARY_STUCT_DOWN   0xDEC0DED

#define POISON_VALUE        0xDEADBEEF

#define CAPACITY_MAX 1000000

#ifdef STK_DEBUG
#define ON_DEBUG(...) __VA_ARGS__
#else
#define ON_DEBUG(...)
#endif


enum errors_t
{
    noProblem,
    callocError,
    capacityProblem,
    sizeProblem,
    alreadyInit,
    initProblem,
    nullProblem,
    canaryLeftDead,
    canaryRightDead,
    canaryUpDead,
    canaryDownDead,
    hashError,
};

#ifdef STK_DEBUG
struct debug_info_t
{
    const char* file;
    const char* func;
    int line;
    const char* fileName;
};
#endif


struct stack_t
{
    stack_element_t canaryUp;


#ifdef STK_DEBUG
    debug_info_t    debugInfo;
#endif
    stack_element_t *data;
    stack_element_t *canaryData;
    size_t           size;
    size_t           capacity;

    long long    hashData;
    long long    hashStruct;

    stack_element_t  canaryDown;
};


void            makeDebugLog        (const stack_t *stk, char *fileName   ON_DEBUG(, debug_info_t debugInfo, errors_t errCode));

errors_t        mainVerifier        (const stack_t *const stk             ON_DEBUG(, debug_info_t debugInfo));

void            stackInit           (stack_t *stk, size_t capacity        ON_DEBUG(, debug_info_t debugInfo));
void            stackDestroy        (stack_t *stk                         ON_DEBUG(, debug_info_t debugInfo));
void            stackPush           (stack_t *stk, stack_element_t value  ON_DEBUG(, debug_info_t debugInfo));
stack_element_t stackPop            (stack_t *stk                         ON_DEBUG(, debug_info_t debugInfo));
stack_element_t stackTop            (stack_t *const stk                   ON_DEBUG(, debug_info_t debugInfo));

void            printStackConsole   (const stack_t *stk                   ON_DEBUG(, debug_info_t debugInfo));

