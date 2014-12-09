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

    emmc.curcard = 0xFFFF;
    emmc.blocklen = 0;
    emmc.blockcount = 0;

    if(!emmc_hal_init()) return false;

    if(!emmc_card_select(1)) {
        return false;
    }
    else {
    }

    for(i = 1; i <= emmc.ncards; i++)
    {
        if(!emmc_card_status(i)) continue;

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d current state: %s\n", i, emmc_state_name[emmc.status.fields.current_state]);
        #endif
    }

    if(!emmc_switch(0xB7, 0x02, 0x00)) {
        return false;
    }

    if(!emmc_blocklen_set(512)) {
        return false;
    }

    if(!emmc_blockcount_set(1)) {
        return false;
    }

    return true;
}

