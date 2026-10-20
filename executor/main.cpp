#include "executor.cpp"

int main(int argc, char *argv[])
{
    stack_t stk = {};
    stackInit (&stk, 1 );

    stack_t retStk = {};
    stackInit (&retStk, 1 );

    processor proc = {};

    if (argc < 2)
    {
        printf("Write code's file name as first argument\n");
        //commandFromConsoleExecutor(&stk);
        return 0;
    }

    FILE* file = fopen(argv[1], "rb");
    if(file == NULL)
    {
        printf("Cannot open file <%s>\n", argv[1]);
        return 0;
    }
    commandFromFileExecutor(&stk, &retStk, file, &proc);
    fclose(file);
    return 0;
}
