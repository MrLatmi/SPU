#include "disassembler.cpp"

int main(int argc, char *argv[])
{
    if  (argc != 3)
    {
        printf("Wrong argument count\n");
        return 0;
    }
    FILE* fileWrite = fopen(argv[1], "wb");
    FILE* fileRead  = fopen(argv[1], "rb");

    if(fileWrite == NULL)
    {
        printf("Can not open write file <%s>\n", argv[2]);
        return 0;
    }
    if(fileRead == NULL)
    {
        printf("Can not open asm file <%s>\n", argv[1]);
        return 0;
    }
    getCommands(argv[1], fileRead, fileWrite);
    return 0;
}
