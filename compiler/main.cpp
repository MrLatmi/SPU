#include "executor.cpp"

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
}
