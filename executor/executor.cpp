#include <TXlib.h>
#include "stackWithCanary.cpp"
#include "fileReader.cpp"
#include "..\header.h"
#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })


#define ERROR_PRINTF(format, ...) printf("\033[1;31m" format " %s, %d \033[0m\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define GREEN_PRINTF(format, ...) printf("\033[1;32m" format "\033[0m\n" __VA_OPT__(,) __VA_ARGS__)

struct processor
{
    int reg[4];
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
bool jaCommand                  (stack_t *stk);
bool jaeCommand                 (stack_t *stk);
bool jbCommand                  (stack_t *stk);
bool jbeCommand                 (stack_t *stk);
bool jeCommand                  (stack_t *stk);

bool callCommand                (stack_t *retStk, processor *proc, byteCode *code);
bool retCommand                 (stack_t *retStk, processor *proc);

bool readArgument               (processor *proc, byteCode *code, int *value, const char *name);
void pushHandler                (stack_t *stk, processor *proc, byteCode *code);
void pushRHandler               (stack_t *stk, processor *proc, byteCode *code);
void popRHandler                (stack_t *stk, processor *proc, byteCode *code);

void pushRCommand               (stack_t *stk, int value, processor *proc);
void popRCommand                (stack_t *const stk, int reg, processor *proc);

void commandFromFileExecutor(stack_t *const stk, stack_t *const retStk, FILE* file, processor *proc)
{
    byteCode code = {};
    if (!loadByteCode(file, &code))
        return;

    while((size_t)proc->instructionPointer < code.size && proc->instructionPointer >= 0)
    {
        int command = code.words[proc->instructionPointer];

        bool jumped = false;

        switch (command)
        {
            case JMP: case JA: case JAE: case JB: case JBE: case JE:
                jumped = anyJMPCommand(stk, proc, &code, command);
                break;
            case CALL:
                jumped = callCommand(retStk, proc, &code);
                break;
            case RET:
                jumped = retCommand(retStk, proc);
                break;
            case PUSH:
                pushHandler(stk, proc, &code);
                break;
            case PUSHr:
                pushRHandler(stk, proc, &code);
                break;
            case POPr:
                popRHandler(stk, proc, &code);
                break;
            case ADD:
                addCommand(stk);
                printStackConsole(stk);
                break;
            case SUB:
                subCommand(stk);
                printStackConsole(stk);
                break;
            case DIV:
                divCommand(stk);
                printStackConsole(stk);
                break;
            case MUL:
                mulCommand(stk);
                printStackConsole(stk);
                break;
            case OUT:
                outCommand(stk);
                break;
            case IN:
                inCommand(stk);
                printStackConsole(stk);
                break;
            case HLT:
                printStackConsole(stk);
                GREEN_PRINTF("Program finished\n");

                free(code.words);
                return;

            default:
                ERROR_PRINTF("Unknown command %d at code %d ", command, proc->instructionPointer);
                break;
        }


        if (!jumped)
            proc->instructionPointer++;
    }

    free(code.words);
}


bool loadByteCode(FILE* file, byteCode *code)
{
    size_t capacity = 64;
    size_t size     = 0;
    int   *words    = (int*) calloc(capacity, sizeof(int));
    if (words == nullptr)
        return false;

    int value = 0;
    while (fread(&value, sizeof(int), 1, file) == 1)
    {
        if (size == capacity)
        {
            capacity *= 2;
            int *tmp = (int*) realloc(words, capacity * sizeof(int));
            if (tmp == nullptr)
            {
                free(words);
                return false;
            }
            words = tmp;
        }
        words[size++] = value;
    }

    code->words = words;
    code->size  = size;

    return true;
}




int bufferChecker()
{
    int symbol           = 0;
    bool hasAnotherArgue = false;
    while((symbol = getchar()) != '\n' && symbol != EOF)
    {
        if(symbol != ' ')
            hasAnotherArgue = true;
    }
    return hasAnotherArgue;
}

void addCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (stk);
    int value2 = stackPop (stk);
    int sum    = value1 + value2;
    stackPush(stk, sum);
}

void pushRCommand(stack_t *const stk, int value, processor *proc)
{
    switch(value)
    {
        case AX:
            stackPush(stk, proc->reg[AX]);
            break;
        case BX:
            stackPush(stk, proc->reg[BX]);
            break;
        case CX:
            stackPush(stk, proc->reg[CX]);
            break;
        case DX:
            stackPush(stk, proc->reg[DX]);
            break;
        default:
            ERROR_PRINTF("Wrong argument\n");
            break;
    }
}




bool anyJMPCommand(stack_t *const stk, processor *proc, byteCode *code, int command)
{
    int nextIp = proc->instructionPointer + 1;
    if ((size_t)nextIp >= code->size)
    {
        ERROR_PRINTF("Missing argument for jump at code %d", proc->instructionPointer);
        return false;
    }

    int targetAddress = code->words[nextIp];

    bool condition = false;
    switch (command)
    {
        case JMP: condition = true;          break;
        case JA:  condition = jaCommand(stk);  break;
        case JAE: condition = jaeCommand(stk); break;
        case JB:  condition = jbCommand(stk);  break;
        case JBE: condition = jbeCommand(stk); break;
        case JE:  condition = jeCommand(stk);  break;
        default:  break;
    }

    if (condition)
    {
        proc->instructionPointer = targetAddress;
        return true;
    }

    proc->instructionPointer++;
    return false;
}

bool callCommand(stack_t *const retStk, processor *proc, byteCode *code)
{
    int targetAddress = 0;
    if (!readArgument(proc, code, &targetAddress, "CALL"))
        return false;

    if (targetAddress < 0 || (size_t)targetAddress >= code->size)
    {
        ERROR_PRINTF("Bad call target %d", targetAddress);
        return false;
    }

    int returnAddress = proc->instructionPointer + 1;
    stackPush(retStk, returnAddress);

    proc->instructionPointer = targetAddress;
    return true;
}

bool retCommand(stack_t *const retStk, processor *proc)
{
    if (retStk->size < 1)
    {
        ERROR_PRINTF("RET without CALL at code %d", proc->instructionPointer);
        return false;
    }

    proc->instructionPointer = stackPop(retStk);
    return true;
}

void popRCommand(stack_t *const stk, int reg, processor *proc)
{
    if (stk->size < 1)
    {
        ERROR_PRINTF("You haven't enough values");
        return;
    }

    int value = stackPop(stk);
    switch (reg)
    {
        case AX: proc->reg[AX] = value; break;
        case BX: proc->reg[BX] = value; break;
        case CX: proc->reg[CX] = value; break;
        case DX: proc->reg[DX] = value; break;
        default: ERROR_PRINTF("Wrong register %d", reg); break;
    }
}


void subCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (stk);
    int value2 = stackPop (stk);
    int sub    = value2 - value1;
    stackPush(stk, sub);
}

void divCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    if (stackTop(stk) == 0)
    {
        ERROR_PRINTF("Zero division");
        return;
    }
    int value1 = stackPop(stk);
    int value2 = stackPop(stk);
    int div = value2 / value1;
    stackPush(stk, div);
}

void mulCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    if (stackTop(stk) == 0)
    {
        ERROR_PRINTF("Zero division");
        return;
    }
    int value1 = stackPop(stk);
    int value2 = stackPop(stk);
    int mul = value2 * value1;
    stackPush(stk, mul);
}

void outCommand(stack_t *const stk)
{
    if (stk->size < 1)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    GREEN_PRINTF("Last operation value = <%d>\n", stackTop(stk));
}

void inCommand(stack_t *const stk)
{
    int value = 0;

    printf("Enter value: ");
    if (scanf("%d", &value) != 1)
    {
        ERROR_PRINTF("Wrong IN value\n");
        bufferChecker();
        return;
    }

    if (bufferChecker())
    {
        ERROR_PRINTF("Too many arguments\n");
        return;
    }

    stackPush(stk, value);
}

bool jaCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    return value1 > value2;
}

bool jaeCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    return value1 >= value2;
}

bool jbCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    return value1 < value2;
}

bool jbeCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    return value1 <= value2;
}

bool jeCommand(stack_t *const stk)
{
    if (stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    return value1 == value2;
}


bool readArgument(processor *proc, byteCode *code, int *value, const char *name)
{
    proc->instructionPointer++;
    if ((size_t)proc->instructionPointer >= code->size)
    {
        ERROR_PRINTF("Missing argument for %s", name);
        return false;
    }

    *value = code->words[proc->instructionPointer];
    return true;
}

void pushHandler(stack_t *stk, processor *proc, byteCode *code)
{
    int value = 0;
    if (!readArgument(proc, code, &value, "PUSH"))
        return;

    stackPush(stk, value);
    printStackConsole(stk);
}

void pushRHandler(stack_t *stk, processor *proc, byteCode *code)
{
    int reg = 0;
    if (!readArgument(proc, code, &reg, "PUSHr"))
        return;

    pushRCommand(stk, reg, proc);
    printStackConsole(stk);
}

void popRHandler(stack_t *stk, processor *proc, byteCode *code)
{
    int reg = 0;
    if (!readArgument(proc, code, &reg, "POPr"))
        return;

    popRCommand(stk, reg, proc);
    printStackConsole(stk);
}
