#ifndef EMBLOCKS_LUN_H
#define EMBLOCKS_LUN_H


struct LUNMap
{
    unsigned int base;
    unsigned int sectors;
};

struct LUNDesc
{
    unsigned char name[16];
};


bool lun_init(unsigned char data_nvmblk, unsigned char table_nvmblk, unsigned int table_offset);

unsigned char lun_max_count(void);
unsigned char lun_count(void);

bool lun_ready(unsigned char lun);
unsigned char lun_state(unsigned char lun);

unsigned int lun_sectors(unsigned char lun);

struct LUNMap* lun_map(void);
bool lun_map_flush(void);

bool lun_push_table(void);
bool lun_pop_table(void);

bool lun_read(unsigned char lun, unsigned int sector, void* data, unsigned int count);
bool lun_write(unsigned char lun, unsigned int sector, const void* data, unsigned int count);


#endif

