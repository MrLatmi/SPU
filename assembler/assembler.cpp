#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fileReader.cpp"
#include "..\header.h"

#define POISON_VALUE -13

int    getCommands      (char *readFileName, char *writeFileName);
void   commandCompound  (size_t lineCount, FILE* fileWrite, char **index);


int getCommands(char *readFileName, char *writeFileName)
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
        int  value       = 0;
        char command[5] = {};

        int nScanned = sscanf(index[i], "%4s %d", command, &value);

        if (nScanned <= 0)
            continue;

        if (strcmp(command, "PUSH") == 0)
        {
            if(nScanned < 2)
            {
                printf("Line %zu: PUSH need a number\n", i + 1);
                break;
            }
            fprintf(fileWrite, "%d %d\n", PUSH, value);
        }
        else if (strcmp(command, "ADD") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "%d\n", ADD);
        }
        else if (strcmp(command, "SUB") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "%d\n", SUB);
        }
        else if (strcmp(command, "DIV") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "%d\n", DIV);
        }
        else if (strcmp(command, "OUT") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "%d\n", OUT);
        }
        else if (strcmp(command, "HLT") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            fprintf(fileWrite, "%d\n", HLT);
        }
        else
        {
            printf("Line %zu: unknown command \"%s\"\n", i + 1, command);
            break;
        }
    }
}
