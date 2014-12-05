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

unsigned char buf[1024];

bool emmc_init(void)
{
    char res;
    int i;

    emmc.curcard = 0xFFFF;
    emmc.blocklen = 0;

    if(!emmc_hal_init()) return false;

    if((res = emmc_card_select(1))) {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d selection error [%d]\r\n", 1, res);
        #endif

        return false;
    }
    else {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d selected [%d]\r\n", 1, res);
        #endif
    }

    for(i = 1; i <= emmc.ncards; i++)
    {
        if(emmc_card_status(i)) continue;

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d current state: %s\r\n", i, emmc_state_name[emmc.status.fields.current_state]);
        #endif
    }

    if((res = emmc_switch(0xB7, 0x02, 0x00))) {
        return false;
    }

    if((res = emmc_blocklen_set(512))) {
        return false;
    }

    for(i = 0; i < 512; i++)
    {
        buf[i] = i;
    }

    emmc_write_single_block(0, buf);

    //memset(buf, 0xAA, 512);

    //emmc_read_single_block(0, buf);

    //for(i = 0; i < 16; i++)
    //    debug_printf(0, "%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
    //                 buf[i*16+ 0], buf[i*16+ 1], buf[i*16+ 2], buf[i*16+ 3],
    //                 buf[i*16+ 4], buf[i*16+ 5], buf[i*16+ 6], buf[i*16+ 7],
    //                 buf[i*16+ 8], buf[i*16+ 9], buf[i*16+10], buf[i*16+11],
    //                 buf[i*16+12], buf[i*16+13], buf[i*16+14], buf[i*16+15]);

    return true;
}

