#include "wifi.h"

const uint8_t echoOff[] = "ATE0\r\n";
const uint8_t uartconf[] = "AT+UART_CUR=3456000,8,1,0,0\r\n";
const uint8_t para1[] = "AT+CWMODE_CUR=2\r\n";
const uint8_t para2[] = "AT+CIPAP_CUR=\"192.168.1.1\",\"192.168.1.1\",\"255.255.255.0\"\r\n";
const uint8_t para3[] = "AT+CIPMUX=1\r\n";
const uint8_t para4[] = "AT+CIPSERVER=1,1001\r\n";
const uint8_t SendCommandTemplate[] = "AT+CIPSEND=0,";
const uint8_t CRLF[] = "\r\n";

uint8_t *datBuff;

void WIFI_Init(void)
{
    if (HAL_UART_Transmit(&huart1, (uint8_t *)echoOff, sizeof(echoOff) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, (uint8_t *)para1, sizeof(para1) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, (uint8_t *)para2, sizeof(para2) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, (uint8_t *)para3, sizeof(para3) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, (uint8_t *)para4, sizeof(para4) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, (uint8_t *)uartconf, sizeof(uartconf) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    WIFI_Delay(1000);
    UART_SetBaud();

    datBuff = (uint8_t *)SDRAM_Malloc(OV2640_IMG_WIDTH + 3);
}

uint8_t *WIFI_GetSendCommand(uint16_t length)
{
    uint8_t datLengthTmp[5];
    uint8_t datLength[5];
    uint8_t numTmp;
    uint8_t i = 0, j = 0;
    uint8_t *cmd = (uint8_t *)malloc(WIFI_COMMAND_SIZE);

    if (!cmd)
    {
        WIFI_Error_Handler();

        return NULL;
    }
    else
    {
        while (length)
        {
            numTmp = length % 10;
            datLengthTmp[j++] = numTmp + '0';
            length /= 10;
        }
        while (j)
        {
            j--;
            datLength[i++] = datLengthTmp[j];
        }
        datLength[i] = 0x00;
        strcpy((char *)cmd, (char *)SendCommandTemplate);
        strcat((char *)cmd, (char *)datLength);
        strcat((char *)cmd, (char *)CRLF);

        return cmd;
    }
}

void WIFI_SendData(uint8_t *dat)
{
    uint8_t *SendCommand = WIFI_GetSendCommand(strlen((char *)dat));
    if (HAL_UART_Transmit(&huart1, SendCommand, strlen((char *)SendCommand), 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
    free(SendCommand);
    strcpy((char *)datBuff, (char *)dat);
    strcat((char *)datBuff, (char *)CRLF);
    WIFI_Delay(1000);
    if (HAL_UART_Transmit(&huart1, datBuff, strlen((char *)datBuff), 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
}

void WIFI_ReceiveData(uint8_t *recvBuf)
{
    WIFI_Delay(100);
    while (HAL_UART_Receive(&huart1, (uint8_t *)recvBuf, RXBUFFERSIZE - 1, RECEIVETIMEOUT) != HAL_OK)
        ;

    /**Drop head info */
    // for (int i = 0; i < RXBUFFERSIZE; ++i)
    // {
    //     if (recvBuf[i] == ':')
    //     {
    //         recvBuf
    //     }
    // }
}

void WIFI_Error_Handler(void)
{
    while (1)
    {
        ;
    }
}

void WIFI_Delay(uint32_t cnt)
{
    uint32_t i, j;
    for (i = 0; i < cnt; i++)
    {
        for (j = 0; j < 200; j++)
        {
            ;
        }
    }
}
