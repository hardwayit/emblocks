#define __MODULE__ "emmc"

#include <emmc/emmc.h>
#include <emmc/hal/hal.h>
#include <error/error.h>

#ifdef EMMC_DEBUG
  #include <debug/debug.h>
#endif


#define EMMC_BLOCK_SIZE 512


struct EMMC emmc;


const char* state_name[] = {
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

const unsigned int state_name_count = sizeof(state_name)/sizeof(state_name[0]);


const char* emmc_state_name(unsigned char state)
{
    if(state >= state_name_count) error_critical("Bad state name.", 0);
    return state_name[state];
}

unsigned char emmc_card_state(unsigned short rca)
{
    return emmc.status.fields.current_state;
}

unsigned char emmc_lastcmd(unsigned short rca)
{
    return emmc.lastcmd;
}

bool emmc_init(void)
{
    int i;

    emmc.curcard = 0xFFFF;
    emmc.blocklen = 0;
    emmc.blockcount = 0;

    if (!emmc_hal_init()) {
        error_msg("eMMC HAL initialization failed!\n", 0);
        return false;
    }

    if (!emmc_card_select(1)) {
        error_msg("Error selecting card.\n", 0);
        return false;
    }

    for (i = 1; i <= emmc.ncards; i++)
    {
        if(!emmc_card_status(i)) continue;

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d current state: %s\n", i, emmc_state_name(emmc.status.fields.current_state));
        #endif
    }

    if (!emmc_switch(0xB7, 0x02, 0x00)) {
        error_msg("Error invoking switch command.\n", 0);
        return false;
    }

    if (!emmc_blocklen_set(EMMC_BLOCK_SIZE)) {
        error_msg("Error invoking set block length command.\n", 0);
        return false;
    }

    if (!emmc_blockcount_set(1)) {
        error_msg("Error invoking set block count command.\n", 0);
        return false;
    }

    return true;
}

bool emmc_deinit(void)
{
    if (!emmc_hal_deinit()) {
        error_msg("Error stopping eMMC HAL.\n", 0);
        return false;
    }

    return true;
}


bool emmc_write(unsigned int iblock, const void* buf, unsigned int blocks)
{
    unsigned int i;
    const unsigned char* cbuf = (const unsigned char*) buf;

    for (i = 0; i < blocks; i++) {
        if (!emmc_write_single_block(iblock+i, &cbuf[i*EMMC_BLOCK_SIZE])) {
            return false;
        }
    }

    return true;
}


bool emmc_read(unsigned int iblock, void* buf, unsigned int blocks)
{
    unsigned int i;
    unsigned char* cbuf = (unsigned char*) buf;

    for (i = 0; i < blocks; i++) {
        if (!emmc_read_single_block(iblock+i, &cbuf[i*EMMC_BLOCK_SIZE])) {
            return false;
        }
    }

    return true;
}


