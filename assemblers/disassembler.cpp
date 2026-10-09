#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fileReader.cpp"

#define POISON_VALUE -13

enum commands
{
    PUSH = 1,
    ADD,
    SUB,
    DIV,
    OUT,
    HLT,
};


int    getCommands      (const char *readFileName, const char *writeFileName);
void   commandCompound  (size_t lineCount, FILE* fileWrite, char **index);



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

int getCommands(const char *const readFileName, const char *const writeFileName)
{
    textInfo text = {};
    char **index = getIndex(readFileName, &text);

    if (index == nullptr)
    {
        printf("Fail\n");
        return POISON_VALUE;
    }

    FILE *fileWrite = fopen(writeFileName, "w");

    if (fileWrite == NULL)
    {
        printf("Cannot open write file %s\n", writeFileName);
        free(index);
        free(text.buffer);
        return POISON_VALUE;
    }

    commandCompound(text.linesCount, fileWrite, index);


    fclose(fileWrite);
    free(index);
    free(text.buffer);
    return 1;
}



void commandCompound(size_t lineCount, FILE* fileWrite, char **index)
{
    for (size_t i = 0; i < lineCount; i++)
    {
        int value   = 0;
        int command = 0;

        int nScanned = sscanf(index[i], "%d %d", &command, &value);

        if (nScanned <= 0)
            continue;


        switch(command)
        {
        case PUSH:
            if(nScanned < 2)
            {
                printf("Line %zu: PUSH need a number\n", i + 1);
                break;
            }
            fprintf(fileWrite, "PUSH %d\n", value);
            break;
        case ADD:
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "ADD\n");
            break;
        case SUB:
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "SUB\n");
            break;
        case DIV:
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "DIV\n");
            break;
        case OUT:
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "OUT\n");
            break;
        case HLT:
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "HLT\n");
            break;
        default:
            printf("Line %zu: unknown command \"%d\"\n", i + 1, command);
            break;
        }
    }
}
