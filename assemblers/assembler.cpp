#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "fileReader.cpp"
#include <./stack/stackWithCanary.cpp> //need?

#define POISON_VALUE -13

struct processor_t
{
    int instructionPointer;
    char *processorBuffer;
}
enum commands
{
    HLT,
    PUSH,
    ADD,
    SUB,
    DIV,
    OUT,
    PUSHr,
};

int    getCommands      (char *readFileName, char *writeFileName);
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
    int instructionPointer = lineCount;
    char labels[20]        = 0;

    for (size_t i = 0; i < instructionPointer; i++)
    {
        int     value      = 0;
        char    command[5] = {};
        int     label[20]  = {};
        int     nScanned   = sscanf(index[i], "%4s %s", command, &value);
        stack_t buffer     = {};
        buffer             = stackInit(&stk);


        if (nScanned <= 0)
            continue;

        if (strcmp(command, "PUSH") == 0)
        {
            if(nScanned < 2)
            {
                printf("Line %zu: PUSH need a number\n", i + 1);
                break;
            }
            if(isAlpha(value))
            {
                stackPush(buffer, PUSH);
                stackPush(buffer, value);
                fprintf(fileWrite, "%d %d\n", PUSH, value);
            }
            else if(strcmp(value, "ax"))
            {
                fprintf(fileWrite, "%d %d\n", PUSHr, value);
            }
            else if(strcmp(value, "bx"));
            {

            }
            else if(strcmp(value, "cx"))
            {

            }
            else if(strcmp(value, "dx"))
            {

            }
        }
        else if (strcmp(command, "ADD") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            stackPush(buffer, ADD);
            fprintf(fileWrite, "%d\n", ADD);
        }
        else if (strcmp(command, "SUB") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            stackPush(buffer, SUB);
            fprintf(fileWrite, "%d\n", SUB);
        }
        else if (strcmp(command, "DIV") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            stackPush(buffer, DIV);
            fprintf(fileWrite, "%d\n", DIV);
        }
        else if (strcmp(command, "OUT") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            stackPush(buffer, OUT);
            fprintf(fileWrite, "%d\n", OUT);
        }
        else if (strcmp(command, "HLT") == 0)
        {
            if(nScanned > 1)
            {
                printf("Line %zu: wrong %s usage. Command cannot exist argument\n", i + 1, command);
                break;
            }
            stackPush(buffer, HLT);
            fprintf(fileWrite, "%d\n", HLT);
        }
        else if(strcmp(command, ":"))
        {
             if(nScanned < 2)
            {
                printf("Line %zu: Label need a number\n", i + 1);\
                break;
            }
            label[value] = instructionPointer;
        }
        else
        {
            printf("Line %zu: unknown command \"%s\"\n", i + 1, command);
            break;
        }
    }
}
