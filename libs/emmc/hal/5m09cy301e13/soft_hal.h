#ifndef EMMC_SOFT_HAL_H
#define EMMC_SOFT_HAL_H


__attribute__ ((aligned (1)))
struct EMMCCMDFrame
{
	unsigned char startb : 1;
	unsigned char transb : 1;
	unsigned char cmdi : 6;
	unsigned char arg[4];
	unsigned char crc : 7;
	unsigned char endb : 1;
};

__attribute__ ((aligned (1)))
struct EMMCRespR3Frame
{
	unsigned char endb : 1;
	unsigned char chk2 : 7;
	unsigned char ocr[4];
	unsigned char chk1 : 6;
	unsigned char transb : 1;
	unsigned char startb : 1;
};

__attribute__ ((aligned (1)))
struct EMMCRespR2Frame
{
	unsigned char endb : 1;
	unsigned char crc : 7;
	unsigned char arg[15];
	unsigned char chk1 : 6;
	unsigned char transb : 1;
	unsigned char startb : 1;
};

__attribute__ ((aligned (1)))
struct EMMCRespR1Frame
{
	unsigned char endb : 1;
	unsigned char crc : 7;
	unsigned char status[4];
	unsigned char cmd : 6;
	unsigned char transb : 1;
	unsigned char startb : 1;
};


/*
 * eMMC Software HAL interface:
 */

void emmc_delay_us(unsigned int us);
void emmc_line_init(void);
void emmc_line_cmd_setdir(char binput);
void emmc_line_cmd_set(char val);
char emmc_line_cmd_get(void);
void emmc_line_clk_set(char val);

void emmc_session_start(void);
void emmc_session_stop(void);
void emmc_send_cmd(unsigned char cmd, unsigned int arg);
char emmc_receive_ocr(unsigned char* ocr);
char emmc_receive_cid(unsigned char* cid);
char emmc_receive_status(void);

/* -------------------------- */

#endif

