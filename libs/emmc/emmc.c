#include <emmc/emmc.h>
#include <emmc/hal/hal.h>

#ifdef EMMC_DEBUG
#include <debug/debug.h>
#endif


struct EMMC emmc;

const char* emmc_state_name[] = {
    "idle",
    "ready",
    "ident",
    "stby",
    "tran",
    "data",
    "rcv",
    "prg",
    "dis",
    "btst",
    "slp"
};


bool emmc_init(void)
{
    int i;

    if(!emmc_hal_init()) return false;

    while(1)
    {
        for(i = 1; i <= emmc.ncards; i++)
        {
            if(emmc_card_status(i)) continue;

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "Card %d current state: %s\r\n", i, emmc_state_name[emmc.status.fields.current_state]);
            #endif

            emmc_delay_ms(1000);
        }
    }

    return true;
}

