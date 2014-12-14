#include <sys/sys.h>

#include <cyu3system.h>
#include <cyu3os.h>


void sys_delay_ms(unsigned int delay)
{
    CyU3PThreadSleep(delay);
}

