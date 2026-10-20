#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "fileReader.cpp"
#include "..\header.h"

#define POISON_VALUE -13
#define LABELS_COUNT 10

struct output
{
    FILE *text;
    FILE *binary;
};

int    getCommands      (char *readFileName, FILE*fileRead, FILE* fileText, FILE* fileBinary);
void   commandCompound  (size_t lineCount, output *out, char **index);

void   createLabel      (int *pointerLabels);

void   firstEntrance    (size_t lineCount, char** index, int *pointerLabels);

void   secondEntrance   (size_t lineCount, output *out, char **index, int *pointerLabels);

bool   writeCommand     (output *out, int opCode);
bool   writeCommandArg  (output *out, int opCode, int arg);

bool   commandPush      (const char *command, const char *args, size_t lineNum, output *out);
bool   commandPop       (const char *command, const char *args, size_t lineNum, output *out);
bool   commandJump      (int opCode, const char *args, size_t lineNum, const int *pointerLabels, output *out);

int    registerCode     (const char *name);
bool   jumpOpCode       (const char *command, int *opCode);
bool   simpleOpCode     (const char *command, int *opCode);

int getCommands(char *readFileName, FILE*fileRead, FILE* fileText, FILE* fileBinary)
{
    textInfo text = {};
    char **index = getIndex(fileRead, readFileName, &text, lineCompare);

    if (index == nullptr)
    {
        printf("Fail\n");
        return POISON_VALUE;
    }

    if (fileText == NULL || fileBinary == NULL || fileRead == NULL)
    {
        printf("Cannot open file\n");
        free(index);
        free(text.buffer);
        return POISON_VALUE;
    }

    output out = { fileText, fileBinary };

    commandCompound(text.linesCount, &out, index);


    free(index);
    free(text.buffer);
    return 1;
}


void commandCompound(size_t lineCount, output *out, char **index)
{
    int pointerLabels[LABELS_COUNT] = {};
    createLabel(pointerLabels);

    firstEntrance (lineCount, index, pointerLabels);
    secondEntrance(lineCount, out, index, pointerLabels);
}


bool writeCommand(output *out, int opCode)
{
    bool binaryWrite = fwrite(&opCode, sizeof(int), 1, out->binary) == 1;
    bool textWrite   = fprintf(out->text, "%d\n", opCode) > 0;

    if (!binaryWrite || !textWrite)
    {
        printf("Write error\n");
        return false;
    }

    return true;
}


bool writeCommandArg(output *out, int opCode, int arg)
{
    int words[2] = { opCode, arg };

    bool binaryWrite = fwrite(words, sizeof(int), 2, out->binary) == 2;
    bool textWrite   = fprintf(out->text, "%d %d\n", opCode, arg) > 0;

    if (!binaryWrite || !textWrite)
    {
        printf("Write error\n");
        return false;
    }

    return true;
}


void secondEntrance(size_t lineCount, output *out, char **index, int *pointerLabels)
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
            if (!commandPush(command, args, i + 1, out))
                return;
        }
        else if (strcmp(command, "POP") == 0)
        {
            if (!commandPop(command, args, i + 1, out))
                return;
        }
        else if (simpleOpCode(command, &opCode))
        {
            if (!writeCommand(out, opCode))
                return;
        }
        else if (jumpOpCode(command, &opCode))
        {
            if (!commandJump(opCode, args, i + 1, pointerLabels, out))
                return;
        }
        else
        {
            printf("Line %zu: undefined command\n", i + 1);
            return;
        }
    }
}


bool commandPush(const char *command, const char *args, size_t lineNum, output *out)
{
    int value         = 0;
    char lineArgs[32] = {};

    if (sscanf(args, "%d", &value) == 1)
        return writeCommandArg(out, PUSH, value);

    if (sscanf(args, "%31s", lineArgs) == 1)
    {
        int reg = registerCode(lineArgs);

        if (reg >= 0)
            return writeCommandArg(out, PUSHr, reg);

        printf("Line %zu: undefined %s value\n", lineNum, command);
        return false;
    }

    printf("Line %zu: undefined value\n", lineNum);
    return false;
}


bool commandPop(const char *command, const char *args, size_t lineNum, output *out)
{
    char lineArgs[32] = {};

    if (sscanf(args, "%31s", lineArgs) == 1)
    {
        int reg = registerCode(lineArgs);

        if (reg >= 0)
            return writeCommandArg(out, POPr, reg);

        printf("Line %zu: undefined %s value\n", lineNum, command);
        return false;
    }

    printf("Line %zu: undefined value\n", lineNum);
    return false;
}


bool commandJump(int opCode, const char *args, size_t lineNum, const int *pointerLabels, output *out)
{
    char lineArg[32] = {};

    if (sscanf(args, "%31s", lineArg) != 1)
    {
        printf("Line %zu: missing jump target\n", lineNum);
        return false;
    }

    int targetWord = 0;
    if (lineArg[0] == ':')
    {
        int label = 0;
        if (sscanf(lineArg + 1, "%d", &label) != 1 ||
            label < 0 || label >= LABELS_COUNT || pointerLabels[label] < 0)
        {
            printf("Line %zu: undefined label %s\n", lineNum, lineArg);
            return false;
        }

        targetWord = pointerLabels[label];
    }
    else if (sscanf(lineArg, "%d", &targetWord) != 1)
    {
        printf("Line %zu: bad jump target %s\n", lineNum, lineArg);
        return false;
    }


    return writeCommandArg(out, opCode, targetWord);
}


void createLabel(int *pointerLabels)
{
    for(int i = 0; i < LABELS_COUNT; i++)
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

            if (value >= 0 && value < LABELS_COUNT)
                pointerLabels[value] = (int)wordCounter;
            else
                printf("Line %zu: label out of range\n", i + 1);
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
    if      (strcmp(command, "JMP")  == 0) *opCode = JMP;
    else if (strcmp(command, "JA")   == 0) *opCode = JA;
    else if (strcmp(command, "JAE")  == 0) *opCode = JAE;
    else if (strcmp(command, "JB")   == 0) *opCode = JB;
    else if (strcmp(command, "JBE")  == 0) *opCode = JBE;
    else if (strcmp(command, "JE")   == 0) *opCode = JE;
    else if (strcmp(command, "CALL") == 0) *opCode = CALL;
    else return false;

    return true;
}



bool simpleOpCode(const char *command, int *opCode)
{
    if      (strcmp(command, "ADD") == 0) *opCode = ADD;
    else if (strcmp(command, "SUB") == 0) *opCode = SUB;
    else if (strcmp(command, "DIV") == 0) *opCode = DIV;
    else if (strcmp(command, "OUT") == 0) *opCode = OUT;
    else if (strcmp(command, "IN")  == 0) *opCode = IN;
    else if (strcmp(command, "RET") == 0) *opCode = RET;
    else if (strcmp(command, "HLT") == 0) *opCode = HLT;
    else if (strcmp(command, "MUL") == 0) *opCode = MUL;
    else return false;

    return true;
}
