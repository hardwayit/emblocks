#include <emmc/emmc.h>
#include <emmc/hal/hal.h>
#include <emmc/hal/5m09cy301e13/maxv_hal.h>
#include <emmc/hal/5m09cy301e13/cyfxgpif2config.h>

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
void emmc_line_init(void);
bool emmc_gpif_init(void);
void emmc_line_dq_setdir(char binput);
void emmc_line_dq_set(char val);
char emmc_line_dq_get(void);
void emmc_line_rst_set(char val);
void emmc_line_clk_set(char val);
void emmc_line_dq_putb(char bit);
char emmc_line_dq_getb(void);


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

    if(!emmc_gpif_init())
    {
        error_msg_set("GPIF II initialization failed.\n");
        
        return false;
    }

    #ifdef EMMC_DEBUG
    debug_printf(EMMC_DEBUG_LVL, "GPIF II initialized.\n");
    #endif

	emmc_line_init();

	if(!emmc_send_cmd(0, 0x00000000, 0))// reset to IDLE
    {
        error_msg_set("CMD0 sending error.\n");

        return false;
    }

	emmc_delay_ms(100);

	while(1)
	{
        unsigned char ocr[4];

		if(!emmc_send_cmd(1, 0x40FF8080, 0))
        {
			error_msg_set("Receive error.\n");
		}
		else
        {
            res = emmc_receive_ocr(ocr);

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "CMD1 sended.\n");
            #endif

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC ocr: 0x%02x 0x%02x 0x%02x 0x%02x [%d]\n", ocr[3], ocr[2], ocr[1], ocr[0], res);
            #endif

            if(ocr[3] & 0x80) break;
        }

		emmc_delay_ms(100);
	}

    emmc.ncards = 0;

    while(1)
    {
        unsigned char cid[16];

        if(!emmc_send_cmd(2, 0x00000000, 0, (timeout=2000,&timeout)))
        {
            error_msg_set("eMMC read CID error.\n");

            if(timeout == 0) break;

            goto w_continue;
        }

        res = emmc_receive_cid(cid);

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL,
            "eMMC CID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x [%d]\n",
            cid[15], cid[14], cid[13], cid[12], cid[11], cid[10], cid[ 9], cid[ 8],
            cid[ 7], cid[ 6], cid[ 5], cid[ 4], cid[ 3], cid[ 2], cid[ 1], cid[ 0], res);
        #endif

        emmc_delay_ms(100);

        res = emmc_send_cmd(3, ((unsigned int)(emmc.ncards+1))<<16, 0);

        if(res)
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read status error [%d]\n", res);
            #endif

            goto w_continue;
        }
        else
        {
            res = emmc_receive_status();

            emmc.ncards++;

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL,
                         "eMMC set relative addr for card %d OK (status=%02x%02x%02x%02x) [%d]\n",
                         emmc.ncards, emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
            #endif
        }

        w_continue:

        emmc_delay_ms(100);
    }

    if(emmc.ncards == 0) return false;

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

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "%d", bit);
    // #endif
}

bool emmc_line_dq_getb(void)
{
	bool bit;

	emmc_line_clk_set(0);
	emmc_line_clk_set(1);
	bit = emmc_line_dq_get();

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "[%d]\n", bit);
    // #endif

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

    for(; !emmc_line_done() && *timeout > 0; *timeout--) emmc_delay_ms(1);

    emmc_line_dq_putb(0);
	emmc_line_dq_setdir(1);

    if(*timeout == 0) return false;

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
    if(emmc_send_cmd(13, ((unsigned int)(rca))<<16, 0))
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x [%d]\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
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
    if(emmc_send_cmd(7, ((unsigned int)(rca))<<16, 0))
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x [%d]\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d select error [%d]\n", rca, res);
        #endif

        return false;
    }

    emmc.curcard = rca;

    return true;
}

char emmc_switch(unsigned char index, unsigned char value, unsigned char cmdset)
{
    char res;
    unsigned char cmd = 6;

    res = emmc_send_cmd(cmd, 3UL<<24 | ((unsigned int)index)<<16 | ((unsigned int)value)<<8 | (cmdset&7), 0);

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error [%d]\n", emmc.curcard, cmd, res);
        #endif

        return 1;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x [%d]\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    return 0;
}

char emmc_blocklen_set(unsigned int len)
{
    char res;
    unsigned char cmd = 16;

    res = emmc_send_cmd(cmd, len, 0);

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error [%d]\n", emmc.curcard, cmd, res);
        #endif

        return 1;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x [%d]\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    emmc.blocklen = len;

    return 0;
}

char emmc_blockcount_set(unsigned int count)
{
    char res;
    unsigned char cmd = 23;

    res = emmc_send_cmd(cmd, count, 0);

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error [%d]\n", emmc.curcard, cmd, res);
        #endif

        return 1;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x [%d]\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    emmc.blockcount = count;

    return 0;
}

typedef void (*EMMCCallback_t) (unsigned char* buf, unsigned int count);

static CyU3PDmaBuffer_t _buffer __attribute__ ((aligned (32)));
static CyU3PDmaChannel eMMCDMAChHandle;
static EMMCCallback_t callback = 0;

static void emmc_hal_callback(CyU3PDmaChannel *handle, CyU3PDmaCbType_t cbType, CyU3PDmaCBInput_t *cbInput)
{
    if(callback) callback(cbInput->buffer_p.buffer, cbInput->buffer_p.count);
}

char emmc_hal_read_start(EMMCCallback_t cb)
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

        return 1;
    }

        /* Start the GPIF state machine off. */
    stat = CyU3PGpifSMStart (START, ALPHA_START);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL write_start Gpif start failed\n");
        #endif

        return 2;
    }

    return 0;
}

void emmc_hal_read_end(void)
{
    CyU3PDmaChannelDestroy(&eMMCDMAChHandle);

    CyU3PGpifDisable(CyFalse);
}

char emmc_hal_read_commit(uint8_t* buf, uint32_t size, uint32_t count)
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

        return 1;
    }

    return 0;
}

char emmc_hal_write_start(EMMCCallback_t cb)
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

        return 1;
    }

        /* Start the GPIF state machine off. */
    stat = CyU3PGpifSMStart (START2, ALPHA_START2);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC HAL write_start Gpif start failed\n");
        #endif

        return 2;
    }

    return 0;
}

void emmc_hal_write_end(void)
{
    CyU3PDmaChannelDestroy(&eMMCDMAChHandle);

    CyU3PGpifDisable(CyFalse);
}

static CyU3PDmaBuffer_t _buffer __attribute__ ((aligned (4096)));

void emmc_hal_write_commit(uint8_t* buf, uint32_t size, uint32_t count)
{
    CyU3PReturnStatus_t stat;

    _buffer.buffer = buf;
    _buffer.count = count;
    _buffer.size = size;
    _buffer.status = 0;

    stat = CyU3PDmaChannelSetupSendBuffer(&eMMCDMAChHandle, &_buffer);// TODO: restrictions!!!
    led_set(1, 1);

    if(stat != CY_U3P_SUCCESS)
    {
        debug_printf(0, "Write commit Err\n");
    }
}

void emmc_hal_trans_wait_complete(uint32_t wait)
{
    CyU3PReturnStatus_t stat;

    stat = CyU3PDmaChannelWaitForCompletion(&eMMCDMAChHandle, wait);
    led_set(1, 0);

    if (stat != CY_U3P_SUCCESS)
    {
        #ifdef EMMC_DEBUG
        debug_printf(0, "wait timeout\n");
        #endif
    }
    else
    {
        #ifdef EMMC_DEBUG
        debug_printf(0, "wait OK\n");
        #endif
    }
}

char emmc_read_single_block(unsigned int iblock, unsigned char* buf)
{
    char res;
    unsigned char cmd = 17;

    res = emmc_hal_read_start(0);

    if(res)
    {
        return 1;
    }

    res = emmc_hal_read_commit(buf, 1024, 1+512+16+1);

    if(res)
    {
        return 2;
    }

    res = emmc_send_cmd(cmd, iblock*512, 0); 

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error [%d]\n", emmc.curcard, cmd, res);
        #endif

        return 3;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x [%d]\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    emmc_hal_trans_wait_complete(1000);
    
    emmc_hal_read_end();

    return 0;
}

static unsigned char crc[16];
static unsigned char _buf[1024] __attribute__ ((aligned (4096)));

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

char emmc_write_single_block(unsigned int iblock, const unsigned char* buf)
{
    int i;
    char res;
    unsigned char cmd = 24;

    res = emmc_hal_write_start(NULL);

    if(res)
    {
        return 1;
    }

    _buf[0] = 0x00;// start byte
    for(i = 0; i < 512; i++) _buf[1+i] = buf[i];

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

    res = emmc_send_cmd(cmd, iblock*512, 0);// Write single block 

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d error [%d]\n", emmc.curcard, cmd, res);
        #endif

        return 1;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: cmd %d OK status=%02x%02x%02x%02x [%d]\n", emmc.curcard, cmd,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    emmc_hal_write_commit(_buf, 1024, 1+512+16+1);

    emmc_hal_trans_wait_complete(1000);

    emmc_hal_write_end();

    #ifdef EMMC_DEBUG
    debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: write OK\n", emmc.curcard);
    #endif

    emmc_delay_ms(1);

    for(i = 0; i < 2000; i++) {
        emmc_card_status(1);
        if(emmc.status.fields.current_state == 4) break;

        emmc_delay_ms(1);
    }

    if(i == 2000) {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC[%d]: write timeout.\n", emmc.curcard);
        #endif
        
        return 3;
    }

    return 0;
}

