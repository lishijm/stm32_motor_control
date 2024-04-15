/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal_rcc.h"
#include "stdio.h"
#include "bsp_motor.h"
#include "bsp_hall.h"
#include "protocol_niming_upper.h"
#include "protocol_uart_sei.h"
#include "algorithm_pid.h"
#include "algorithm_filtering.h"
#include "math.h"
#include "debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
MotorSta_Typedef global_motorsta;
MotorDir_Typedef global_motordir;
FOLPF_HandleTypeDef global_speed_hz;
PID_LOC_HandleTypedef motor_speed_pid;
uint32_t global_pwm_duty=0;
uint8_t global_state=0x00;
uint8_t uart_rx_buffer[UART_BUFFER_LEN];
float32_t motor_control_val=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
#if MPU_ENABLE
static void MPU_Config(void);
#endif
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
#if MPU_ENABLE
  MPU_Config();
#endif
  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Delay(200);//wait for powersupply stability
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
  // __HAL_DBGMCU_FREEZE_TIM1();
  // __HAL_DBGMCU_FREEZE_TIM3();
  HALLSENSOR_TIMxStart(&htim3);
  global_motorsta=MOTOR_STA_DISABLE;
  FLOAT_FirstOrderLowPassFiltering_DataInit(&global_speed_hz,SPEED_HZ_FILTERING_ALPHA);
  PID_LOC_Init(&motor_speed_pid,round(0./60.*PPR),0.75f,0.45f,0.f);
  
  __HAL_UART_ENABLE_IT(&huart4,UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart4,(uint8_t *)uart_rx_buffer,UART_BUFFER_LEN);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  MOTOR_Start(&htim1,&htim3);
  while (1)
  {
    Protocol_UARTxRXProcess();

    if(GPIO_PIN_RESET==HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){
      MOTOR_ENABLE();
      global_motorsta=MOTOR_STA_ENABLE;
    }
    
    if(GPIO_PIN_RESET==HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){

#if BREAKING_INERTIA
      MOTOR_Breaking_Inertia();
#endif
#if BREAKING_LOWBRIDGE
      MOTOR_Breaking_LowBridge(&htim1);
#endif

      /*speed_duty or control_val must be zero when motor enable*/
      /********if not motor will shake while motor runing********/
      motor_control_val=0;
      MOTOR_SpeedControl(&htim1,round(motor_control_val));

      global_speed_hz.current_val = 0;
      global_speed_hz.last_val    = 0;
      motor_speed_pid.control_val = 0;
      motor_speed_pid.err         = 0;
      motor_speed_pid.err_last    = 0;
      motor_speed_pid.integral    = 0;
    }
    
    if(global_state&0x01){
      global_speed_hz.current_val=HALLSENSOR_SpeedFrequency_Hz();
      FLOAT_FirstOrderLowPassFiltering_Process(&global_speed_hz);
      if(0==global_speed_hz.current_val){
        uint8_t hall_phase=0;
        hall_phase=HALLSENSOR_GetPhase();
        
#if BREAKING_INERTIA
        /*aim to phase when motor enable*/
        global_pwm_duty=30;
#endif
        
        MOTOR_Control(&htim1,hall_phase);
        HAL_TIM_GenerateEvent(&htim1,TIM_EVENTSOURCE_COM);
      }
      motor_control_val=PID_LOC_Process(&motor_speed_pid,global_speed_hz.current_val);
      MOTOR_SpeedControl(&htim1,round(motor_control_val));
      global_state&=~0x01;
    }
    
    if(global_state&0x02){
      if(MOTOR_STA_DISABLE==global_motorsta){
        global_speed_hz.current_val=HALLSENSOR_SpeedFrequency_Hz();
        FLOAT_FirstOrderLowPassFiltering_Process(&global_speed_hz);
      }
      Protocol_NIMING_Mortor(&huart4,0xF1,global_speed_hz.current_val,global_speed_hz.current_val/PPR,(global_speed_hz.current_val/PPR)*60);
      global_state&=~0x02;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */
#if MPU_ENABLE
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF);

}
#endif
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
