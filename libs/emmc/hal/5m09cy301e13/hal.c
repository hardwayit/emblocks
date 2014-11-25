#include "emmc/hal/5m09cy301e13/hal.h"

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3error.h>
#include <cyu3gpio.h>
#include <cyu3uart.h>


extern void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t apiRetStatus    /* API return status */
        );


void emmc_delay_us(unsigned int us)
{
	CyU3PThreadSleep(us);
}

#define EMMC_LINE_CMD_W 0
#define EMMC_LINE_CMD_R 1
#define EMMC_LINE_CMD_DIR 2
#define EMMC_LINE_CMD_DIR_OUT 1
#define EMMC_LINE_CMD_DIR_IN 0
#define EMMC_LINE_CLK 16

void emmc_line_init(void)
{
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CMD_W, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		CyU3PDebugPrint (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_CMD_W, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CMD_R, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		CyU3PDebugPrint (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	gpioConfig.outValue = CyFalse;
	gpioConfig.inputEn = CyTrue;
	gpioConfig.driveLowEn = CyFalse;
	gpioConfig.driveHighEn = CyFalse;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_CMD_R, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}


	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CMD_DIR, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		CyU3PDebugPrint (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(EMMC_LINE_CMD_DIR, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	apiRetStatus = CyU3PDeviceGpioOverride (EMMC_LINE_CLK, CyTrue);
	if (apiRetStatus != 0)
	{
		/* Error Handling */
		CyU3PDebugPrint (4, "CyU3PDeviceGpioOverride failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
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
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}
}

void emmc_line_cmd_setdir(char binput)
{
	CyU3PGpioSetValue(EMMC_LINE_CMD_DIR, binput == 0 ? EMMC_LINE_CMD_DIR_OUT : EMMC_LINE_CMD_DIR_IN);
}

void emmc_line_cmd_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_CMD_W, val != 0 ? CyTrue : CyFalse);
}

char emmc_line_cmd_get(void)
{
	CyBool_t val;

	CyU3PGpioGetValue(EMMC_LINE_CMD_R, &val);

	return val != CyFalse;
}

void emmc_line_clk_set(char val)
{
	CyU3PGpioSetValue(EMMC_LINE_CLK, val != 0 ? CyTrue : CyFalse);
}

