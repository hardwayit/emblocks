#include <lun/lun.h>

#include <nvm/nvm.h>
#include <emmc/emmc.h>
#include <error/error.h>


#define LUN_MAX 8
#define BLOCK_SIZE 512


static struct LUNMap lunmap[LUN_MAX];
static struct LUNDesc luns[LUN_MAX];


static struct
{
    bool map_valid;
    unsigned char count; 
    unsigned char data_nvmblk;
    unsigned char table_nvmblk;
    unsigned int table_offset;
} module;


static bool lun_check_map(void)
{
    int i, j, l, m;

    for(i = 0; i < LUN_MAX-1; i++)
    {
        if(lunmap[i].sectors == 0) break;

        for(j = i+1; j < LUN_MAX; j++)
        {
            if(lunmap[j].base == lunmap[i].base) return false;

            lunmap[j].base > lunmap[i].base ? (l = j, m = i) : (l = i, m = j);

            if(lunmap[l].base < lunmap[m].base+lunmap[m].sectors) return false;
        }
    }

    return true;
}


bool lun_init(unsigned char data_nvmblk, unsigned char table_nvmblk, unsigned int table_offset)
{
    memset(lunmap, 0x00, sizeof(lunmap));
    memset(luns, 0x00, sizeof(luns));

    module.map_valid = false;
    module.count = 0;

    if(data_nvmblk > nvm_count()-1 || table_nvmblk > nvm_count()-1)
    {
        error_code = EINVALARG;
        return false;
    }

    module.data_nvmblk  = data_nvmblk;
    module.table_nvmblk = table_nvmblk;
    module.table_offset = table_offset;

    return true;
}

unsigned char lun_max_count(void)
{
    return LUN_MAX;
}

unsigned char lun_count(void)
{
    return module.count;
}

bool lun_ready(unsigned char lun)
{
    if(emmc_lastcmd(1) == 24)
    {
        if(!emmc_card_status(1))
        {
            error_msg("Error get card status.\n", 0);

            return false;
        }
    }

    if(emmc_card_state(1) == EMMC_ST_TRAN) return true;

    return false;
}

unsigned char lun_state(unsigned char lun)
{
    return emmc_card_state(1);
}

unsigned int lun_sectors(unsigned char lun)
{
    return lunmap[lun].sectors;
}

struct LUNMap* lun_map(void)
{
    return lunmap;
}

bool lun_map_flush(void)
{
    int i;

    if(!lun_check_map())
    {
        return false;
    }

    for(i = 0; i < LUN_MAX; i++)
    {
        if(lunmap[i].sectors == 0) break;
    }

    module.count = i;

    return true;
}

bool lun_push_nvm(void)
{
    if(!nvm_write(module.table_nvmblk, module.table_offset, lunmap, sizeof(lunmap)))
    {
        error_code = EHWIO;
        return false;
    }

    if(!nvm_write(module.table_nvmblk, module.table_offset+sizeof(lunmap), luns, sizeof(luns)))
    {
        error_code = EHWIO;
        return false;
    }

    return true;
}

bool lun_pop_nvm(void)
{
    if(!nvm_read(module.table_nvmblk, module.table_offset, lunmap, sizeof(lunmap)))
    {
        error_code = EHWIO;
        return false;
    }

    if(!nvm_read(module.table_nvmblk, module.table_offset+sizeof(lunmap), luns, sizeof(luns)))
    {
        error_code = EHWIO;
        return false;
    }

    return true;
}

bool lun_write(unsigned char lun, unsigned int sectors, const void* buf, unsigned int count)
{
    return nvm_write(module.data_nvmblk, lunmap[lun].base+sectors*BLOCK_SIZE, buf, count);
}

bool lun_read(unsigned char lun, unsigned int sectors, void* buf, unsigned int count)
{
    return nvm_read(module.data_nvmblk, lunmap[lun].base+sectors*BLOCK_SIZE, buf, count);
}

