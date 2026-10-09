#include <TXlib.h>
#include "stack/stackWithCanary.cpp"
#include "fileReader.cpp"
#define MAKE_DEBUG(arg) ON_DEBUG(, debug_info_t{ __FILE__, __func__, __LINE__, arg })

void commandFromFileExecutor    (stack_t *stk,const char *fileName);
void commandFromConsoleExecutor (stack_t *stk);
void addCommand                 (stack_t *stk);
void subCommand                 (stack_t *stk);
void divCommand                 (stack_t *stk);
void outCommand                 (stack_t *stk);
int  bufferChecker              ();

int main(int argc, char *argv[])
{
    stack_t stk = {};
    stackInit (&stk, 1 );
    if (argc < 2)
    {
        printf("Getting your command from console\n");
        commandFromConsoleExecutor(&stk);
        return 0;
    }
    commandFromFileExecutor(&stk, argv[1]);
    return 0;
    //Получаем буффер
    //В буфере выполняем команды
}


void commandFromFileExecutor(stack_t *stk, const char *fileName)
{
    int command = 0;
    int value   = 0;

    FILE* file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printf("Cannot open file <%s>\n", fileName);
        return;
    }
    textInfo text = {};

    char **index = getIndex(fileName, &text);

    for(size_t i = 0; i < text.linesCount; i++)
    {
        sscanf(index[i], "%d %d", &command, &value);
        switch (command)
        {
            case 1:
                stackPush(stk, value);
                break;
            case 2:
                addCommand(stk);
                break;
            case 3:
                subCommand(stk);
                break;
            case 4:
                divCommand(stk);
                break;
            case 5:
                outCommand(stk);
                break;
            case 6:
                printf("Program finished\n");
                return;
            default: ;
        }
        //printStackConsole(stk);
        value = 0;
        command = 0;
    }
}

void commandFromConsoleExecutor(stack_t *stk)
{
    int command  = 0;
    int value    = 0;
    int nScanned = 0;
    while((nScanned = scanf("%d", &command)) == 1)
    {
        switch (command)
        {
            case 1:
                if(scanf("%d", &value) != 1)
                {
                    printf("Too low arguments\n");
                    while(getchar() != '\n');
                    continue;
                }

                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }

                stackPush(stk, value);
                break;
            case 2:
                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }
                addCommand(stk);
                break;
            case 3:
                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }
                subCommand(stk);
                break;
            case 4:
                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }
                divCommand(stk);
                break;
            case 5:
                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }
                outCommand(stk);
                break;
            case 6:
                if (bufferChecker())
                {
                    printf("Too much arguments\n");
                    break;
                }
                printf("Program finished\n");
                return;
            default:
                printf("Unknown command\n");
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

void addCommand(stack_t *stk)
{
    if (stk->size < 2)
    {
        printf("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (stk );
    int value2 = stackPop (stk );
    int sum = value1 + value2;
    stackPush(stk, sum);
}


void subCommand(stack_t *stk)
{
    if (stk->size < 2)
    {
        printf("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (stk);
    int value2 = stackPop (stk);
    int sub   = value2 - value1;
    stackPush(stk, sub);
}

void divCommand(stack_t *stk)
{
    if (stk->size < 2)
    {
        printf("You haven't enough values\n");
        return;
    }
    int value1 = stackPop (stk);
    int value2 = stackPop (stk);
    if  (value1 == 0)
    {
        printf("Zero division\n");
        return;
    }
    int div = value2 / value1;
    stackPush(stk, div);
}

void outCommand(stack_t *stk)
{
    printf("Last operation value = <%d>\n", stackTop(stk));
}
