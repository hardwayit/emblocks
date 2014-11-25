#include <debug/debug.h>
#include <debug/hal/hal.h>


bool debug_init(void)
{
    if(!debug_hal_init()) return false;

    return true;
}

void debug_printf(int priority, const char* format, ...)
{
    va_list args;

    va_start(args, format);

    debug_vprintf(priority, format, args);

    va_end(args);
}

