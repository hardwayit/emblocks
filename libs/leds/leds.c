#include "leds/leds.h"

#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3error.h>
#include <cyu3gpio.h>

#define LED_RED_GPIO 38
#define LED_GREEN_GPIO 39

extern void
CyFxAppErrorHandler (
        CyU3PReturnStatus_t apiRetStatus    /* API return status */
        );


void led_set(unsigned char led, char value)
{
	uint32_t portnum;

    switch(led)
    {
    case LED_RED: portnum = LED_RED_GPIO; break;
    case LED_GREEN: portnum = LED_GREEN_GPIO; break;
    default:
        return;
    }

    switch(value)
    {
    case 0:
        CyU3PGpioSetValue(portnum, value);
        break;
    case 1:
        CyU3PGpioSetValue(portnum, value);
        break;
    }

}

void led_init(void)
{
	CyU3PGpioSimpleConfig_t gpioConfig;
    CyU3PReturnStatus_t apiRetStatus = CY_U3P_SUCCESS;

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(LED_RED_GPIO, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	gpioConfig.outValue = CyTrue;
	gpioConfig.inputEn = CyFalse;
	gpioConfig.driveLowEn = CyTrue;
	gpioConfig.driveHighEn = CyTrue;
	gpioConfig.intrMode = CY_U3P_GPIO_NO_INTR;
	apiRetStatus = CyU3PGpioSetSimpleConfig(LED_GREEN_GPIO, &gpioConfig);
	if (apiRetStatus != CY_U3P_SUCCESS)
	{
		/* Error handling */
		CyU3PDebugPrint (4, "CyU3PGpioSetSimpleConfig failed, error code = %d\n",
				apiRetStatus);
		CyFxAppErrorHandler(apiRetStatus);
	}

	led_set(LED_RED, 0);
	led_set(LED_GREEN, 0);
}
