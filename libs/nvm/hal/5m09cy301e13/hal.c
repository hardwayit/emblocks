#include <nvm/nvm.h>
#include <nvm/hal/hal.h>
#include <error/error.h>

#include <emmc/emmc.h>
#include <nor/nor.h>

#define NVM_COUNT 6

#if NVM_COUNT > NVM_MAX_COUNT
    #error "NVM_COUNT > NVM_MAX_COUNT"
#endif

#define K (1024)
#define M (1024*1024)

/* eMMC:
 *   0:1M   - kernel usage
 *   1M:16M - app usage
 *   17M:~  - MSC usage
 * NOR:
 *   0:32K    - kernel usage
 *   32K:128K - firmware update usage
 *   160K:~   - app usage
 */

struct NVMBank a_nvm_banks[NVM_COUNT] = {
    { // eMMC, 1 MB, for kernel usage
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 512
        .sectors = 1*M/SECSIZE,
        .sectorsize = SECSIZE,
        .startsector = 0,
    },
    { // eMMC, 16 MB, for app usage
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 512
        .sectors = 16*M/SECSIZE,
        .sectorsize = SECSIZE,
        .startsector = 1*M/SECSIZE,
    },
    { // eMMC, the rest, for MSC usage
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 512
        .sectors = 0,
        .sectorsize = SECSIZE,
        .startsector = (1+16)*M/SECSIZE,
    },
    { // NOR, 32 kB, for kernel usage
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 256
        .sectors = 32*K/SECSIZE,
        .sectorsize = SECSIZE,
        .startsector = 0,
    },
    { // NOR, 128 kB, for firmware update usage 
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 256
        .sectors = 128*K/SECSIZE,
        .sectorsize = SECSIZE,
        .startsector = 32*K/SECSIZE,
    },
    { // NOR, the rest, for app usage
#ifdef SECSIZE
    #undef SECSIZE
#endif
#define SECSIZE 256
        .sectors = 0,
        .sectorsize = SECSIZE,
        .startsector = (32+128)*K/SECSIZE,
    },
#undef SECSIZE
};

const struct NVMBank* nvm_banks = a_nvm_banks;

typedef bool (*nvm_write_func) (unsigned int sector, const void* data, unsigned int count);
typedef bool (*nvm_read_func) (unsigned int sector, void* data, unsigned int count);

static const struct NVMDrv
{
    nvm_write_func write;
    nvm_read_func read;
} nvm_drvs[] = {
    {
        .write = &emmc_write,
        .read = &emmc_read
    },
    {
        .write = &emmc_write,
        .read = &emmc_read
    },
    {
        .write = &emmc_write,
        .read = &emmc_read
    },
    {
        .write = &nor_write,
        .read = &nor_read
    },
    {
        .write = &nor_write,
        .read = &nor_read
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

bool nvm_write(unsigned char bank, unsigned int sector, const void* data, unsigned int count)
{
    if(bank >= NVM_COUNT)
    {
        error_code = EINVALARG;
        return false;
    }

    if(sector+count > a_nvm_banks[bank].sectors)
    {
        error_code = EINVALARG;
        return false;
    }

    return nvm_drvs[bank].write(a_nvm_banks[bank].startsector+sector, data, count);
}

bool nvm_read(unsigned char bank, unsigned int sector, void* data, unsigned int count)
{
    if(bank >= NVM_COUNT)
    {
        error_code = EINVALARG;
        return false;
    }

    if(sector+count > a_nvm_banks[bank].sectors)
    {
        error_code = EINVALARG;
        return false;
    }

    return nvm_drvs[bank].read(a_nvm_banks[bank].startsector+sector, data, count);
}

