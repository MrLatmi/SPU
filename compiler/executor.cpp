#include <TXlib.h>
#include "stackWithCanary.cpp"
#include "fileReader.cpp"
#include "..\header.h"
#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })


#define ERROR_PRINTF(format, ...) printf("\033[1;31m" format "%s, %d \033[0m\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define GREEN_PRINTF(format, ...) printf("\033[1;32m" format "\033[0m\n" __VA_OPT__(,) __VA_ARGS__)


void commandFromFileExecutor    (stack_t *stk,const char *fileName);
void commandFromConsoleExecutor (stack_t *stk);

int  bufferChecker              ();

void addCommand                 (stack_t *stk);
void subCommand                 (stack_t *stk);
void divCommand                 (stack_t *stk);
void outCommand                 (stack_t *stk);


void commandFromFileExecutor(stack_t *const stk, const char *fileName)
{
    int command = 0;
    int value   = 0;

    FILE* file = fopen(fileName, "rb");
    if (file == NULL)
    {
        ERROR_PRINTF("Cannot open file <%s>", fileName);
        return;
    }
    textInfo text = {};

    char **index = getIndex(fileName, &text);

    for(size_t i = 0; i < text.linesCount; i++)
    {
        sscanf(index[i], "%d %d", &command, &value);
        switch (command)
        {
            case PUSH:
                stackPush(stk, value);
                printStackConsole(stk);
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
                return;
            default:
                ERROR_PRINTF("Unknow command.");
                //assert(false);
                break;
        }
        value = 0;
        command = 0;
    }
}

void commandFromConsoleExecutor(stack_t *const stk)
{
    int command  = 0;
    int value    = 0;
    int nScanned = 0;
    while((nScanned = scanf("%d", &command)) == 1)
    {
        switch (command)
        {
            case PUSH:
                if(scanf("%d", &value) != 1)
                {
                    ERROR_PRINTF("Too low arguments");
                    while(getchar() != '\n');
                    continue;
                }

                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }

                stackPush(stk, value);
                printStackConsole(stk);
                break;
            case ADD:
                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }
                addCommand(stk);
                printStackConsole(stk);
                break;
            case SUB:
                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }
                subCommand(stk);
                printStackConsole(stk);
                break;
            case DIV:
                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }
                divCommand(stk);
                printStackConsole(stk);
                break;
            case OUT:
                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }
                outCommand(stk);
                printStackConsole(stk);
                break;
            case HLT:
                if (bufferChecker())
                {
                    ERROR_PRINTF("Too much arguments\n");
                    break;
                }
                GREEN_PRINTF("Program finished\n");
                return;
            default:
                ERROR_PRINTF("Unknown command\n");
        }
        //printStackConsole(stk);
        value = 0;
    }
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
    int value1 = stackPop (stk);
    int value2 = stackPop (stk);

    if  (value1 == 0)
    {
        ERROR_PRINTF("Zero division\n");
        return;
    }
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
