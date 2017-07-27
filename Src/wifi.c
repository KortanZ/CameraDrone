#include "wifi.h"

uint8_t echoOff[] = "ATE0\r\n";
uint8_t para1[] = "AT+CWMODE_DEF=2\r\n";
uint8_t para2[] = "AT+CIPAP_DEF=\"192.168.1.1\",\"192.168.1.1\",\"255.255.255.0\"\r\n";
uint8_t para3[] = "AT+CIPMUX=1\r\n";
uint8_t para4[] = "AT+CIPSERVER=1,1001\r\n";
uint8_t SendCommandTemplate[] = "AT+CIPSEND=0,";
uint8_t CRLF[] = "\r\n";

uint8_t *datBuff;

void WIFI_Init(void)
{
    if (HAL_UART_Transmit(&huart1, echoOff, sizeof(echoOff) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
	HAL_Delay(1);
    if (HAL_UART_Transmit(&huart1, para1, sizeof(para1) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
	HAL_Delay(1);
    if (HAL_UART_Transmit(&huart1, para2, sizeof(para2) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
	HAL_Delay(1);
    if (HAL_UART_Transmit(&huart1, para3, sizeof(para3) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
	HAL_Delay(1);
    if (HAL_UART_Transmit(&huart1, para4, sizeof(para4) - 1, 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
	
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
            datLength[i++] = datLengthTmp[--j];
        }
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
    HAL_Delay(1);
    *datBuff = 0x00;
    strcat((char *)datBuff, (char *)dat);
    strcat((char *)datBuff, (char *)CRLF);
    if (HAL_UART_Transmit(&huart1, datBuff, strlen((char *)datBuff), 0xffffffff) != HAL_OK)
    {
        WIFI_Error_Handler();
    }
}

void WIFI_ReceiveData(uint8_t *recvBuf)
{
    HAL_Delay(100);
    while (HAL_UART_Receive(&huart1, (uint8_t *)recvBuf, RXBUFFERSIZE - 1, RECEIVETIMEOUT) != HAL_OK);

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
