#include "emmc/emmc.h"
#include "emmc/hal/hal.h"
#include "leds/leds.h"
#include "crc/crc7.h"

#ifdef EMMC_DEBUG
#include "debug/debug.h"

#define EMMC_DEBUG_LVL 4
#endif

#define EMMC_T_NID 5
#define EMMC_T_NCR_MIN 2
#define EMMC_T_NCR_MAX 64


static const char* state_names[] = {
    "idle",
    "ready",
    "ident",
    "stby",
    "tran",
    "data",
    "rcv",
    "prg",
    "dis",
    "btst",
    "slp"
};


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

static void emmc_cmdframe_init(struct EMMCCMDFrame* frame)
{
	frame->startb = 0;
	frame->transb = 1;
	// frame->cmdi = 0x00;
	// frame->arg = 0x00;
	// frame->crc = 0x00;
	frame->endb = 1;
}


static void emmc_line_cmd_putb(char bit)
{
	emmc_line_clk_set(1);
	emmc_delay_us(1);
	emmc_line_clk_set(0);
	emmc_line_cmd_set(bit);
	emmc_delay_us(1);

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "%d", bit);
    // #endif
}

static char emmc_line_cmd_getb(void)
{
	char bit;

	emmc_line_clk_set(1);
	emmc_delay_us(1);
	emmc_line_clk_set(0);
	bit = emmc_line_cmd_get();
	emmc_delay_us(1);

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "[%d]\r\n", bit);
    // #endif

	return bit;
}

void emmc_receive_error(void)
{
    int i;

    for(i = 0; i < 160; i++) emmc_line_cmd_getb();
}

void emmc_session_start(void)
{
	emmc_line_clk_set(0);
	emmc_delay_us(1);
}

void emmc_session_stop(void)
{
	emmc_line_clk_set(1);
}

void emmc_send_cmd(unsigned char cmd, unsigned int arg)
{
	int i;
	struct EMMCCMDFrame frame;
	uns7 crc;

	emmc_cmdframe_init(&frame);

	frame.cmdi = cmd;
    for(i = 0; i < 32; i++) {
        if(arg & 1UL<<i) frame.arg[i/8] |= 1<<(i%8);
        else frame.arg[i/8] &= ~(1<<(i%8));
    }

	emmc_line_cmd_setdir(0);

	crc.u = 0;
	emmc_line_cmd_putb(frame.startb);
	crc7_pushb(&crc, frame.startb);
	emmc_line_cmd_putb(frame.transb);
	crc7_pushb(&crc, frame.transb);

	i = 5;

	do
	{
		char b = (frame.cmdi & 1<<i) != 0;
		emmc_line_cmd_putb(b);
		crc7_pushb(&crc, b);
	} while(i-- > 0);

	i = 31;

	do
	{
		char b = (frame.arg[i/8] & 1<<(i%8)) != 0;
		emmc_line_cmd_putb(b);
		crc7_pushb(&crc, b);
	} while(i-- > 0);

	frame.crc = crc.u;

	i = 6;

	do
	{
		char b = (frame.crc & 1<<i) != 0;
		emmc_line_cmd_putb(b);
	} while(i-- > 0);

	emmc_line_cmd_putb(frame.endb);

	emmc_line_cmd_setdir(1);

	emmc_delay_us(5);
}

static struct EMMC
{
    unsigned char ncards;

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

char emmc_receive_ocr(unsigned char* ocr)
{
	int i;
	struct EMMCRespR3Frame frame;
	unsigned char* bytes = (unsigned char*)&frame;

	for(i = 0; i < EMMC_T_NID; i++)
    {
        if(!emmc_line_cmd_getb()) {
            // Bus error
            emmc_receive_error();
            return 1;
        }
    }

    i = sizeof(struct EMMCRespR3Frame)*8-1;

    do	
	{
		if(emmc_line_cmd_getb()) bytes[i/8] |= 1<<(i%8);
		else bytes[i/8] &= ~(1<<(i%8));
	}
    while(i-- > 0);

    if(frame.startb || frame.transb) {
        emmc_receive_error();
        return 2;
    }

    if(frame.chk1 != 0x3F || frame.chk2 != 0x7F) {
        emmc_receive_error();
        return 3;
    }

	memcpy(ocr, frame.ocr, sizeof(frame.ocr));

	return 0;
}

char emmc_receive_cid(unsigned char* cid)
{
	int i;
	struct EMMCRespR2Frame frame;
	unsigned char* bytes = (unsigned char*)&frame;

	for(i = 0; i < EMMC_T_NID; i++)
    {
        if(!emmc_line_cmd_getb()) {
            // Bus error
            emmc_receive_error();
            return 1;
        }
    }

    i = sizeof(struct EMMCRespR2Frame)*8-1;

    do	
	{
		if(emmc_line_cmd_getb()) bytes[i/8] |= 1<<(i%8);
		else bytes[i/8] &= ~(1<<(i%8));
	}
    while(i-- > 0);

    if(frame.startb || frame.transb) {
        emmc_receive_error();
        return 2;
    }

    if(frame.chk1 != 0x3F) {
        emmc_receive_error();
        return 3;
    }

	memcpy(cid, frame.arg, sizeof(frame.arg));

	return 0;
}

char emmc_receive_status(void)
{
	int i;
	struct EMMCRespR1Frame frame;
	unsigned char* bytes = (unsigned char*)&frame;

	for(i = 0; i < EMMC_T_NCR_MIN; i++)
    {
        if(!emmc_line_cmd_getb()) {
            // Bus error
            emmc_receive_error();
            return 1;
        }
    }

    bytes[0] |= 3;

    for(i = EMMC_T_NCR_MIN; i < EMMC_T_NCR_MAX; i++)
    {
        bytes[0] = (bytes[0]<<1)&3;
        bytes[0] |= (emmc_line_cmd_getb() != 0);

        if((bytes[0]&3) == 0) break;
    }

    if(i == EMMC_T_NCR_MAX)
    {
        emmc_receive_error();

        return 2;
    }

    frame.startb = frame.transb = 0;

    i = sizeof(struct EMMCRespR1Frame)*8-1-2;

    do
	{
		if(emmc_line_cmd_getb()) bytes[i/8] |= 1<<(i%8);
		else bytes[i/8] &= ~(1<<(i%8));
	}
    while(i-- > 0);

    /*TODO: if(frame.crc != crc) {
        emmc_receive_error();
        return 4;
    }*/

	memcpy(&emmc.status, frame.status, sizeof(frame.status));

	return 0;
}

char emmc_card_status(unsigned short rca)
{
    char res;

    emmc_session_start();
    emmc_send_cmd(13, ((unsigned int)(rca))<<16);
    res = emmc_receive_status();
    emmc_session_stop();

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d read status error [%d]\r\n", rca, res);
        #endif

        return 1;
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x\r\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }

    return 0;
}

char emmc_card_select(unsigned short rca)
{
    char res;

    emmc_session_start();
    emmc_send_cmd(7, ((unsigned int)(rca))<<16);
    res = emmc_receive_status();
    emmc_session_stop();

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d select error [%d]\r\n", rca, res);
        #endif

        return 1;
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x\r\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }

    return 0;
}

void emmc_init(void)
{
    int i;

	emmc_line_init();

    emmc_session_start();
	emmc_send_cmd(0, 0x00000000);// reset to IDLE
    emmc_session_stop();

    #ifdef EMMC_DEBUG
	debug_printf(EMMC_DEBUG_LVL, "CMD0 sended.\r\n");
    #endif

	emmc_delay_us(100);

	while(1)
	{
        char res;
        unsigned char ocr[4];

        emmc_session_start();
		emmc_send_cmd(1, 0x40FF8080);

        #ifdef EMMC_DEBUG
		debug_printf(EMMC_DEBUG_LVL, "CMD1 sended.\r\n");
        #endif

		if((res = emmc_receive_ocr(ocr))) {
            #ifdef EMMC_DEBUG
			debug_printf(EMMC_DEBUG_LVL, "Receive error [%d]\r\n", res);
            #endif
		}
		else {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC ocr: 0x%02x 0x%02x 0x%02x 0x%02x\r\n", ocr[3], ocr[2], ocr[1], ocr[0]);
            #endif
        }

        emmc_session_stop();

        if(ocr[3] & 0x80) break;

		emmc_delay_us(100);
	}

    emmc.ncards = 0;

    while(1)
    {
        char res;
        unsigned char cid[15];

        emmc_session_start();
        emmc_send_cmd(2, 0x00000000);
        res = emmc_receive_cid(cid);
        emmc_session_stop();

        if(res)
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read CID error [%d]\r\n", res);
            #endif

            if(res == 1 || res == 2) break;

            continue;
        }
        else
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC CID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
                cid[14], cid[13], cid[12], cid[11], cid[10], cid[ 9], cid[ 8], cid[ 7],
                cid[ 6], cid[ 5], cid[ 4], cid[ 3], cid[ 2], cid[ 1], cid[ 0]);
            #endif
        }

        emmc_session_start();
        emmc_send_cmd(3, ((unsigned int)(emmc.ncards+1))<<16);
        res = emmc_receive_status();
        emmc_session_stop();

        if(res)
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read status error [%d]\r\n", res);
            #endif

            continue;
        }
        else
        {
            emmc.ncards++;

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC set relative addr for card %d OK (status=%02x%02x%02x%02x)\r\n", emmc.ncards,
                emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
            #endif
        }
    }

    for(i = 1; i <= emmc.ncards; i++)
    {
        if(emmc_card_status(i)) continue;

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "Card %d current state: %s\r\n", i, state_names[emmc.status.fields.current_state]);
        #endif
    }
}
