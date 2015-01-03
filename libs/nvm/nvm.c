#include <nvm/nvm.h>
#include <nvm/hal/hal.h>

#include <error/error.h>


static struct
{
} module;


bool nvm_init(void)
{
    return true;
}

bool nvm_get_desc(unsigned char blk, NVMBlk* blkdesc)
{
    if(blk >= nvm_count())
    {
        error_code = EINVALARG;
        return false;
    }

    memcpy(blkdesc, &nvm_blocks[blk]);

    return true;
}

