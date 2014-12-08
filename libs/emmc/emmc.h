/*
 * emmc.h
 *
 *  Created on: 21.11.2014
 *      Author: ALev
 */

#ifndef EMMC_H_
#define EMMC_H_

#ifdef EMMC_DEBUG
#define EMMC_DEBUG_LVL 4
#endif

const char* emmc_state_name[11];

bool emmc_init(void);
char emmc_card_status(unsigned short rca);
char emmc_card_select(unsigned short rca);
char emmc_switch(unsigned char index, unsigned char value, unsigned char cmdset);
char emmc_blocklen_set(unsigned int len);
char emmc_blockcount_set(unsigned int count);
char emmc_read_single_block(unsigned int iblock, unsigned char* buf);
char emmc_write_single_block(unsigned int iblock, const unsigned char* buf);


#endif /* EMMC_H_ */
