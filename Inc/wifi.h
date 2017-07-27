#ifndef _WIFI_H
#define _WIFI_H

#include "stm32f7xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "sdram.h"
#include "ov2640.h"

#include <stdlib.h>
#include <string.h>

#define WIFI_COMMAND_SIZE   25
#define RXBUFFERSIZE        3
#define RECEIVETIMEOUT      5000

void WIFI_Init(void);
void WIFI_SendData(uint8_t *dat);
void WIFI_ReceiveData(uint8_t *recvBuf);

static void WIFI_Error_Handler(void);

#endif
