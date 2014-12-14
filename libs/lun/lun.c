#include <lun/lun.h>

#include <nvm.h>

#define LUN_MAX 8
#define BLOCK_SIZE 512


static struct LUNMap lunmap[LUN_MAX];
static struct LUNDesc luns[LUN_MAX];


#define LUN_OFFSET (sizeof(lunmap) + sizeof(luns))


static struct
{
    bool map_valid;
    unsigned char count; 
} module;


static bool lun_check_map(const LUNMap* map)
{
    int i, j, l, m;

    for(i = 0; i < MAX_LUN-1; i++)
    {
        if(lunmap[i].blocks == 0) break;

        for(j = i+1; j < MAX_LUN; j++)
        {
            if(lunmap[j].base == lunmap[i].base) return false;

            lunmap[j].base > lunmap[i].base ? (l = j, m = i) : (l = i, m = j);

            if(lunmap[l].base < lunmap[m].base+lunmap[m].blocks) return false;
        }
    }

    return true;
}


bool lun_init(void)
{
    memset(lunmap, 0x00, sizeof(lunmap));
    memset(luns, 0x00, sizeof(luns));

    module.map_valid = false;
    module.count = 0;

    return true;
}

unsigned char lun_max_count(void)
{
    return MAX_LUN;
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

LUNMap* lun_map(void)
{
    return lunmap;
}

bool lun_map_flush(void)
{
    int i;

    if(!lun_check_map(map))
    {
        return false;
    }

    for(i = 0; i < MAX_LUN; i++)
    {
        if(lunmap[i].blocks == 0) break;
    }

    module.count = i;

    return true;
}

//bool lun_push_nvm(void)
//{
//    if(!nvm_write(0, lunmap, sizeof(lunmap)))
//    {
//        return false;
//    }
//
//    if(!nvm_write(sizeof(lunmap), luns, sizeof(luns)))
//    {
//        return false;
//    }
//
//    return true;
//}
//
//bool lun_pop_nvm(void)
//{
//    if(!nvm_read(0, lunmap, sizeof(lunmap)))
//    {
//        return false;
//    }
//
//    if(!nvm_read(sizeof(lunmap), luns, sizeof(luns)))
//    {
//        return false;
//    }
//
//    return true;
//}

bool lun_read_single(unsigned char lun, unsigned int iblock, void* buf)
{
    return emmc_read_single(LUN_OFFSET+lunmap[lun].base+iblock*BLOCK_SIZE, buf);
}

bool lun_write_single(unsigned char lun, unsigned int iblock, const void* buf);
{
    return emmc_write_single(LUN_OFFSET+lunmap[lun].base+iblock*BLOCK_SIZE, buf);
}

