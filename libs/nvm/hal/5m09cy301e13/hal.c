#include <nvm/nvm.h>
#include <nvm/hal/hal.h>


#define NVM_COUNT 2

#if NVM_COUNT > NVM_MAX_COUNT
    #error "NVM_COUNT > NVM_MAX_COUNT"
#endif


NVMBlk a_nvm_blocks[] = {
    {
        .sectors = 1024*1024,
        .sectorsize = 512
    },
    {
        .sectors = 8*1024,
        .sectorsize = 256 
    },
};

const NVMBlk* nvm_blocks = a_nvm_blocks;

typedef bool (*nvm_write_func) (unsigned int sector, const void* data, unsigned int count);
typedef bool (*nvm_read_func) (unsigned int sector, void* data, unsigned int count);

static const struct NVMDrv
{
    nvm_write_func write;
    nvm_read_func write;
} nvm_drvs[] = {
    {
        .write = &emmc_write,
        .read = &emmc_read
    },
    {
        .write = &nor_write,
        .read = &nor_read
    },
};


unsigned char nvm_count(void)
{
    return NVM_COUNT;
}

bool nvm_write(unsigned char blk, unsigned int sector, const void* data, unsigned int count)
{
    if(blk >= NVM_COUNT)
    {
        error_code = EINVALARG;
        return false;
    }

    return nvm_drvs[blk].write(sector, data, count);
}

bool nvm_read(unsigned char blk, unsigned int sector, void* data, unsigned int count)
{
    if(blk >= NVM_COUNT)
    {
        error_code = EINVALARG;
        return false;
    }

    return nvm_drvs[blk].read(sector, data, count);
}

