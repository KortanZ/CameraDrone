/**
  ******************************************************************************
  * @file    stm32746g_discovery_sdram.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    30-December-2016
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32746g_discovery_sdram.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SDRAM_H
#define _SDRAM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "fmc.h"

/** 
  * @brief  SDRAM status structure definition  
  */     
#define   SDRAM_OK         ((uint8_t)0x00)
#define   SDRAM_ERROR      ((uint8_t)0x01)

/** @defgroup STM32746G_DISCOVERY_SDRAM_Exported_Constants STM32746G_DISCOVERY_SDRAM Exported Constants
  * @{
  */ 
#define SDRAM_DEVICE_ADDR  ((uint32_t)0xD0000000)
#define SDRAM_DEVICE_SIZE  ((uint32_t)0x800000)  /* SDRAM device size in MBytes */


//#define REFRESH_COUNT                    ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz SD clock) */
#define REFRESH_COUNT                    ((uint32_t)1292)
   
#define SDRAM_TIMEOUT                    ((uint32_t)0xFFFF)

  
/**
  * @brief  FMC SDRAM Mode definition register defines
  */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200) 


void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount);
void *SDRAM_Malloc(uint32_t size);


#endif /*_SDRAM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
