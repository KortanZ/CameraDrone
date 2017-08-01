/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Drone_Reserved_0_Pin GPIO_PIN_0
#define Drone_Reserved_0_GPIO_Port GPIOC
#define Drone_Reserved_1_Pin GPIO_PIN_1
#define Drone_Reserved_1_GPIO_Port GPIOC
#define Drone_Reserved_2_Pin GPIO_PIN_2
#define Drone_Reserved_2_GPIO_Port GPIOC
#define Drone_Reserved_3_Pin GPIO_PIN_3
#define Drone_Reserved_3_GPIO_Port GPIOC
#define Board_Reserved_Pin GPIO_PIN_0
#define Board_Reserved_GPIO_Port GPIOB
#define Board_ReservedB1_Pin GPIO_PIN_1
#define Board_ReservedB1_GPIO_Port GPIOB
#define Board_ReservedB2_Pin GPIO_PIN_2
#define Board_ReservedB2_GPIO_Port GPIOB
#define Drone_Forward_Pin GPIO_PIN_12
#define Drone_Forward_GPIO_Port GPIOB
#define Drone_Backward_Pin GPIO_PIN_13
#define Drone_Backward_GPIO_Port GPIOB
#define Drone_Left_Pin GPIO_PIN_14
#define Drone_Left_GPIO_Port GPIOB
#define Drone_Right_Pin GPIO_PIN_15
#define Drone_Right_GPIO_Port GPIOB
#define Board_ReservedC8_Pin GPIO_PIN_8
#define Board_ReservedC8_GPIO_Port GPIOC
#define Board_ReservedC9_Pin GPIO_PIN_9
#define Board_ReservedC9_GPIO_Port GPIOC
#define Drone_Flag_0_Pin GPIO_PIN_4
#define Drone_Flag_0_GPIO_Port GPIOD
#define Drone_Flag_1_Pin GPIO_PIN_5
#define Drone_Flag_1_GPIO_Port GPIOD
#define Drone_Spin_0_Pin GPIO_PIN_6
#define Drone_Spin_0_GPIO_Port GPIOD
#define Drone_Spin_1_Pin GPIO_PIN_7
#define Drone_Spin_1_GPIO_Port GPIOD
#define Board_ReservedB3_Pin GPIO_PIN_3
#define Board_ReservedB3_GPIO_Port GPIOB
#define Board_ReservedB4_Pin GPIO_PIN_4
#define Board_ReservedB4_GPIO_Port GPIOB
#define Board_ReservedB5_Pin GPIO_PIN_5
#define Board_ReservedB5_GPIO_Port GPIOB
#define Board_ReservedB6_Pin GPIO_PIN_6
#define Board_ReservedB6_GPIO_Port GPIOB
#define Board_ReservedB7_Pin GPIO_PIN_7
#define Board_ReservedB7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
