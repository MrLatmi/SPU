#include "executor.cpp"

int main(int argc, char *argv[])
{
    stack_t stk = {};
    stackInit (&stk, 1 );

    stack_t retStk = {};
    stackInit (&retStk, 1 );

    processor proc = {};

    proc.stk    = &stk;
    proc.retStk = &retStk;
    if (argc < 2)
    {
        printf("Enter file name with code as first argument!!!\n");
        return 0;
    }

    FILE* file = fopen(argv[1], "rb");
    if(file == NULL)
    {
        printf("Cannot open file <%s>\n", argv[1]);
        return 0;
    }
    commandFromFileExecutor(file, &proc);
    fclose(file);
    return 0;
}
