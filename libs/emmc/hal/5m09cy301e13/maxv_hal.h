#ifndef EMMC_SOFT_HAL_H
#define EMMC_SOFT_HAL_H


/*
 * Software eMMC Software HAL interface:
 */

char emmc_send_cmd(unsigned char cmd, unsigned int arg, char dat_dir);
char emmc_receive_ocr(unsigned char* ocr);
char emmc_receive_cid(unsigned char* cid);
char emmc_receive_status(void);

/* -------------------------- */

#endif

