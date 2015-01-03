#include <error/error.h>

// Dependencies:
#include <debug/debug.h>
#include <sys/sys.h>

#include <stdarg.h>


#define ERROR_PRIORITY 1


int error_code;


void error_throw(bool critical, const char* filename, unsigned int line, const char* funcname, const char* msg, ...)
{
    va_list args;

    va_start(args, msg);

    debug_printf(ERROR_PRIORITY, "<%s:%d>:\n  in function %s:\n", filename, line, funcname);
    debug_vprintf(ERROR_PRIORITY, msg, args);

    va_end(args);

    if(critical) //while(1);
    {
        led_set(1, 0);
        sys_delay_ms(1000);
        led_set(1, 1);
        sys_delay_ms(200);
    }
}

void error_msg_set(const char* msg)
{
}

