#include "stackHeader.h"



void            canaryAdder         (stack_element_t *canaryData, size_t capacity);
errors_t        canaryChecker       (const stack_t *const stk             ON_DEBUG(, debug_info_t debugInfo));

long long       djb2_hash_stackStruc(const stack_t *const stk);
long long       djb2_hash_stackData (const stack_t *const stk);
errors_t        hashChecker         (      stack_t *stk                   ON_DEBUG(, debug_info_t debugInfo));
void            changeCash          (stack_t *const stk);

errors_t        mainVerifier        (const stack_t *const stk             ON_DEBUG(, debug_info_t debugInfo));
void            printStackConsole   (const stack_t *stk                   ON_DEBUG(, debug_info_t debugInfo));
const char*     getErrorName        (errors_t err);

bool            popRealloc          (stack_t *const stk);
bool            pushRealloc         (stack_t *const stk);
bool            changeMemoryUpStack (stack_t *const stk, size_t newCapacity);



#ifdef STK_DEBUG
void makeDebugLog(const stack_t *const stk, debug_info_t debugInfo, errors_t errCode)
{
    FILE *file = nullptr;
    if((file = fopen(debugInfo.fileName, "ab")) == NULL)
    {
        printf("Cannot open file for debug\n");\
        assert(NULL);
        return;
    }

    if (stk == NULL)
    {

        fprintf(file, "Stack wasn't initialised. Created by %s() at %s:%d\n Error code: %d - %s\n",
                debugInfo.func, debugInfo.file, debugInfo.line, errCode, getErrorName(errCode));
        fclose(file);
        return;
    }

    fprintf(file, "created by %s() at %s:%d\n Error code: %d - %s\n Size = <%zu>\n Capacity = <%zu>\n data[%p]:\n",
            debugInfo.func, debugInfo.file, debugInfo.line, errCode, getErrorName(errCode), stk->size, stk->capacity, (void*)stk->data);

    if(stk->data != 0)
    {
        fprintf(file, "{\n");

        for (size_t i = 0; i < stk->capacity; i++)
        {
            if(i < stk->size)
                fprintf(file, "* ");

            fprintf(file, "[%zu] = <%d>\n", i, stk->data[i]);
        }

        fprintf(file, "}\n");
    }

    fprintf(file, "\n\n\n\n\n");
    fclose(file);
}
#endif



long long djb2_hash_stackData(const stack_t *const stk)
{
    long long hash = 5381;
    for(size_t i = 0; i < stk->capacity + CANARY_COUNT;i++)
        hash = ((hash<<5) + hash) + (long long)stk->canaryData[i];

    return hash;
}

long long djb2_hash_stackStruc(const stack_t *const stk)
{
    long long hash = 5381;
    const unsigned char *bytes = (const unsigned char*) stk;

    for (size_t i = 0; i < sizeof(stack_t); i++)
        hash = ((hash << 5) + hash) + bytes[i];

    return hash;
}


errors_t hashChecker(stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{
    long long hashDataTemp   = stk->hashData;
    long long hashStructTemp = stk->hashStruct;
    stk->hashData   = 0;
    stk->hashStruct = 0;
    long long hashData   = djb2_hash_stackData(stk);
    long long hashStruct = djb2_hash_stackStruc(stk);
    stk->hashData   = hashDataTemp;
    stk->hashStruct = hashStructTemp;

    if (hashDataTemp != hashData || hashStructTemp != hashStruct)
    {
        printf("Your hash was changed\n");
    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, hashError);
    #endif
        return hashError;
    }
    return noProblem;
}

void changeCash(stack_t *const stk)
{
    stk->hashData        = 0;
    stk->hashStruct      = 0;
    long long hashData   = djb2_hash_stackData(stk);
    long long hashStruct = djb2_hash_stackStruc(stk);

    stk->hashData        = hashData;
    stk->hashStruct      = hashStruct;
}

void canaryAdder(stack_element_t *canaryData, size_t capacity)
{
    canaryData[0]            = CANARY_LEFT;
    canaryData[capacity + 1] = CANARY_RIGHT;
}


void stackInit(stack_t *const stk, size_t capacity ON_DEBUG(, debug_info_t debugInfo))
{
    if (!stk || capacity == 0 || capacity > CAPACITY_MAX)
        return;

    stk->canaryUp   = CANARY_STUCT_UP;
    stk->canaryDown = CANARY_STUCT_DOWN;
    stk->size       = 0;
    stk->capacity   = capacity;
    stk->hashData   = 0;
    stk->hashStruct = 0;

    stk->canaryData = (stack_element_t*) calloc(capacity + CANARY_COUNT, sizeof(stack_element_t));
    if (stk->canaryData == NULL)
    {
        stk->data = NULL;
        return;
    }
    stk->data = &stk->canaryData[1];

    for (size_t i = 0; i < capacity; i++)
        stk->data[i] = POISON_VALUE;

    canaryAdder(stk->canaryData, capacity);
    changeCash(stk);

    if (mainVerifier(stk ON_DEBUG(, debugInfo)) != noProblem)
        fprintf(stderr, "Critical error. Check debug file\n");
}


void stackDestroy(stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{
    if(mainVerifier(stk ON_DEBUG(, debugInfo)) != noProblem)
        return;

    free(stk->canaryData);
    stk->data     = nullptr;
    stk->capacity = 0;
    stk->size     = 0;
    free(stk);
}


void stackPush(stack_t *const stk, stack_element_t value ON_DEBUG(, debug_info_t debugInfo))
{
    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
    {
        fprintf(stderr, "Critical error. Check debug file\n");
        return;
    }

    if (hashChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
    {
        fprintf(stderr, "Critical error. Check debug file\n");
        return;
    }

    if(!pushRealloc(stk))
        return;

    stk->data[stk->size++] = value;
    changeCash(stk);


    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
    {
        fprintf(stderr, "Critical error. Check debug file\n");
        return;
    }
}




void printStackConsole(const stack_t *stk ON_DEBUG(, debug_info_t debugInfo))
{
    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return;


    printf(" size = %zu, capacity = %zu\n", stk->size, stk->capacity);

    printf("Elements:\n");
    for (size_t i = 0; i < stk->size; i++)
    {
        printf("\033[1;33m[%zu] = <%d>\033[0m\n", i, stk->data[i]);
    }
}




errors_t canaryChecker(const stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{
    errors_t verifierErr = mainVerifier(stk ON_DEBUG(, debugInfo));
    if (verifierErr != noProblem)
        return verifierErr;

    if (stk->canaryUp != CANARY_STUCT_UP)
    {
        fprintf(stderr, "Your UP canary dead:(. Check memory\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, canaryUpDead);
    #endif

        return canaryUpDead;
    }

    if (stk->canaryDown != CANARY_STUCT_DOWN)
    {
        fprintf(stderr, "Your DOWN canary dead:(. Check memory\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, canaryDownDead);
    #endif

        return canaryDownDead;
    }

    if (stk->canaryData[0] != (stack_element_t) CANARY_LEFT)
    {
        fprintf(stderr, "Your LEFT canary dead:(. Check memory\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, canaryLeftDead);
    #endif

        return canaryLeftDead;
    }
    if (stk->canaryData[stk->capacity+1] != (stack_element_t) CANARY_RIGHT)
    {
        fprintf(stderr, "Your RIGHT canary dead:(. Check memory\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, canaryRightDead);
    #endif

        return canaryRightDead;
    }
    return noProblem;
}





errors_t mainVerifier(const stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{
    if(stk == NULL)
    {
        fprintf(stderr, "Function called with NULL pointer. Check debug file\n");

    #ifdef STK_DEBUG
        makeDebugLog(NULL, debugInfo, nullProblem);
    #endif

        return nullProblem;
    }

    if (stk->canaryData == NULL || stk->data == NULL)
    {
        fprintf(stderr, "Memory error\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, callocError);
    #endif
        return callocError;
    }


    if (stk->capacity > CAPACITY_MAX)
    {
        fprintf(stderr, "Capacity error\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, capacityProblem);
    #endif

        return capacityProblem;
    }


    if (stk->size > stk->capacity || stk->size > CAPACITY_MAX)
    {
        fprintf(stderr, "Size error\n");

    #ifdef STK_DEBUG
        makeDebugLog(stk, debugInfo, sizeProblem);
    #endif

        return sizeProblem;
    }

    return noProblem;
}






stack_element_t stackPop(stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{

    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;


    if (hashChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;

    if(stk->size == 0)
    {
        fprintf(stderr, "Void stack\n");
        return (stack_element_t) POISON_VALUE;
    }

    stk->size--;
    stack_element_t returnedData = stk->data[stk->size];
    stk->data[stk->size] = POISON_VALUE;
    popRealloc(stk);
    changeCash(stk);
    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;

    return returnedData;
}



bool popRealloc(stack_t *const stk)
{
    if (stk->capacity > 4 && stk->size == stk->capacity/4)
    {
        size_t newCapacity = (stk->capacity / 4);


        stack_element_t* temp = (stack_element_t*) realloc(stk->canaryData, (newCapacity + CANARY_COUNT) * sizeof(stack_element_t));

        if(temp == NULL)
        {
            fprintf(stderr, "Error with memory getting\n");
            return false;
        }

        canaryAdder(temp, newCapacity);

        stk->canaryData  = temp;
        stk->data = &stk->canaryData[1];
        stk->capacity = newCapacity;

        changeCash(stk);
    }
    return true;
}


bool pushRealloc(stack_t *const stk)
{
    if (stk->size == stk->capacity)
    {
        size_t newCapacity = stk->capacity ? stk->capacity * 2 : 1;
        if (newCapacity > CAPACITY_MAX)
        {
            fprintf(stderr, "Capacity limit\n");
            return false;
        }

        if (!changeMemoryUpStack(stk, newCapacity))
        {
            printf("Memory getting problem\n");
            return false;
        }
    }
    return true;
}

stack_element_t stackTop(stack_t *const stk ON_DEBUG(, debug_info_t debugInfo))
{
    if (mainVerifier(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;

    if (canaryChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;

    if (hashChecker(stk ON_DEBUG(, debugInfo)) != noProblem)
        return (stack_element_t) POISON_VALUE;

    if (stk->size == 0)
    {
        fprintf(stderr, "Void stack\n");
        return (stack_element_t) POISON_VALUE;
    }

    return stk->data[stk->size - 1];
}

bool changeMemoryUpStack(stack_t *const stk, size_t newCapacity)
{
    if(newCapacity < stk->capacity)
        return false;

    if(newCapacity > CAPACITY_MAX)
        return false;

    stack_element_t *temp = (stack_element_t*) realloc (stk->canaryData, (newCapacity +CANARY_COUNT )*sizeof(stack_element_t));

    if (temp == NULL)
        return false;


    for (size_t i = stk->capacity; i < newCapacity; i++)
    {
        temp[i + 1] = POISON_VALUE;
    }
    canaryAdder(temp, newCapacity);
    stk->canaryData = temp;
    stk->data = &stk->canaryData[1];
    stk->capacity = newCapacity;

    changeCash(stk);
    return true;
}

const char* getErrorName(errors_t err)
{
    switch(err)
    {
        case noProblem:
            return "no problem";
        case callocError:
            return "calloc problem";
        case capacityProblem:
            return "capacity problem";
        case sizeProblem:
            return "size problem";
        case alreadyInit:
            return "already init problem";
        case initProblem:
            return "initialize problem";
        case nullProblem:
            return "null pointer problem";
        case canaryLeftDead:
            return "left canary problem";
        case canaryRightDead:
            return "right canary problem";
        case canaryUpDead:
            return "upper canary in stack problem";
        case canaryDownDead:
            return "downer canary in stack problem";
        case hashError:
            return "hash problem";
        default:
            return "undefined problem";

    }
}
