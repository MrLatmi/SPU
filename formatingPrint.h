#define ERROR_PRINTF(format, ...) printf("\033[1;31m" format " %s, %d \033[0m\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define GREEN_PRINTF(format, ...) printf("\033[1;32m" format "\033[0m\n" __VA_OPT__(,) __VA_ARGS__)

#define PRINT_NAME(variable)      printf("Variable name: %s\n", #variable)
