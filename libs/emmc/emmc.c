#include "emmc/emmc.h"
#include "emmc/hal/hal.h"
#include "leds/leds.h"
#include "crc/crc7.h"

#ifdef EMMC_DEBUG
#include "debug/debug.h"

#define EMMC_DEBUG_LVL 4
#endif

#define EMMC_NID 5


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
	unsigned char arg[4];
	unsigned char chk1 : 6;
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
	unsigned char status[4];
} emmc;

char emmc_receive_status(void)
{
	int i;
	struct EMMCRespR3Frame frame;
	unsigned char* bytes = (unsigned char*)&frame;

	for(i = 0; i < EMMC_NID; i++)
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

	memcpy(emmc.status, frame.arg, 4);

	return 0;
}

void emmc_init(void)
{
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

        emmc_session_start();
		emmc_send_cmd(1, 0x40FF8080);

        #ifdef EMMC_DEBUG
		debug_printf(EMMC_DEBUG_LVL, "CMD1 sended.\r\n");
        #endif

		if((res = emmc_receive_status())) {
            #ifdef EMMC_DEBUG
			debug_printf(EMMC_DEBUG_LVL, "Receive error [%d]\r\n", res);
            #endif
		}
		else {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC status: 0x%02x 0x%02x 0x%02x 0x%02x\r\n", emmc.status[3], emmc.status[2], emmc.status[1], emmc.status[0]);
            #endif
        }

        emmc_session_stop();

		emmc_delay_us(1000);
	}
}
