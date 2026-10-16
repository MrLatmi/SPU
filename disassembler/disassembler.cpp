#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "fileReader.cpp"
#include "..\header.h"

#define POISON_VALUE -13

int           getCommands      (char *readFileName, FILE*fileRead, FILE* fileWrite);
void          commandCompound  (size_t lineCount, FILE* fileWrite, char **index);

bool          commandPush      (int nScanned, int value, size_t lineNum, FILE* fileWrite);
bool          commandRegister  (const char *name, int nScanned, int value, size_t lineNum, FILE* fileWrite);
bool          commandJump      (const char *name, int nScanned, int value, size_t lineNum, FILE* fileWrite);

const char*   registerName     (int code);
const char*   jumpName         (int opCode);
const char*   simpleName       (int opCode);

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
    for (size_t i = 0; i < lineCount; i++)
    {
        int command = 0;
        int value   = 0;

        int nScanned = sscanf(index[i], "%d %d", &command, &value);

        if (nScanned <= 0)
            continue;

        const char *name = nullptr;

        if (command == PUSH)
        {
            if (!commandPush(nScanned, value, i + 1, fileWrite))
                return;
        }
        else if (command == PUSHr)
        {
            if (!commandRegister("PUSH", nScanned, value, i + 1, fileWrite))
                return;
        }
        else if (command == POPr)
        {
            if (!commandRegister("POP", nScanned, value, i + 1, fileWrite))
                return;
        }
        else if ((name = simpleName(command)) != nullptr)
        {
            if (nScanned > 1)
            {
                printf("Line %zu: wrong %d usage. Command cannot exist argument\n", i + 1, command);
                return;
            }

            fprintf(fileWrite, "%s\n", name);
        }
        else if ((name = jumpName(command)) != nullptr)
        {
            if (!commandJump(name, nScanned, value, i + 1, fileWrite))
                return;
        }
        else
        {
            printf("Line %zu: unknown command \"%d\"\n", i + 1, command);
            return;
        }
    }
}


bool commandPush(int nScanned, int value, size_t lineNum, FILE* fileWrite)
{
    if (nScanned < 2)
    {
        printf("Line %zu: PUSH need a number\n", lineNum);
        return false;
    }

    fprintf(fileWrite, "PUSH %d\n", value);
    return true;
}


bool commandRegister(const char *name, int nScanned, int value, size_t lineNum, FILE* fileWrite)
{
    if (nScanned < 2)
    {
        printf("Line %zu: %s need a register\n", lineNum, name);
        return false;
    }

    const char *reg = registerName(value);

    if (reg == nullptr)
    {
        printf("Line %zu: undefined %s value \"%d\"\n", lineNum, name, value);
        return false;
    }

    fprintf(fileWrite, "%s %s\n", name, reg);
    return true;
}


bool commandJump(const char *name, int nScanned, int value, size_t lineNum, FILE* fileWrite)
{
    if (nScanned < 2)
    {
        printf("Line %zu: %s need an address\n", lineNum, name);
        return false;
    }

    fprintf(fileWrite, "%s %d\n", name, value);
    return true;
}


const char* registerName(int code)
{
    if      (code == AX) return "AX";
    else if (code == BX) return "BX";
    else if (code == CX) return "CX";
    else if (code == DX) return "DX";

    return nullptr;
}



const char* jumpName(int opCode)
{
    if      (opCode == JMP) return "JMP";
    else if (opCode == JA)  return "JA";
    else if (opCode == JAE) return "JAE";
    else if (opCode == JB)  return "JB";
    else if (opCode == JBE) return "JBE";
    else if (opCode == JE)  return "JE";

    return nullptr;
}


const char* simpleName(int opCode)
{
    if      (opCode == ADD) return "ADD";
    else if (opCode == SUB) return "SUB";
    else if (opCode == DIV) return "DIV";
    else if (opCode == OUT) return "OUT";
    else if (opCode == HLT) return "HLT";

    return nullptr;
}
