#define __MODULE__ "emmc"

#include <emmc/emmc.h>
#include <emmc/hal/hal.h>
#include <emmc/hal/5m09cy301e13/maxv_hal.h>
#include <emmc/hal/5m09cy301e13/cyfxgpif2config.h>

#include <error/error.h>

#ifdef EMMC_DEBUG
#include <debug/debug.h>
#endif

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3error.h>
#include <cyu3gpio.h>
#include <cyu3uart.h>
#include <cyu3pib.h>
#include <cyu3gpif.h>


void emmc_delay_ms(unsigned int us);
bool emmc_line_init(void);
bool emmc_gpif_init(unsigned short speed);
bool emmc_gpif_deinit(void);
bool emmc_line_dq_setdir(bool binput);
void emmc_line_dq_set(bool val);
bool emmc_line_dq_get(void);
void emmc_line_rst_set(bool val);
void emmc_line_clk_set(bool val);
void emmc_line_dq_putb(bool bit);
bool emmc_line_dq_getb(void);


void emmc_delay_ms(unsigned int us)
{
	CyU3PThreadSleep(us);
}


#define EMMC_LINE_DQ   (37) 
#define EMMC_LINE_DONE (40)
#define EMMC_LINE_CLK  (14)
#define EMMC_LINE_RST  (38)

#define EMMC_LINE_D0   (0) 
#define EMMC_LINE_D1   (1)
#define EMMC_LINE_D2   (2)
#define EMMC_LINE_D3   (3)
#define EMMC_LINE_D4   (4)
#define EMMC_LINE_D5   (5)
#define EMMC_LINE_D6   (6)
#define EMMC_LINE_D7   (7)


bool emmc_hal_init(void)
{
    unsigned int timeout;

    if(!emmc_gpif_init(512))
    {
        error_msg_set("GPIF II initialization failed.\n");
        
        return false;
    }

    #ifdef EMMC_DEBUG
    debug_printf(EMMC_DEBUG_LVL, "GPIF II initialized.\n");
    #endif

	emmc_line_init();

	if(!emmc_send_cmd(0, 0x00000000, 0, (timeout=100,&timeout)))// reset to IDLE
    {
        error_msg_set("CMD0 sending error.\n");

        return false;
    }

	emmc_delay_ms(100);

	while(1)
	{
        unsigned char ocr[4];

		if(!emmc_send_cmd(1, 0x40FF8080, 0, (timeout=100,&timeout)))
        {
			error_msg_set("Receive error.\n");
		}
		else
        {
            emmc_receive_ocr(ocr);

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "CMD1 sended.\n");
            #endif

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC ocr: 0x%02x 0x%02x 0x%02x 0x%02x\n", ocr[3], ocr[2], ocr[1], ocr[0]);
            #endif

            if(ocr[3] & 0x80) break;
        }

		emmc_delay_ms(100);
	}

    emmc.ncards = 0;

    while(1)
    {
        unsigned char cid[16];

        if(!emmc_send_cmd( 2, 0x00000000, 0, (timeout=2000,&timeout) ))
        {
            error_msg_set("eMMC read CID error.\n");

            if(timeout == 0) break;

            goto w_continue;
        }

        emmc_receive_cid(cid);

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL,
            "eMMC CID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
            cid[15], cid[14], cid[13], cid[12], cid[11], cid[10], cid[ 9], cid[ 8],
            cid[ 7], cid[ 6], cid[ 5], cid[ 4], cid[ 3], cid[ 2], cid[ 1], cid[ 0]);
        #endif

        emmc_delay_ms(100);

        if(emmc_send_cmd(3, ((unsigned int)(emmc.ncards+1))<<16, 0, (timeout=1000,&timeout)))
        {
            emmc_receive_status();

            emmc.ncards++;

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL,
                         "eMMC set relative addr for card %d OK (status=%02x%02x%02x%02x)\n",
                         emmc.ncards, emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
            #endif
        }
        else
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read status error\n");
            #endif

            goto w_continue;
        }

        w_continue:

        emmc_delay_ms(100);
    }

    if(emmc.ncards == 0) return false;

    return true;
}

bool emmc_hal_deinit(void)
{
    unsigned int timeout;

	if(!emmc_send_cmd(0, 0x00000000, 0, (timeout=100,&timeout)))// reset to IDLE
    {
        error_msg_set("CMD0 sending error.\n");

        return false;
    }

    if(!emmc_gpif_deinit())
    {
        error_msg_set("GPIF II deinitialization failed.\n");
        
        return false;
    }

    return true;
}

bool emmc_line_init(void)
{
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CLK, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		error_msg_set("CyU3PDeviceGpioOverride failed, error code.\n");
        return false;
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_CLK, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		error_msg_set("CyU3PGpioSetSimpleConfig failed.\n");
        return false;
	}


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_DQ, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		error_msg_set("CyU3PDeviceGpioOverride failed.\n");
        return false;
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_DQ, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		error_msg_set("CyU3PGpioSetSimpleConfig failed.\n");
        return false;
	}

    CyU3PGpioSetIoMode(EMMC_LINE_DQ, CY_U3P_GPIO_IO_MODE_WPU);


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_DONE, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		error_msg_set("CyU3PDeviceGpioOverride failed.\n");
        return false;
	}

	gpioConfig.outValue = CyFalse;
	gpioConfig.inputEn = CyTrue;
	gpioConfig.driveLowEn = CyFalse;
	gpioConfig.driveHighEn = CyFalse;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_DONE, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		error_msg_set("CyU3PGpioSetSimpleConfig failed.\n");
        return false;
	}

    CyU3PGpioSetIoMode(EMMC_LINE_DONE, CY_U3P_GPIO_IO_MODE_WPD);

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_RST, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		error_msg_set("CyU3PDeviceGpioOverride failed.\n");
        return false;
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_RST, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		error_msg_set("CyU3PGpioSetSimpleConfig failed.\n");
        return false;
	}

    CyU3PGpioSetIoMode(EMMC_LINE_D0, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D1, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D2, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D3, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D4, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D5, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D6, CY_U3P_GPIO_IO_MODE_WPU);
    CyU3PGpioSetIoMode(EMMC_LINE_D7, CY_U3P_GPIO_IO_MODE_WPU);

    emmc_line_rst_set(0);

    return true;
}

bool emmc_gpif_init(unsigned short speed)
{
    CyU3PPibClock_t pibClock;
    CyU3PReturnStatus_t stat;

    if(speed > 512) speed = 512;

    pibClock.clkDiv      = 1024/speed;
    pibClock.clkSrc      = CY_U3P_SYS_CLK_BY_16;
    pibClock.isHalfDiv   = CyFalse;
    pibClock.isDllEnable = CyFalse;

    stat = CyU3PPibInit (CyTrue, &pibClock);

    if (stat != CY_U3P_SUCCESS)
    {
        error_msg_set("");
        return false;
    }

    stat = CyU3PGpifLoad ((CyU3PGpifConfig_t *)&CyFxGpifConfig);

    if (stat != CY_U3P_SUCCESS)
    {
        error_msg_set("");
        return false;
    }

    return true;
}

bool emmc_gpif_deinit(void)
{
    CyU3PGpifDisable(CyTrue);
    CyU3PPibDeInit();

    return true;
}

bool emmc_line_dq_setdir(bool binput)
{
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

    if(binput)
    {
        gpioConfig.outValue = CyFalse;
        gpioConfig.inputEn = CyTrue;
        gpioConfig.driveLowEn = CyFalse;
        gpioConfig.driveHighEn = CyFalse;
        gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
        apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_DQ, &gpioConfig);

        if (apiRetStatus != CY_U3P_SUCCESS)
        {
            error_msg_set("");
            return false;
        }
    }
    else
    {
        gpioConfig.outValue = CyTrue;
        gpioConfig.inputEn = CyFalse;
        gpioConfig.driveLowEn = CyTrue;
        gpioConfig.driveHighEn = CyTrue;
        gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
        apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_DQ, &gpioConfig);

        if (apiRetStatus != CY_U3P_SUCCESS)
        {
            error_msg_set("");
            return false;
        }
    }

    return true;
}

void emmc_line_dq_set(bool val)
{
	CyU3PGpioSetValue(EMMC_LINE_DQ, val != 0 ? CyTrue : CyFalse);
}

bool emmc_line_dq_get(void)
{
	CyBool_t val;

	CyU3PGpioGetValue(EMMC_LINE_DQ, &val);

	return val != CyFalse;
}

void emmc_line_clk_set(bool val)
{
	CyU3PGpioSetValue(EMMC_LINE_CLK, val != 0 ? CyTrue : CyFalse);
}

void emmc_line_rst_set(bool val)
{
	CyU3PGpioSetValue(EMMC_LINE_RST, val != 0 ? CyTrue : CyFalse);
}

bool emmc_line_done(void)
{
	CyBool_t val;

	CyU3PGpioGetValue(EMMC_LINE_DONE, &val);

	return val != CyFalse;
}


void emmc_line_dq_putb(bool bit)
{
	emmc_line_dq_set(bit);
	emmc_line_clk_set(1);
	emmc_line_clk_set(0);
}

bool emmc_line_dq_getb(void)
{
	bool bit;

	emmc_line_clk_set(0);
	emmc_line_clk_set(1);
	bit = emmc_line_dq_get();

	return bit;
}

bool emmc_send_cmd(unsigned char cmd, unsigned int arg, char dat_dir, unsigned int* timeout)
{
	int i;

    emmc_line_rst_set(0);
	emmc_line_dq_setdir(0);

    emmc_line_dq_putb(dat_dir);

	i = 5;

	do
	{
		emmc_line_dq_putb((cmd & 1<<i) != 0);
	} while(i-- > 0);

	i = 31;

	do
	{
		emmc_line_dq_putb((arg & 1UL<<i) != 0);
	} while(i-- > 0);

    emmc_line_rst_set(1);

    emmc_line_dq_putb(0);

    for(; !emmc_line_done() && (*timeout) > 0; (*timeout)--) emmc_delay_ms(1);

    emmc_line_dq_putb(0);
	emmc_line_dq_setdir(1);

    if(*timeout == 0) return false;

    emmc.lastcmd = cmd;

    return true;
}

bool emmc_receive_ocr(unsigned char* ocr)
{
    int i;
    bool b;

	emmc_line_dq_setdir(1);

    b = emmc_line_dq_getb();

    i = 32-1;

    do
    {
        b = emmc_line_dq_getb();

        if(b) ocr[i/8] |= (1<<(i%8));
        else ocr[i/8] &= ~(1<<(i%8));
    } while(i-- > 0);

    return b;
}

bool emmc_receive_cid(unsigned char* cid)
{
    bool b;
    int i;

	emmc_line_dq_setdir(1);

    b = emmc_line_dq_getb();

    i = 4*32-1-1;

    do
    {
        b = emmc_line_dq_getb();

        if(b) cid[i/8] |= (1UL<<(i%8));
        else cid[i/8] &= ~(1UL<<(i%8));
    } while(i-- > 0);

    return b;
}

bool emmc_receive_status(void)
{
    bool b;
    int i;
    unsigned char* status = (unsigned char*)&emmc.status;


    b = emmc_line_dq_getb();

    i = 32-1;

    do
    {
        b = emmc_line_dq_getb();

        if(b) status[i/8] |= (1<<(i%8));
        else status[i/8] &= ~(1<<(i%8));
    } while(i-- > 0);

    return b;
}

bool emmc_card_status(unsigned short rca)
{
    unsigned int timeout;

    if(rca == 0) rca = emmc.curcard;

    if(emmc_send_cmd(13, ((unsigned int)(rca))<<16, 0, (timeout=100,&timeout)))
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }
    else
    {
        error_msg_set("eMMC card read status error.\n");

        return false;
    }

    return true;
}

bool emmc_card_select(unsigned short rca)
{
    unsigned int timeout;

    if(emmc_send_cmd(7, ((unsigned int)(rca))<<16, 0, (timeout=100,&timeout)))
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d select error\n", rca);
        #endif

        return false;
    }

    emmc.curcard = rca;

    return true;
}

bool emmc_switch(unsigned char index, unsigned char value, unsigned char cmdset)
{
    unsigned int timeout;
    unsigned char cmd = 6;

    if(emmc_send_cmd(cmd, 3UL<<24 | ((unsigned int)index)<<16 | ((unsigned int)value)<<8 | (cmdset&7), 0, (timeout=100,&timeout)))
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);
        #endif

        return false;
    }

    return true;
}

bool emmc_blocklen_set(unsigned int len)
{
    unsigned int timeout;
    unsigned char cmd = 16;

    if(!emmc_send_cmd(cmd, len, 0, (timeout=100,&timeout)))
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);
        #endif

        return false;
    }
    else
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }

    emmc.blocklen = len;

    return true;
}

bool emmc_blockcount_set(unsigned int count)
{
    unsigned int timeout;
    unsigned char cmd = 23;

    if(emmc_send_cmd(cmd, count, 0, (timeout=100,&timeout)))
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);
        #endif

        return false;
    }

    emmc.blockcount = count;

    return true;
}

typedef void (*EMMCCallback_t) (unsigned char* buf, unsigned int count);

static CyU3PDmaBuffer_t _buffer __attribute__ ((aligned (32)));
static CyU3PDmaChannel eMMCDMAChHandle;
static EMMCCallback_t callback = 0;

static void emmc_hal_callback(CyU3PDmaChannel *handle, CyU3PDmaCbType_t cbType, CyU3PDmaCBInput_t *cbInput)
{
    if(callback) callback(cbInput->buffer_p.buffer, cbInput->buffer_p.count);
}

bool emmc_hal_read_start(EMMCCallback_t cb)
{
    CyU3PReturnStatus_t stat;
    CyU3PDmaChannelConfig_t dmaCfg;

    /* Create a DMA AUTO channel for the GPIF to USB transfer. */
    CyU3PMemSet ((uint8_t *)&dmaCfg, 0, sizeof (dmaCfg));

    dmaCfg.size           = 512;
    dmaCfg.count          = 1;
    dmaCfg.prodSckId      = CY_U3P_PIB_SOCKET_0;
    dmaCfg.consSckId      = CY_U3P_CPU_SOCKET_CONS;
    dmaCfg.prodAvailCount = 0;
    dmaCfg.prodHeader     = 0;
    dmaCfg.prodFooter     = 0;
    dmaCfg.consHeader     = 0;
    dmaCfg.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    dmaCfg.notification   = cb ? CY_U3P_DMA_CB_RECV_CPLT : 0;
    dmaCfg.cb             = emmc_hal_callback;
    callback = cb;

    /* Set DMA Channel transfer size */
    stat = CyU3PDmaChannelCreate (&eMMCDMAChHandle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaCfg);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL read_start DMA channel create failed\n");
        #endif

        return false;
    }

        /* Start the GPIF state machine off. */
    stat = CyU3PGpifSMStart (START, ALPHA_START);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL write_start Gpif start failed\n");
        #endif

        return false;
    }

    return true;
}

void emmc_hal_read_end(void)
{
    CyU3PDmaChannelDestroy(&eMMCDMAChHandle);

    CyU3PGpifDisable(CyFalse);
}

bool emmc_hal_read_commit(uint8_t* buf, uint32_t size, uint32_t count)
{
    CyU3PReturnStatus_t stat;

    _buffer.buffer = buf;
    _buffer.count = count;
    _buffer.size = size;
    _buffer.status = 0;

    stat = CyU3PDmaChannelSetupRecvBuffer(&eMMCDMAChHandle, &_buffer);// TODO: restrictions!!!

    if(stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL read_commit setuprecvbuffer failed.\n");
        #endif

        return false;
    }

    return true;
}

bool emmc_hal_write_start(EMMCCallback_t cb)
{
    CyU3PReturnStatus_t stat;
    CyU3PDmaChannelConfig_t dmaCfg;

    /* Create a DMA AUTO channel for the GPIF to USB transfer. */
    CyU3PMemSet ((uint8_t *)&dmaCfg, 0, sizeof (dmaCfg));

    dmaCfg.size           = 32;
    dmaCfg.count          = 0;
    dmaCfg.prodSckId      = CY_U3P_CPU_SOCKET_PROD;
    dmaCfg.consSckId      = CY_U3P_PIB_SOCKET_0;
    dmaCfg.prodAvailCount = 0;
    dmaCfg.prodHeader     = 0;
    dmaCfg.prodFooter     = 0;
    dmaCfg.consHeader     = 0;
    dmaCfg.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    dmaCfg.notification   = cb ? CY_U3P_DMA_CB_SEND_CPLT : 0;
    dmaCfg.cb             = emmc_hal_callback;
    callback = cb;

    /* Set DMA Channel transfer size */
    stat = CyU3PDmaChannelCreate (&eMMCDMAChHandle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaCfg);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL write_start DMA channel create failed\n");
        #endif

        return false;
    }

        /* Start the GPIF state machine off. */
    stat = CyU3PGpifSMStart (START2, ALPHA_START2);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL write_start Gpif start failed\n");
        #endif

        emmc_hal_write_end();

        return false;
    }

    return true;
}

void emmc_hal_write_end(void)
{
    CyU3PDmaChannelDestroy(&eMMCDMAChHandle);

    CyU3PGpifDisable(CyFalse);
}

static CyU3PDmaBuffer_t _buffer __attribute__ ((aligned (32)));

void emmc_hal_write_commit(uint8_t* buf, uint32_t size, uint32_t count)
{
    CyU3PReturnStatus_t stat;

    _buffer.buffer = buf;
    _buffer.count = count;
    _buffer.size = size;
    _buffer.status = 0;

    led_set(1, 1);
    stat = CyU3PDmaChannelSetupSendBuffer(&eMMCDMAChHandle, &_buffer);// TODO: restrictions!!!

    if(stat != CY_U3P_SUCCESS)
    {
        debug_printf(0, "Write commit Err\n");
    }
}

bool emmc_hal_trans_wait_complete(uint32_t wait)
{
    CyU3PReturnStatus_t stat;

    stat = CyU3PDmaChannelWaitForCompletion(&eMMCDMAChHandle, wait);
    led_set(1, 0);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(0, "wait timeout\n");
        #endif

        if (stat != CY_U3P_ERROR_TIMEOUT) {
            error_critical("Undefined error on WaitForCompletion", 0);
        }

        return false;
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(0, "wait OK\n");
        #endif
    }

    return true;
}

bool emmc_read_single_block(unsigned int iblock, unsigned char* buf)
{
    bool res;
    unsigned int timeout;
    unsigned char cmd = 17;

    if(!emmc_hal_read_start(0))
    {
        error_msg("!emmc_hal_read_start\n", 0);

        res = false;
        goto emmc_read_single_block_cleanup;
    }

    if(!emmc_hal_read_commit(buf, 1024, 1+512+16+1))
    {
        error_msg("!emmc_hal_read_commit\n", 0);

        res = false;
        goto emmc_read_single_block_cleanup;
    }

    if(!emmc_send_cmd(cmd, iblock, 0, (timeout=100,&timeout)))
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);
        #endif
        error_msg("!emmc_send_cmd\n", 0);

        res = false;
        goto emmc_read_single_block_cleanup;
    }
    else
    {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }

    res = emmc_hal_trans_wait_complete(1000);
    
    if(!res) {
        error_msg("!emmc_hal_trans_wait_complete\n", 0);
    }

emmc_read_single_block_cleanup:
    emmc_hal_read_end();

    return res;
}

static unsigned char crc[16];
static unsigned char _buf[1024] __attribute__ ((aligned (32)));

#define _bit(d, n) ((d>>n)&1)
#define _setbit(d, n, value) (d = d&(~(1UL<<n)) | (value&1 ? 1UL<<n : 0))


void crc16_8x(const unsigned char *dt, unsigned int count, unsigned char* crc)
{
    unsigned int i;

    for(i = 0; i < 16; i++) crc[i] = 0;

    for(i = 0; i < count; i++)
    {
        unsigned char in = dt[i] ^ crc[15]; 

        crc[15] = crc[14];
        crc[14] = crc[13];
        crc[13] = crc[12];
        crc[12] = crc[11] ^ in;
        crc[11] = crc[10];
        crc[10] = crc[ 9];
        crc[ 9] = crc[ 8];
        crc[ 8] = crc[ 7];
        crc[ 7] = crc[ 6];
        crc[ 6] = crc[ 5];
        crc[ 5] = crc[ 4] ^ in;
        crc[ 4] = crc[ 3];
        crc[ 3] = crc[ 2];
        crc[ 2] = crc[ 1];
        crc[ 1] = crc[ 0];
        crc[ 0] = in;
    }
}

bool emmc_tran_busy(void)
{
    CyBool_t val;

    CyU3PGpioGetValue(0, &val);

    return val == CyFalse;
}

bool emmc_write_single_block(unsigned int iblock, const unsigned char* buf)
{
    int i;
    bool res;
    unsigned int timeout;
    unsigned char cmd = 24;

    if (!emmc_hal_write_start(NULL)) {
        error_msg("!emmc_hal_write_start\n", 0);
        return false;
    }

    _buf[0] = 0x00;// start byte
    for (i = 0; i < 512; i++) _buf[1+i] = buf[i];

    crc16_8x(_buf+1, 512, crc);

    _buf[1 + 512 + 0]  = crc[15];
    _buf[1 + 512 + 1]  = crc[14];
    _buf[1 + 512 + 2]  = crc[13];
    _buf[1 + 512 + 3]  = crc[12];
    _buf[1 + 512 + 4]  = crc[11];
    _buf[1 + 512 + 5]  = crc[10];
    _buf[1 + 512 + 6]  = crc[ 9];
    _buf[1 + 512 + 7]  = crc[ 8];
    _buf[1 + 512 + 8]  = crc[ 7];
    _buf[1 + 512 + 9]  = crc[ 6];
    _buf[1 + 512 + 10] = crc[ 5];
    _buf[1 + 512 + 11] = crc[ 4];
    _buf[1 + 512 + 12] = crc[ 3];
    _buf[1 + 512 + 13] = crc[ 2];
    _buf[1 + 512 + 14] = crc[ 1];
    _buf[1 + 512 + 15] = crc[ 0];
    _buf[1 + 512 + 16] = 0xFF;// stop byte

    if (!emmc_send_cmd(cmd, iblock, 0, (timeout=10,&timeout)))// Write single block
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);
        #endif
        error_msg("eMMC[%d]: cmd %d error\n", emmc.curcard, cmd);

        res = false;
        goto emmc_write_single_block_cleanup;
    } else {
        emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0]);
        #endif
    }

    emmc_hal_write_commit(_buf, 1024, 1+512+16+1);

    res = emmc_hal_trans_wait_complete(1000);

    if (!res) {
        error_msg("!write:emmc_hal_trans_wait_complete\n", 0);
        goto emmc_write_single_block_cleanup;
    }

    #ifdef EMMC_DEBUG
    debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: write OK\n", emmc.curcard);
    #endif

emmc_write_single_block_cleanup:
    emmc_hal_write_end();

    return res;
}

