#include <TXlib.h>
#include "stackWithCanary.cpp"
#include "fileReader.cpp"
#include "../processor.h"
#include "executor.h"
#include "../formatingPrint.h"


static void parseCommand(int *const op, int *const flags, const int word)
{
    *op    = word & OPCODE_MASK;
    *flags = word & ~OPCODE_MASK;
}

void commandFromFileExecutor(FILE*const file, processor *const proc)
{
    byteCode code = {};
    if (!loadByteCode(file, &code))
        return;

    while (proc->instructionPointer < code.size && proc->instructionPointer >= 0)
    {
        int word  = code.words[proc->instructionPointer];
        int op    = 0;
        int flags = 0;

        parseCommand(&op, &flags, word);

        bool jumped = false;

        switch (op)
        {
            case JMP: case JA: case JAE: case JB: case JBE: case JE:
                jumped = anyJMPCommand(proc, &code, op);
                break;
            case CALL:
                jumped = callCommand(proc, &code);
                break;
            case RET:
                jumped = retCommand(proc);
                break;
            case PUSH:
                pushHandler(proc, &code, flags);
                break;
            case POP:
                popHandler(proc, &code, flags);
                break;
            case ADD:
                addCommand(proc);
                printStackConsole(proc->stk);
                break;
            case SUB:
                subCommand(proc);
                printStackConsole(proc->stk);
                break;
            case DIV:
                divCommand(proc);
                printStackConsole(proc->stk);
                break;
            case MUL:
                mulCommand(proc);
                printStackConsole(proc->stk);
                break;
            case OUT:
                outCommand(proc);
                break;
            case IN:
                inCommand(proc);
                printStackConsole(proc->stk);
                break;
            case DRAW:
                drawCommand(proc);
                break;
            case HLT:
                printStackConsole(proc->stk);
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


static bool loadByteCode(FILE*const file, byteCode *const code)
{
    int const startCapacitySize    = 64;

    int    capacity             = startCapacitySize;
    int    size                 = 0;
    int*   words                = (int*) calloc(capacity, sizeof(int));

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


static int bufferChecker()
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


static void addCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);
    int sum    = value1 + value2;
    stackPush(proc->stk, sum);
}


static bool anyJMPCommand(processor *const proc, const byteCode *const code, const int command)
{
    int nextIp = proc->instructionPointer + 1;
    if (nextIp >= code->size)
    {
        ERROR_PRINTF("Null argument for jump at code %d", proc->instructionPointer);
        return false;
    }

    int targetAddress = code->words[nextIp];

    bool condition = false;
    switch (command)
    {
        case JMP: condition = true;              break;
        case JA:  condition = jaCommand (proc);  break;
        case JAE: condition = jaeCommand(proc);  break;
        case JB:  condition = jbCommand (proc);  break;
        case JBE: condition = jbeCommand(proc);  break;
        case JE:  condition = jeCommand (proc);  break;
        default:                                 break;
    }

    if (condition)
    {
        proc->instructionPointer = targetAddress;
        return true;
    }

    proc->instructionPointer++;
    return false;
}


static bool callCommand(processor *const proc, const byteCode *const code)
{
    int targetAddress = 0;
    if (!readArgument(proc, code, &targetAddress, "CALL"))
        return false;

    if (targetAddress < 0 || targetAddress >= code->size)
    {
        ERROR_PRINTF("Bad call target %d", targetAddress);
        return false;
    }

    int returnAddress = proc->instructionPointer + 1;
    stackPush(proc->retStk, returnAddress);

    proc->instructionPointer = targetAddress;
    return true;
}


static bool retCommand(processor *const proc)
{
    if (proc->retStk->size < 1)
    {
        ERROR_PRINTF("RET without CALL at code %d", proc->instructionPointer);
        return false;
    }

    proc->instructionPointer = stackPop(proc->retStk);
    return true;
}


static void subCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);
    int sub    = value2 - value1;
    stackPush(proc->stk, sub);
}


static void divCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    if (stackTop(proc->stk) == 0)
    {
        ERROR_PRINTF("Zero division");
        return;
    }
    int value1 = stackPop(proc->stk);
    int value2 = stackPop(proc->stk);
    int div = value2 / value1;
    stackPush(proc->stk, div);
}


static void mulCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    int value1 = stackPop(proc->stk);
    int value2 = stackPop(proc->stk);
    int mul = value2 * value1;
    stackPush(proc->stk, mul);
}


static void outCommand(processor *const proc)
{
    if (proc->stk->size < 1)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return;
    }
    GREEN_PRINTF("Last operation value = <%d>\n", stackTop(proc->stk));
}


static void inCommand(processor *const proc)
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

    stackPush(proc->stk, value);
}


static void drawCommand(processor *const proc)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
            printf("%c ", proc->ram[y * SCREEN_WIDTH + x] != 0 ? '*' : '.');

        printf("\n");
    }
    printf("\n");
}


static bool jaCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);

    return value1 > value2;
}


static bool jaeCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);

    return value1 >= value2;
}


static bool jbCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);

    return value1 < value2;
}


static bool jbeCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);

    return value1 <= value2;
}


static bool jeCommand(processor *const proc)
{
    if (proc->stk->size < 2)
    {
        ERROR_PRINTF("You haven't enough values\n");
        return false;
    }

    int value1 = stackPop (proc->stk);
    int value2 = stackPop (proc->stk);

    return value1 == value2;
}


static bool readArgument(processor *const proc, const byteCode *const code, int *const value, const char *const name)
{
    proc->instructionPointer++;
    if (proc->instructionPointer >= code->size)
    {
        ERROR_PRINTF("Null argument for %s", name);
        return false;
    }

    *value = code->words[proc->instructionPointer];
    return true;
}


static void pushHandler(processor *const proc, const byteCode *const code, const int flags)
{

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

    stackPush(proc->stk, value);
    printStackConsole(proc->stk);
}


static void popHandler(processor *const proc, const byteCode *const code, const int flags)
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

    if (proc->stk->size < 1)
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
        proc->ram[addr] = stackPop(proc->stk);
    }
    else if ((flags & FLAG_REG) && !(flags & FLAG_NUM))
    {
        proc->reg[reg] = stackPop(proc->stk);
    }
    else
    {
        ERROR_PRINTF("Bad POP argument");
        return;
    }

    printStackConsole(proc->stk);
}
