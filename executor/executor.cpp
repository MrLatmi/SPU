#include <TXlib.h>
#include "stackWithCanary.cpp"
#include "fileReader.cpp"
#include "../header.h"
#include "header.h"



void commandFromFileExecutor(stack_t *const stk, stack_t *const retStk, FILE* file, processor *proc)
{
    byteCode code = {};
    if (!loadByteCode(file, &code))
        return;

    while ((size_t)proc->instructionPointer < code.size && proc->instructionPointer >= 0)
    {
        int word  = code.words[proc->instructionPointer];
        int op    = word & OPCODE_MASK;
        int flags = word & ~OPCODE_MASK;

        bool jumped = false;

        switch (op)
        {
            case JMP: case JA: case JAE: case JB: case JBE: case JE:
                jumped = anyJMPCommand(stk, proc, &code, op);
                break;
            case CALL:
                jumped = callCommand(retStk, proc, &code);
                break;
            case RET:
                jumped = retCommand(retStk, proc);
                break;
            case PUSH:
                pushHandler(stk, proc, &code, flags);
                break;
            case POP:
                popHandler(stk, proc, &code, flags);
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
            case DRAW:
                drawCommand(proc);
                break;
            case HLT:
                printStackConsole(stk);
                GREEN_PRINTF("Program finished\n");

                free(code.words);
                return;

            default:
                ERROR_PRINTF("Unknown command %d at code %d ", op, proc->instructionPointer);
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
    while ((symbol = getchar()) != '\n' && symbol != EOF)
    {
        if (symbol != ' ')
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
        case JMP: condition = true;            break;
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


void drawCommand(const processor *proc)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
            printf("%c ", proc->ram[y * SCREEN_WIDTH + x] != 0 ? '*' : '.');

        printf("\n");
    }
    printf("\n");
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


void pushHandler(stack_t *stk, processor *proc, byteCode *code, int flags)
{
    if (!(flags & (FLAG_REG | FLAG_NUM)))
    {
        ERROR_PRINTF("PUSH without argument");
        return;
    }

    int value = 0;

    if (flags & FLAG_REG)
    {
        int reg = 0;
        if (!readArgument(proc, code, &reg, "PUSH"))
            return;

        if (reg < 0 || reg >= REG_COUNT)
        {
            ERROR_PRINTF("Wrong register %d", reg);
            return;
        }
        value += proc->reg[reg];
    }

    if (flags & FLAG_NUM)
    {
        int num = 0;
        if (!readArgument(proc, code, &num, "PUSH"))
            return;

        value += num;
    }

    if (flags & FLAG_MEM)
    {
        if (value < 0 || value >= RAM_SIZE)
        {
            ERROR_PRINTF("RAM address %d out of range", value);
            return;
        }
        value = proc->ram[value];
    }

    stackPush(stk, value);
    printStackConsole(stk);
}


void popHandler(stack_t *stk, processor *proc, byteCode *code, int flags)
{
    int reg = 0;
    int num = 0;

    if ((flags & FLAG_REG) && !readArgument(proc, code, &reg, "POP"))
        return;
    if ((flags & FLAG_NUM) && !readArgument(proc, code, &num, "POP"))
        return;

    if ((flags & FLAG_REG) && (reg < 0 || reg >= REG_COUNT))
    {
        ERROR_PRINTF("Wrong register %d", reg);
        return;
    }

    if (stk->size < 1)
    {
        ERROR_PRINTF("You haven't enough values");
        return;
    }

    if (flags & FLAG_MEM)
    {
        int addr = ((flags & FLAG_REG) ? proc->reg[reg] : 0) +
                   ((flags & FLAG_NUM) ? num            : 0);

        if (addr < 0 || addr >= RAM_SIZE)
        {
            ERROR_PRINTF("RAM address %d out of range", addr);
            return;
        }
        proc->ram[addr] = stackPop(stk);
    }
    else if ((flags & FLAG_REG) && !(flags & FLAG_NUM))
    {
        proc->reg[reg] = stackPop(stk);
    }
    else
    {
        ERROR_PRINTF("Bad POP argument");
        return;
    }

    printStackConsole(stk);
}
