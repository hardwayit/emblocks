#include <stdio.h>
#include <stdarg.h>

static void vtprintf(const char* module, const char* msg, va_list list)
{
    printf(module);
    printf(": ");
    vprintf(msg, list);
}

void tprintf(const char* module, const char* msg, ...)
{
    va_list list;

    va_start(list, msg);

    vtprintf(module, msg, list);

    va_end(list);
}

