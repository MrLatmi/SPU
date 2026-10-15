#include "disassembler.cpp"


int main(int argc, char *argv[])
{
    if  (argc != 3)
    {
        printf("Wrong argument count\n");
        return 0;
    }
    getCommands(argv[1], argv[2]);
    return 0;
}
