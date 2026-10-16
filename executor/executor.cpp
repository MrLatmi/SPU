#include <TXlib.h>
#include "stackWithCanary.cpp"
#include "fileReader.cpp"
#include "..\header.h"
#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })


#define ERROR_PRINTF(format, ...) printf("\033[1;31m" format "%s, %d \033[0m\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define GREEN_PRINTF(format, ...) printf("\033[1;32m" format "\033[0m\n" __VA_OPT__(,) __VA_ARGS__)

struct processor
{
    int reg[4];
    int instructionPointer;
};

void commandFromFileExecutor    (stack_t *const stk, FILE* file, const char* fileName, processor *proc);
void commandFromConsoleExecutor (stack_t *stk);

bool anyJMPCommand              (stack_t *stk, processor *proc, textInfo *text, char **code, int command);

int  bufferChecker              ();

void addCommand                 (stack_t *stk);
void subCommand                 (stack_t *stk);
void divCommand                 (stack_t *stk);
void outCommand                 (stack_t *stk);
bool jaCommand                  (stack_t *stk);
bool jaeCommand                 (stack_t *stk);
bool jbCommand                  (stack_t *stk);
bool jbeCommand                 (stack_t *stk);
bool jeCommand                  (stack_t *stk);

bool readArgument               (processor *proc, textInfo *text, char **code, int *value, const char *name);
void pushHandler                (stack_t *stk, processor *proc, textInfo *text, char **code);
void pushRHandler               (stack_t *stk, processor *proc, textInfo *text, char **code);
void popRHandler                (stack_t *stk, processor *proc, textInfo *text, char **code);

void pushRCommand               (stack_t *stk, int value, processor *proc);
void popRCommand                (stack_t *const stk, int reg, processor *proc);

void commandFromFileExecutor(stack_t *const stk, FILE* file, const char* fileName, processor *proc)
{
    textInfo text = {};
    char **code = getIndex(file, fileName, &text, wordCompare);
    if (code == NULL)
    {
        ERROR_PRINTF("Failed to read token code from <%s>", fileName);
        return;
    }

    while((size_t)proc->instructionPointer < text.linesCount && proc->instructionPointer >= 0)
    {
        int command = 0;
        if (sscanf(code[proc->instructionPointer], "%d", &command) != 1)
        {
            ERROR_PRINTF("Failed to parse command code at code %d", proc->instructionPointer);
            break;
        }

        bool jumped = false;

        switch (command)
        {
            case JMP: case JA: case JAE: case JB: case JBE: case JE:
                jumped = anyJMPCommand(stk, proc, &text, code, command);
                break;
                break;
            case PUSH:
                pushHandler(stk, proc, &text, code);
                break;
            case PUSHr:
                pushRHandler(stk, proc, &text, code);
                break;
            case POPr:
                popRHandler(stk, proc, &text, code);
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
            case OUT:
                outCommand(stk);
                break;
            case HLT:
                printStackConsole(stk);
                GREEN_PRINTF("Program finished\n");

                free(code);
                free(text.buffer);
                return;

            default:
                ERROR_PRINTF("Unknown command %d at code %d", command, proc->instructionPointer);
                break;
        }


        if (!jumped)
            proc->instructionPointer++;
    }

    free(code);
    free(text.buffer);
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




bool anyJMPCommand(stack_t *const stk, processor *proc, textInfo *text, char **code, int command)
{
    int nextIp = proc->instructionPointer + 1;
    if ((size_t)nextIp >= text->linesCount)
    {
        ERROR_PRINTF("Missing argument for jump at code %d", proc->instructionPointer);
        return false;
    }

    int targetAddress = 0;
    if (sscanf(code[nextIp], "%d", &targetAddress) != 1)
    {
        ERROR_PRINTF("Bad jump target at code %d", nextIp);
        return false;
    }

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


//Функцяи которая воззвращает функцию. Если команда jjmp, тогда п
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

void outCommand(stack_t *const stk)
{
    if (stk->size < 1)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    GREEN_PRINTF("Last operation value = <%d>\n", stackTop(stk));
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

    if(value1 > value2)
        return true;

    return false;

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

    if(value1 >= value2)
        return true;

    return false;

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

    if(value1 < value2)
        return true;

    return false;

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

    if(value1 <= value2)
        return true;

    return false;

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

    if(value1 == value2)
        return true;

    return false;

}


bool readArgument(processor *proc, textInfo *text, char **code, int *value, const char *name)
{
    proc->instructionPointer++;
    if ((size_t)proc->instructionPointer >= text->linesCount ||
        sscanf(code[proc->instructionPointer], "%d", value) != 1)
    {
        ERROR_PRINTF("Missing argument for %s", name);
        return false;
    }
    return true;
}

void pushHandler(stack_t *stk, processor *proc, textInfo *text, char **code)
{
    int value = 0;
    if (!readArgument(proc, text, code, &value, "PUSH"))
        return;

    stackPush(stk, value);
    printStackConsole(stk);
}

void pushRHandler(stack_t *stk, processor *proc, textInfo *text, char **code)
{
    int reg = 0;
    if (!readArgument(proc, text, code, &reg, "PUSHr"))
        return;

    pushRCommand(stk, reg, proc);
    printStackConsole(stk);
}

void popRHandler(stack_t *stk, processor *proc, textInfo *text, char **code)
{
    int reg = 0;
    if (!readArgument(proc, text, code, &reg, "POPr"))
        return;

    popRCommand(stk, reg, proc);
    printStackConsole(stk);
}
