#ifndef EMBLOCKS_NVM_H
#define EMBLOCKS_NVM_H


#define NVM_MAX_COUNT 8


struct NVMBank
{
    unsigned int sectors;
    unsigned short sectorsize;
    unsigned int startsector;
};


bool nvm_init(void);
unsigned char nvm_count(void);

bool nvm_write(unsigned char bank, unsigned int sector, const void* data, unsigned int count);
bool nvm_read(unsigned char bank, unsigned int sector, void* data, unsigned int count);


#endif

