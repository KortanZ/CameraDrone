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
#define RXBUFFERSIZE        20

extern uint8_t recv[RXBUFFERSIZE];

extern void WIFI_Init(void);
extern void WIFI_SendData(uint8_t *dat);
extern void WIFI_ReceiveData(uint8_t *recvBuf, uint16_t length);
extern void WIFI_Delay(uint32_t cnt);
extern void WIFI_CheckOK(void);

extern void WIFI_Transparent_Init(void);
extern void WIFI_Transparent_SendData(uint8_t *dat, uint16_t size);

static void WIFI_Error_Handler(void);


#endif
