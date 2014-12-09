#ifndef EMMC_SOFT_HAL_H
#define EMMC_SOFT_HAL_H


/*
 * Software eMMC Software HAL interface:
 */

bool emmc_send_cmd(unsigned char cmd, unsigned int arg, char dat_dir, unsigned int* timeout);
bool emmc_receive_ocr(unsigned char* ocr);
bool emmc_receive_cid(unsigned char* cid);
bool emmc_receive_status(void);

/* -------------------------- */

#endif

