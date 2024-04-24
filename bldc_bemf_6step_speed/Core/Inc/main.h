/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOE
#define KEY1_Pin GPIO_PIN_8
#define KEY1_GPIO_Port GPIOI
#define KEY2_Pin GPIO_PIN_13
#define KEY2_GPIO_Port GPIOC
#define KEY3_Pin GPIO_PIN_10
#define KEY3_GPIO_Port GPIOI
#define KEY4_Pin GPIO_PIN_11
#define KEY4_GPIO_Port GPIOI
#define W_BEMF_Pin GPIO_PIN_3
#define W_BEMF_GPIO_Port GPIOH
#define V_BEMF_Pin GPIO_PIN_4
#define V_BEMF_GPIO_Port GPIOH
#define U_BEMF_Pin GPIO_PIN_5
#define U_BEMF_GPIO_Port GPIOH
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOB
#define U_LOW_Pin GPIO_PIN_13
#define U_LOW_GPIO_Port GPIOB
#define V_LOW_Pin GPIO_PIN_14
#define V_LOW_GPIO_Port GPIOB
#define W_LOW_Pin GPIO_PIN_15
#define W_LOW_GPIO_Port GPIOB
#define U_HIGH_Pin GPIO_PIN_8
#define U_HIGH_GPIO_Port GPIOA
#define V_HIGH_Pin GPIO_PIN_9
#define V_HIGH_GPIO_Port GPIOA
#define W_HIGH_Pin GPIO_PIN_10
#define W_HIGH_GPIO_Port GPIOA
#define MOTOR_ENABLE_Pin GPIO_PIN_8
#define MOTOR_ENABLE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
