#ifndef EMBLOCKS_NVM_H
#define EMBLOCKS_NVM_H


#define NVM_MAX_COUNT 4


struct NVMBlk
{
    unsigned int sectors;
    unsigned short sectorsize;
};


bool nvm_init(void);
unsigned char nvm_count(void);
bool nvm_get_desc(unsigned char blk, NVMBlk* blkdesc);

bool nvm_write(unsigned char blk, unsigned int sector, const void* data, unsigned int count);
bool nvm_read(unsigned char blk, unsigned int sector, void* data, unsigned int count);


#endif

