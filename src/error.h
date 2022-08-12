#ifndef ERROR_H_
#define ERROR_H_
#include "stdlib.h"
#include "stdio.h"
static void err_handle(const char *str)
{
    perror(str);
    exit(-1);
}
#endif