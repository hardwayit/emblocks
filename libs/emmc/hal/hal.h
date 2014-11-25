#ifndef EMMC_HAL_H
#define EMMC_HAL_H

/*
 * Software eMMC HAL interface:
 */

void emmc_delay_us(unsigned int us);
void emmc_line_init(void);
void emmc_line_cmd_setdir(char binput);
void emmc_line_cmd_set(char val);
char emmc_line_cmd_get(void);
void emmc_line_clk_set(char val);

/* -------------------------- */

#endif

