#ifndef DEBUG_HAL_H
#define DEBUG_HAL_H

#include <stdarg.h>


bool debug_hal_init(void);
void debug_vprintf(int priority, const char* format, va_list args);

#endif

