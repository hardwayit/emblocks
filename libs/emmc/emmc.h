/*
 * emmc.h
 *
 *  Created on: 21.11.2014
 *      Author: ALev
 */

#ifndef EMBLOCKS_EMMC_H
#define EMBLOCKS_EMMC_H

#ifdef EMMC_DEBUG
#define EMMC_DEBUG_LVL 4
#endif

const char* emmc_state_name[11];

bool emmc_init(void);
bool emmc_card_status(unsigned short rca);
bool emmc_card_select(unsigned short rca);
bool emmc_switch(unsigned char index, unsigned char value, unsigned char cmdset);
bool emmc_blocklen_set(unsigned int len);
bool emmc_blockcount_set(unsigned int count);
bool emmc_read_single_block(unsigned int iblock, unsigned char* buf);
bool emmc_write_single_block(unsigned int iblock, const unsigned char* buf);


#endif /* EMBLOCKS_EMMC_H */

