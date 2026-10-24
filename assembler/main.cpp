#include "assembler.cpp"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: asm <source> <text output> <binary output>\n");
        return 0;
    }

    FILE* fileRead = fopen(argv[1], "rb");
    FILE* fileText   = fopen(argv[2], "w");
    FILE* fileBinary = fopen(argv[3], "wb");

    PRINT_NAME(fileRead);

    if (fileRead == NULL)
    {
        printf("Cannot open read file <%s>\n", argv[1]);
        return 0;
    }

    if (fileText == NULL)
    {
        printf("Cannot open text file <%s>\n", argv[2]);
        return 0;
    }

    if (fileBinary == NULL)
    {
        printf("Cannot open binary file <%s>\n", argv[2]);
        return 0;
    }

    getCommands(argv[1], fileRead, fileText, fileBinary);

    fclose(fileRead);
    fclose(fileText);
    fclose(fileBinary);

    printf("TRUE");

    return 0;
}
