#include <debug/hal/hal.h>

#include <stdarg.h>
#include <cyu3system.h>
#include <cyu3os.h>
#include <cyu3error.h>
#include <cyu3uart.h>

bool debug_hal_init(void)
{
    CyU3PUartConfig_t uartConfig;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    /* Initialize and configure the UART for logging. */
    status = CyU3PUartInit ();
    if (status != CY_U3P_SUCCESS)
    {
        return false;
    }

    CyU3PMemSet ((uint8_t *)&uartConfig, 0, sizeof (uartConfig));
    uartConfig.baudRate = CY_U3P_UART_BAUDRATE_115200;
    uartConfig.stopBit  = CY_U3P_UART_ONE_STOP_BIT;
    uartConfig.parity   = CY_U3P_UART_NO_PARITY;
    uartConfig.txEnable = CyTrue;
    uartConfig.rxEnable = CyFalse;
    uartConfig.flowCtrl = CyFalse;
    uartConfig.isDma    = CyTrue;
    status = CyU3PUartSetConfig (&uartConfig, NULL);
    if (status != CY_U3P_SUCCESS)
    {
        return false;
    }

    /* Set the dma for an inifinity transfer */
    status = CyU3PUartTxSetBlockXfer (0xFFFFFFFF);
    if (status != CY_U3P_SUCCESS)
    {
        return false;
    }

    /* Start the debug module for printing log messages. */
    status = CyU3PDebugInit (CY_U3P_LPP_SOCKET_UART_CONS, 8);
    if (status != CY_U3P_SUCCESS)
    {
        return false;
    }

    CyU3PDebugPreamble(CyFalse);

    return true;
}


void debug_vprintf(int priority, const char* format, va_list args)
{
    CyU3PDebugPrint(priority, (char*)format);
}

