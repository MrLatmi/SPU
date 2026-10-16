#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define POISON_VALUE_READ -13

struct textInfo
{
    char   *buffer;
    size_t  symbolCount;
    size_t  linesCount;
};

int    getText          (textInfo *const info, FILE* file, const char* fileName, int (*CompareFunc)(char symbol));
int    readFromFile     (textInfo *const info, FILE* file);
int    getElementsCount (const char text[], const size_t symbolCount, int (*CompareFunc)(char symbol));
int    makeIndexArr     (const textInfo *const info, char *index[], int (*CompareFunc)(char symbol));
char **getIndex         (FILE* file, const char* fileName, textInfo *info,  int (*CompareFunc)(char symbol));

int    lineCompare      (char symbol);
int    wordCompare      (char symbol);


int getElementsCount(const char text[], const size_t symbolCount, int (*CompareFunc)(char symbol))
{
    int count = 0;
    int inToken = 0;

    for (size_t i = 0; i < symbolCount; i++)
    {
        if (CompareFunc(text[i]))
        {
            inToken = 0;
        }
        else if (!inToken)
        {
            inToken = 1;
            count++;
        }
    }
    return count;
}



int readFromFile(textInfo *const info, FILE *const file)
{
    info->buffer = (char *) malloc(info->symbolCount + 1);
    if (info->buffer == NULL)
    {
        printf("Problems with memory\n");
        return POISON_VALUE_READ;
    }

    info->symbolCount = fread(info->buffer, 1, info->symbolCount, file);
    info->buffer[info->symbolCount] = '\0';
    return 1;
}



int getText(textInfo *const info, FILE* file, const char* fileName, int (*CompareFunc)(char symbol))
{
    if (file == NULL)
    {
        printf("Cannot open file\n");
        return POISON_VALUE_READ;
    }

    struct stat buff = {};
    stat(fileName, &buff);
    info->symbolCount = (size_t) buff.st_size;

    if (readFromFile(info, file) == POISON_VALUE_READ)
    {
        fclose(file);
        return POISON_VALUE_READ;
    }

    info->linesCount = (size_t) getElementsCount(info->buffer, info->symbolCount, CompareFunc);
    return 1;
}



int makeIndexArr(const textInfo *const info, char *index[], int (*CompareFunc)(char symbol))
{
    if (info == NULL || info->buffer == NULL || index == NULL)
    {
        printf("Invalid arguments\n");
        return POISON_VALUE_READ;
    }

    size_t nElements = 0;
    int inSpace = 0;

    if (info->symbolCount > 0 && !CompareFunc(info->buffer[0]))
    {
        index[nElements++] = &(info->buffer[0]);
        inSpace = 1;
    }

    for (size_t i = 0; i < info->symbolCount; i++)
    {
        if (CompareFunc(info->buffer[i]))
        {
            info->buffer[i] = '\0';
            inSpace = 0;
        }
        else
        {
            if (!inSpace)
            {
                index[nElements++] = &(info->buffer[i]);
                inSpace = 1;
            }
        }
    }

    return 1;
}



int lineCompare(char symbol)
{
    return (symbol == '\n');
}

int wordCompare(char symbol)
{
    return (symbol == '\n' || symbol == ' ' || symbol == '\r' || symbol == '\t');
}


char **getIndex(FILE* file, const char* fileName, textInfo *const info, int (*CompareFunc)(char symbol))
{
    if (getText(info, file, fileName, CompareFunc) == POISON_VALUE_READ)
        return nullptr;

    char **index = (char **) calloc(info->linesCount, sizeof(char *));

    if (index == NULL)
    {
        printf("Problems with memory\n");
        free(info->buffer);
        return nullptr;
    }

    if (makeIndexArr(info, index, CompareFunc) == POISON_VALUE_READ)
    {
        free(index);
        free(info->buffer);
        return nullptr;
    }

    return index;
}
