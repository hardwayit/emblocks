#include <emmc/emmc.h>
#include <emmc/hal/hal.h>
#include <emmc/hal/5m09cy301e13/maxv_hal.h>

#ifdef EMMC_DEBUG
#include <debug/debug.h>
#endif

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3error.h>
#include <cyu3gpio.h>
#include <cyu3uart.h>


void emmc_delay_ms(unsigned int us);
void emmc_line_init(void);
void emmc_line_dq_setdir(char binput);
void emmc_line_dq_set(char val);
char emmc_line_dq_get(void);
void emmc_line_rst_set(char val);
void emmc_line_clk_set(char val);
void emmc_line_outclk_set(char val);
void emmc_line_dq_putb(char bit);
char emmc_line_dq_getb(void);

extern void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t apiRetStatus    /* API return status */
        );


void emmc_delay_ms(unsigned int us)
{
	CyU3PThreadSleep(us);
}


#define EMMC_LINE_DQ     (37) 
#define EMMC_LINE_DONE   (40)
#define EMMC_LINE_CLK    (14)
#define EMMC_LINE_OUTCLK (16)
#define EMMC_LINE_RST    (38)

#define EMMC_LINE_D0 (0) 
#define EMMC_LINE_D1 (1)
#define EMMC_LINE_D2 (2)
#define EMMC_LINE_D3 (3)
#define EMMC_LINE_D4 (4)
#define EMMC_LINE_D5 (5)
#define EMMC_LINE_D6 (6)
#define EMMC_LINE_D7 (7)


bool emmc_hal_init(void)
{
    char res;

    // This code led to error. Why??
	emmc_line_init();

	//res = emmc_send_cmd(0, 0x00000000, 0);// reset to IDLE
    //emmc_line_outclk_set(1);
    //emmc_line_rst_set(0);
    //emmc_line_dq_setdir(0);
    //emmc_line_dq_putb(0);
    //emmc_line_dq_putb(0);
    //emmc_line_dq_putb(0);

    //if(res)
    //{
    //    #ifdef EMMC_DEBUG
    //    debug_printf(EMMC_DEBUG_LVL, "CMD0 sending error.\r\n");
    //    #endif

    //    while(1);
    //}
    //else
    //{
    //    #ifdef EMMC_DEBUG
    //    debug_printf(EMMC_DEBUG_LVL, "CMD0 sended.\r\n");
    //    #endif
    //}

	//emmc_delay_ms(1000);

	while(1)
	{
        unsigned char ocr[4];

		res = emmc_send_cmd(1, 0x40FF8080, 0);

		if(res) {
            #ifdef EMMC_DEBUG
			debug_printf(EMMC_DEBUG_LVL, "Receive error [%d]\r\n", res);
            #endif
		}
		else {
            res = emmc_receive_ocr(ocr);

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "CMD1 sended.\r\n");
            #endif

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC ocr: 0x%02x 0x%02x 0x%02x 0x%02x [%d]\r\n", ocr[3], ocr[2], ocr[1], ocr[0], res);
            #endif

            if(ocr[3] & 0x80) break;
        }

		emmc_delay_ms(1000);
	}

    emmc.ncards = 0;

    while(1)
    {
        unsigned char cid[16];

        res = emmc_send_cmd(2, 0x00000000, 0);

        if(res)
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read CID error [%d]\r\n", res);
            #endif

            if(res == 1 || res == 2) break;

            goto w_continue;
        }
        else
        {
            res = emmc_receive_cid(cid);

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC CID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x [%d]\r\n",
                cid[15], cid[14], cid[13], cid[12], cid[11], cid[10], cid[ 9], cid[ 8],
                cid[ 7], cid[ 6], cid[ 5], cid[ 4], cid[ 3], cid[ 2], cid[ 1], cid[ 0], res);
            #endif
        }

        emmc_delay_ms(100);

        res = emmc_send_cmd(3, ((unsigned int)(emmc.ncards+1))<<16, 0);

        if(res)
        {
            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC read status error [%d]\r\n", res);
            #endif

            goto w_continue;
        }
        else
        {
            res = emmc_receive_status();

            emmc.ncards++;

            #ifdef EMMC_DEBUG
            debug_printf(EMMC_DEBUG_LVL, "eMMC set relative addr for card %d OK (status=%02x%02x%02x%02x) [%d]\r\n", emmc.ncards,
                emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
            #endif
        }

        w_continue:

        emmc_delay_ms(1000);
    }

    if(emmc.ncards == 0) return false;

    return true;
}

void emmc_line_init(void)
{
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CLK, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		debug_printf (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
        while(1);
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
		debug_printf (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
				EMMC_LINE_CLK, apiRetStatus);
        while(1);
	}


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_OUTCLK, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		debug_printf (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
        while(1);
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_OUTCLK, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		debug_printf (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
				EMMC_LINE_CLK, apiRetStatus);
        while(1);
	}


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_DQ, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		debug_printf (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
        while(1);
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
		debug_printf (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
				EMMC_LINE_DQ, apiRetStatus);
        while(1);
	}

    CyU3PGpioSetIoMode(EMMC_LINE_DQ, CY_U3P_GPIO_IO_MODE_WPU);


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_DONE, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		debug_printf (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
        while(1);
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
		debug_printf (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
				EMMC_LINE_DONE, apiRetStatus);
        while(1);
	}

    CyU3PGpioSetIoMode(EMMC_LINE_DONE, CY_U3P_GPIO_IO_MODE_WPD);

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_RST, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		debug_printf (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
        while(1);
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
		debug_printf (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
				EMMC_LINE_RST, apiRetStatus);
        while(1);
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
    emmc_line_outclk_set(0);
}

void emmc_line_dq_setdir(char binput)
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
            /* Error handling */
            CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
                    EMMC_LINE_DQ, apiRetStatus);
            CyFxAppErrorHandler(apiRetStatus);
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
            /* Error handling */
            CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig(%d) failed, error code = %d\n",
                    EMMC_LINE_DQ, apiRetStatus);
            CyFxAppErrorHandler(apiRetStatus);
        }
    }
}

void emmc_line_dq_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_DQ, val != 0 ? CyTrue : CyFalse);
}

char emmc_line_dq_get(void)
{
	CyBool_t val;

	CyU3PGpioGetValue(EMMC_LINE_DQ, &val);

	return val != CyFalse;
}

void emmc_line_clk_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_CLK, val != 0 ? CyTrue : CyFalse);
}

void emmc_line_outclk_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_OUTCLK, val != 0 ? CyTrue : CyFalse);
}

void emmc_line_rst_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_RST, val != 0 ? CyTrue : CyFalse);
}

char emmc_line_done(void)
{
	CyBool_t val;

	CyU3PGpioGetValue(EMMC_LINE_DONE, &val);

	return val != CyFalse;
}


void emmc_line_dq_putb(char bit)
{
	emmc_line_dq_set(bit);
	emmc_line_clk_set(1);
	emmc_line_clk_set(0);

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "%d", bit);
    // #endif
}

char emmc_line_dq_getb(void)
{
	char bit;

	emmc_line_clk_set(0);
	emmc_line_clk_set(1);
	bit = emmc_line_dq_get();

    // #ifdef EMMC_DEBUG
	// debug_printf(EMMC_DEBUG_LVL, "[%d]\r\n", bit);
    // #endif

	return bit;
}

char emmc_send_cmd(unsigned char cmd, unsigned int arg, char dat_dir)
{
	int i;
    unsigned int n = 0;

    emmc_line_rst_set(0);
	emmc_line_dq_setdir(0);

    emmc_line_outclk_set(1);
    emmc_line_outclk_set(0);

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

    for(n = 1000; !emmc_line_done() && n > 0; n--) {
        emmc_line_outclk_set(1);
        emmc_line_outclk_set(0);
    }

    emmc_line_dq_putb(0);
	emmc_line_dq_setdir(1);

    if(n == 0) {
        #ifdef EMMC_DEBUG
        debug_printf(0, "emmc_send_cmd(%d): sending cmd timeout!\r\n", cmd);
        #endif

        return 1;
    }

    return 0;
}

char emmc_receive_ocr(unsigned char* ocr)
{
    int i;
    char b;

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

char emmc_receive_cid(unsigned char* cid)
{
    int i;
    char b;

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

char emmc_receive_status(void)
{
    int i;
    char b;
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

char emmc_card_status(unsigned short rca)
{
    char res;

    res = emmc_send_cmd(13, ((unsigned int)(rca))<<16, 0);

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d read status error [%d]\r\n", rca, res);
        #endif
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x [%d]\r\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    return res == 0;
}

char emmc_card_select(unsigned short rca)
{
    char res;

    res = emmc_send_cmd(7, ((unsigned int)(rca))<<16, 0);

    if(res)
    {
        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d select error [%d]\r\n", rca, res);
        #endif

        return 1;
    }
    else
    {
        res = emmc_receive_status();

        #ifdef EMMC_DEBUG
        debug_printf(EMMC_DEBUG_LVL, "eMMC card #%d status=%02x%02x%02x%02x [%d]\r\n", rca,
            emmc.status.b[3], emmc.status.b[2], emmc.status.b[1], emmc.status.b[0], res);
        #endif
    }

    return 0;
}

