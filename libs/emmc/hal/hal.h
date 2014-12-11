#ifndef EMMC_HAL_H
#define EMMC_HAL_H


#define EMMC_T_NID 5
#define EMMC_T_NCR_MIN 2
#define EMMC_T_NCR_MAX 64


extern struct EMMC
{
    unsigned char ncards;
    unsigned short curcard;
    unsigned int blocklen;
    unsigned int blockcount;
    unsigned char lastcmd;

	union
    {
        struct {
            char tst_rsrvd:2;
            char app_spec_cmds:2;
            char rsrvd1:1;
            char app_cmd:1;
            char rsrvd2:1;
            char switch_error:1;
            char ready_for_data:1;
            char current_state:4;
            char erase_reset:1;
            char rsrvd3:1;
            char wp_erase_skip:1;
            char cid_csd_overwrite:1;
            char overrun:1;
            char underrun:1;
            char error:1;
            char cc_error:1;
            char card_ecc_failed:1;
            char illegal_command:1;
            char com_crc_error:1;
            char lock_unlock_failed:1;
            char card_is_locked:1;
            char wp_violation:1;
            char erase_param:1;
            char erase_seq_error:1;
            char block_len_error:1;
            char address_misalign:1;
            char address_out_of_range:1;
        } fields;

        unsigned char b[4];
    } status;
} emmc;


bool emmc_hal_init(void);

#endif

