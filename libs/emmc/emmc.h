/*
 * emmc.h
 *
 *  Created on: 21.11.2014
 *      Author: ALev
 */

#ifndef EMMC_H_
#define EMMC_H_

void emmc_init(void);

void emmc_send_cmd(unsigned char cmd, unsigned int arg);

void emmc_get_status(unsigned int *status);

#endif /* EMMC_H_ */
