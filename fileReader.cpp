#define POISON_VALUE_READ -13


struct textInfo
{
    char   *buffer;
    size_t  symbolCount;
    size_t  linesCount;
};


int    getText          (textInfo *const info, const char fileName[]);
int    readFromFile     (textInfo *const info, FILE *const file);
int    getLinesCount    (char text[], const int symbolCount);
int    makeIndexArr     (const textInfo *const info, char *index[]);
char **getIndex         (const char *readFileName, textInfo *info);


int getLinesCount(char text[], const int symbolCount)
{
    int lineCount = 0;
    for(int i = 0; i < symbolCount; i++)
    {
        if(text[i] == '\n')
            lineCount++;

    }
    return lineCount;
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


int getText(textInfo *const info, const char fileName[])
{
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printf("Cannot open file %s\n", fileName);
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

    info->linesCount = (size_t) getLinesCount(info->buffer, (int) info->symbolCount) + 1;
    fclose(file);
    return 1;
}


int makeIndexArr(const textInfo *const info, char *index[])
{
    if (info == NULL || info->buffer == NULL)
    {
        printf("Cannot open file\n");
        return POISON_VALUE_READ;
    }

    size_t nLines = 0;
    index[nLines++] = &(info->buffer[0]);

    for (size_t i = 0; i < info->symbolCount; i++)
    {
        if (info->buffer[i] == '\n')
        {
            info->buffer[i] = '\0';
            index[nLines++] = &(info->buffer[i + 1]);
        }
    }

    return 1;
}


char **getIndex(const char *const readFileName, textInfo *const info)
{

    if (getText(info, readFileName) == POISON_VALUE_READ)
        return nullptr;

    char **index = (char **) calloc(info->linesCount, sizeof(char *));

    if (index == NULL)
    {
        printf("Problems with memory\n");
        free(info->buffer);
        return nullptr;
    }

    if (makeIndexArr(info, index) == POISON_VALUE_READ)
    {
        free(index);
        free(info->buffer);
        return nullptr;
    }


    return index;
}
