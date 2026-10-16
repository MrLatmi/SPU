#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "fileReader.cpp"
#include "..\header.h"

#define POISON_VALUE -13

int    getCommands      (char *readFileName, FILE*fileRead, FILE* fileWrite);
void   commandCompound  (size_t lineCount, FILE* fileWrite, char **index);

void   createLabel      (int *pointerLabels);

void   firstEntrance    (size_t lineCount, char** index, int *pointerLabels);

void   secondEntrance   (size_t lineCount, FILE* fileWrite, char **index, int *pointerLabels);

bool   commandPush      (const char *command, const char *args, size_t lineNum, FILE* fileWrite);
bool   commandPop       (const char *command, const char *args, size_t lineNum, FILE* fileWrite);
void   commandJump      (int opCode, const char *args, const int *pointerLabels, FILE* fileWrite);

int    registerCode     (const char *name);
bool   jumpOpCode       (const char *command, int *opCode);
bool   simpleOpCode     (const char *command, int *opCode);

int getCommands(char *readFileName, FILE*fileRead, FILE* fileWrite)
{
    textInfo text = {};
    char **index = getIndex(fileRead, readFileName, &text, lineCompare);

    if (index == nullptr)
    {
        printf("Fail\n");
        return POISON_VALUE;
    }

    if (fileWrite == NULL)
    {
        printf("Cannot open write file\n");
        free(index);
        free(text.buffer);
        return POISON_VALUE;
    }

    commandCompound(text.linesCount, fileWrite, index);


    free(index);
    free(text.buffer);
    return 1;
}


void commandCompound(size_t lineCount, FILE* fileWrite, char **index)
{
    int pointerLabels[10] = {};
    createLabel(pointerLabels);

    firstEntrance (lineCount, index, pointerLabels);
    secondEntrance(lineCount, fileWrite, index, pointerLabels);
}


void secondEntrance(size_t lineCount, FILE* fileWrite, char **index, int *pointerLabels)
{
    for (size_t i = 0; i < lineCount; i++)
    {
        char command[6] = {};
        int nScanned = sscanf(index[i], "%5s", command);


        if (nScanned <= 0 || command[0] == ':')
            continue;

        const char *args = index[i] + strlen(command);
        int opCode       = 0;

        if (strcmp(command, "PUSH") == 0)
        {
            if (!commandPush(command, args, i + 1, fileWrite))
                return;
        }
        else if (strcmp(command, "POP") == 0)
        {
            if (!commandPop(command, args, i + 1, fileWrite))
                return;
        }
        else if (simpleOpCode(command, &opCode))
        {
            fprintf(fileWrite, "%d\n", opCode);
        }
        else if (jumpOpCode(command, &opCode))
        {
            commandJump(opCode, args, pointerLabels, fileWrite);
        }
        else
        {
            printf("Line %zu: undefined command", i + 1);
            return;
        }
    }
}


bool commandPush(const char *command, const char *args, size_t lineNum, FILE* fileWrite)
{
    int value         = 0;
    char lineArgs[32] = {};

    if (sscanf(args, "%d", &value) == 1)
    {
        fprintf(fileWrite, "%d %d\n", PUSH, value);
        return true;
    }

    if (sscanf(args, "%31s", lineArgs) == 1)
    {
        int reg = registerCode(lineArgs);

        if (reg >= 0)
        {
            fprintf(fileWrite, "%d %d\n", PUSHr, reg);
            return true;
        }

        printf("Line %zu: undefined %s value", lineNum, command);
        return false;
    }

    printf("Line %zu: undefined value", lineNum);
    return false;
}


bool commandPop(const char *command, const char *args, size_t lineNum, FILE* fileWrite)
{
    char lineArgs[32] = {};

    if (sscanf(args, "%31s", lineArgs) == 1)
    {
        int reg = registerCode(lineArgs);

        if (reg >= 0)
        {
            fprintf(fileWrite, "%d %d\n", POPr, reg);
            return true;
        }

        printf("Line %zu: undefined %s value", lineNum, command);
        return false;
    }

    printf("Line %zu: undefined value", lineNum);
    return false;
}


void commandJump(int opCode, const char *args, const int *pointerLabels, FILE* fileWrite)
{
    char lineArg[32] = {};
    sscanf(args, "%31s", lineArg);

    int targetWord = 0;
    if (lineArg[0] == ':')
    {
        int label = 0;
        sscanf(lineArg + 1, "%d", &label);
        targetWord = pointerLabels[label];
    }
    else
    {
        sscanf(lineArg, "%d", &targetWord);
    }


    fprintf(fileWrite, "%d %d\n", opCode, targetWord);
}


void createLabel(int *pointerLabels)
{
    for(int i = 0; i < 10; i++)
        pointerLabels[i] = -1;
}

void firstEntrance(size_t lineCount, char** index, int *pointerLabels)
{
    size_t wordCounter = 0;

    for (size_t i = 0; i < lineCount; i++)
    {
        char command[6] = {};
        int nScanned = sscanf(index[i], "%5s", command);
        if (nScanned <= 0) continue;

        int opCode = 0;

        if (command[0] == ':')
        {
            int value = 0;
            sscanf(index[i] + 1, "%d", &value);

            pointerLabels[value] = (int)wordCounter;
        }
        else if (strcmp(command, "PUSH") == 0 || strcmp(command, "POP") == 0 ||
                 jumpOpCode(command, &opCode))
        {
            wordCounter += 2;
        }
        else if (simpleOpCode(command, &opCode))
        {
            wordCounter += 1;
        }
    }
}


int registerCode(const char *name)
{
    if      (strcmp(name, "AX") == 0) return AX;
    else if (strcmp(name, "BX") == 0) return BX;
    else if (strcmp(name, "CX") == 0) return CX;
    else if (strcmp(name, "DX") == 0) return DX;

    return -1;
}


bool jumpOpCode(const char *command, int *opCode)
{
    if      (strcmp(command, "JMP") == 0) *opCode = JMP;
    else if (strcmp(command, "JA")  == 0) *opCode = JA;
    else if (strcmp(command, "JAE") == 0) *opCode = JAE;
    else if (strcmp(command, "JB")  == 0) *opCode = JB;
    else if (strcmp(command, "JBE") == 0) *opCode = JBE;
    else if (strcmp(command, "JE")  == 0) *opCode = JE;
    else return false;

    return true;
}



bool simpleOpCode(const char *command, int *opCode)
{
    if      (strcmp(command, "ADD") == 0) *opCode = ADD;
    else if (strcmp(command, "SUB") == 0) *opCode = SUB;
    else if (strcmp(command, "DIV") == 0) *opCode = DIV;
    else if (strcmp(command, "OUT") == 0) *opCode = OUT;
    else if (strcmp(command, "HLT") == 0) *opCode = HLT;
    else return false;

    return true;
}
