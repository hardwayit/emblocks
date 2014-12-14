#ifndef EMBLOCKS_LUN_H
#define EMBLOCKS_LUN_H


struct LUNMap
{
    unsigned int base;
    unsigned int blocks;
};

struct LUNDesc
{
    unsigned char name[16];
};


bool lun_init(void);

unsigned char lun_max_count(void);
unsigned char lun_count(void);

bool lun_ready(unsigned char lun);
unsigned char lun_state(unsigned char lun);

LUNMap* lun_map(void);
bool lun_map_flush(void);

bool lun_push_table(void);
bool lun_pop_table(void);

bool lun_read_single(unsigned char lun, unsigned int iblock, void* buf);
bool lun_write_single(unsigned char lun, unsigned int iblock, const void* buf);


#endif

