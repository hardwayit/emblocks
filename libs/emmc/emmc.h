/*
 * emmc.h
 *
 *  Created on: 21.11.2014
 *      Author: ALev
 */

#ifndef EMB_EMMC_H
#define EMB_EMMC_H

//#define EMMC_DEBUG

#ifdef EMMC_DEBUG
#define EMMC_DEBUG_LVL 4
#endif

#define EMMC_ST_IDLE   0
#define EMMC_ST_READY  1
#define EMMC_ST_IDENT  2
#define EMMC_ST_STBY   3
#define EMMC_ST_TRAN   4
#define EMMC_ST_DATA   5
#define EMMC_ST_RCV    6
#define EMMC_ST_PRG    7
#define EMMC_ST_DIS    8
#define EMMC_ST_BTST   9
#define EMMC_ST_SLP   10


bool emmc_init(void);

const char* emmc_state_name(unsigned char state);
unsigned char emmc_card_state(unsigned short rca);
unsigned char emmc_lastcmd(unsigned short rca);

bool emmc_card_status(unsigned short rca);
bool emmc_card_select(unsigned short rca);
bool emmc_switch(unsigned char index, unsigned char value, unsigned char cmdset);
bool emmc_blocklen_set(unsigned int len);
bool emmc_blockcount_set(unsigned int count);
bool emmc_read_single_block(unsigned int iblock, unsigned char* buf);
bool emmc_write_single_block(unsigned int iblock, const unsigned char* buf);

bool emmc_write(unsigned int iblock, const void* buf, unsigned int blocks);
bool emmc_read(unsigned int iblock, void* buf, unsigned int blocks);


#endif /* EMB_EMMC_H */

