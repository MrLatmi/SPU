#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "fileReader.cpp"
#include "../processor.h"
#include "assembler.h"
#define POISON_VALUE -13
#define LABELS_COUNT 10






int getCommands(char *readFileName, FILE *fileRead, FILE *fileText, FILE *fileBinary)
{
    if (fileText == NULL || fileBinary == NULL || fileRead == NULL)
    {
        printf("Missing file\n");
        return POISON_VALUE;
    }

    textInfo text = {};
    char **index = getIndex(fileRead, readFileName, &text, lineCompare);

    if (index == nullptr)
    {
        printf("Fail\n");
        return POISON_VALUE;
    }

    output out = { fileText, fileBinary };

    bool ok = commandCompound(text.linesCount, &out, index);

    free(index);
    free(text.buffer);
    return ok ? 1 : POISON_VALUE;
}


bool commandCompound(size_t lineCount, output *out, char **index)
{
    int pointerLabels[LABELS_COUNT] = {};
    createLabel(pointerLabels);

    if (!firstEntrance(lineCount, index, pointerLabels))
        return false;

    return secondEntrance(lineCount, out, index, pointerLabels);
}


bool writeWords(output *out, const int *words, int n)
{
    assert(out   != 0);
    assert(words != 0);



    if(!out)
    {
        printf("Null pointer to files struct");
        return false;
    }

    if(!words)
    {
        printf("Null pointer to command words");
        return false;
    }

    bool binaryWrite = (fwrite(words, sizeof(int), n, out->binary) == (size_t)n);
    bool textWrite   = true;

    for (int i = 0; i < n; i++)
        textWrite = (fprintf(out->text, i + 1 < n ? "%d " : "%d\n", words[i])) > 0;

    if (!binaryWrite || !textWrite)
    {
        printf("Write error\n");
        return false;
    }

    return true;
}


bool parseOperand(const char *args, operand *oper)
{
    char name[32] = {};
    int  value    = 0;
    *oper = {};

    if (sscanf(args, " [ %31[^+ ] + %d ]", name, &value) == 2)
        *oper = { true,  true,  true,  0, value };
    else if (sscanf(args, " [ %31[^] ] ]", name) == 1)
    {
        if (sscanf(name, "%d", &value) == 1)
            *oper = { true, false, true,  0, value };
        else
            *oper = { true, true,  false, 0, 0     };
    }
    else if (sscanf(args, " %31[^+ ] + %d", name, &value) == 2)
        *oper = { false, true,  true,  0, value };
    else if (sscanf(args, " %d", &value) == 1)
        *oper = { false, false, true,  0, value };
    else if (sscanf(args, " %31s", name) == 1)
        *oper = { false, true,  false, 0, 0     };
    else
        return false;

    if (oper->hasReg)
    {
        oper->reg = registerCode(name);
        if (oper->reg < 0)
            return false;
    }

    return true;
}


int operandWords(const operand *oper)
{
    return 1 + (oper->hasReg ? 1 : 0) + (oper->hasNum ? 1 : 0);
}

//TODO: const for func, defines for FLAG (not enum)
bool writeOperand(output *out, int opCode, const operand *oper)
{
    int words[3] = {};
    int n        = 0;
    int flag     = 0;

    if(oper->mem)    flag |= FLAG_MEM;
    if(oper->hasReg) flag |= FLAG_REG;
    if(oper->hasNum) flag |= FLAG_NUM;

    words[n++] = opCode | flag;
    if (oper->hasReg) words[n++] = oper->reg;
    if (oper->hasNum) words[n++] = oper->value;

    return writeWords(out, words, n);
}


bool commandPush(const char *command, const char *args, size_t lineNum, output *out)
{
    operand oper = {};

    if (!parseOperand(args, &oper))
    {
        printf("Line %zu: bad %s argument\n", lineNum, command);
        return false;
    }

    return writeOperand(out, PUSH, &oper);
}


bool commandPop(const char *command, const char *args, size_t lineNum, output *out)
{
    operand oper = {};

    if (!parseOperand(args, &oper))
    {
        printf("Line %zu: bad %s argument\n", lineNum, command);
        return false;
    }

    return writeOperand(out, POP, &oper);
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

    int words[2] = { opCode, targetWord };
    return writeWords(out, words, 2);
}


void createLabel(int *pointerLabels)
{
    for (int i = 0; i < LABELS_COUNT; i++)
        pointerLabels[i] = -1;
}


bool firstEntrance(size_t lineCount, char **index, int *pointerLabels)
{
    size_t wordCounter = 0;

    for (size_t i = 0; i < lineCount; i++)
    {
        char command[16] = {};
        int  used        = 0;
        int  nScanned    = sscanf(index[i], "%15s%n", command, &used);
        if (nScanned <= 0) continue;

        int opCode = 0;

        if (command[0] == ':')
        {
            int value = 0;
            sscanf(command + 1, "%d", &value);

            if (value >= 0 && value < LABELS_COUNT)
                pointerLabels[value] = (int)wordCounter;
            else
                printf("Line %zu: label out of range\n", i + 1);
        }
        else if (strcmp(command, "PUSH") == 0 || strcmp(command, "POP") == 0)
        {
            operand a = {};
            if (!parseOperand(index[i] + used, &a))
            {
                printf("Line %zu: bad %s argument\n", i + 1, command);
                return false;
            }

            wordCounter += operandWords(&a);
        }
        else if (jumpOpCode(command, &opCode))
        {
            wordCounter += 2;
        }
        else if (simpleOpCode(command, &opCode))
        {
            wordCounter += 1;
        }
    }

    return true;
}


bool secondEntrance(size_t lineCount, output *out, char **index, int *pointerLabels)
{
    for (size_t i = 0; i < lineCount; i++)
    {
        const int commandMaxLength     = 16;
        char command[commandMaxLength] = {};
        int  used                      = 0;
        int  nScanned                  = sscanf(index[i], "%16s%n", command, &used);

        if (nScanned <= 0 || command[0] == ':')
            continue;

        const char *args = index[i] + used;
        int opCode       = 0;

        if (strcmp(command, "PUSH") == 0)
        {
            if (!commandPush(command, args, i + 1, out))
                return false;
        }
        else if (strcmp(command, "POP") == 0)
        {
            if (!commandPop(command, args, i + 1, out))
                return false;
        }
        else if (simpleOpCode(command, &opCode))
        {
            if (!writeWords(out, &opCode, 1))
                return false;
        }
        else if (jumpOpCode(command, &opCode))
        {
            if (!commandJump(opCode, args, i + 1, pointerLabels, out))
                return false;
        }
        else
        {
            printf("Line %zu: undefined command\n", i + 1);
            return false;
        }
    }

    return true;
}

//TODO: Создать одну функцию для одинаковых strcmp, чтобы не путаться и быть уверенным, что все в одном месте

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
    else if (strcmp(command, "DRAW") == 0) *opCode = DRAW;
    else return false;

    return true;
}
