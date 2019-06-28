/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stdlib.h"
#include "TLS3001.h"
#include "stdbool.h"
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
uint32_t uwTimerPeriod;
uint32_t aCCValue;

uint32_t *OCxM_DMA_buffer;
volatile bool TLS3001_PWM_transfer_finish = false;

#define TLS3001_RESET_LEN 19
#define TLS3001_SYNC_LEN 30
#define TLS3001_START_LEN 19

const uint16_t TLS3001_RESET [TLS3001_RESET_LEN] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0};						// RESET = 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0,  1,0,0
const uint16_t TLS3001_SYNC [TLS3001_SYNC_LEN] = {1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0};	// SYNC_ = 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0,  0,0,1,0, 0,0,0,0, 0,0,0,0, 0,0
const uint16_t TLS3001_START [TLS3001_START_LEN] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0};						// START = 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,1,0


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void user_init_DMA_buffer(uint32_t **DMA_buffer, uint16_t *data_in_buffer, uint32_t data_len);
void user_deinit_DMA_buffer(uint32_t *DMA_buffer);
void user_PWM_start(TIM_HandleTypeDef *htim, uint32_t Channel);
void TLS3001_send_packet(uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);

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
	const uint32_t data_in_len = 10;
	uint16_t data_in_buffer [10] = {1, 1, 0, 0, 1, 1, 1, 0, 0, 0};
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  uint32_t PCLK1_freq=HAL_RCC_GetPCLK1Freq();
  uwTimerPeriod = (uint32_t)(((PCLK1_freq*2) / TLS3001_CLK) - 1);
  /* Compute CCR2 value to generate a duty cycle at 50% */
  aCCValue = (uint32_t)(((uint32_t) 50 * (uwTimerPeriod - 1)) / 100);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  // Send reset
  user_init_DMA_buffer(&OCxM_DMA_buffer, &data_in_buffer[0], data_in_len);
  TLS3001_send_packet(OCxM_DMA_buffer, &htim3.Instance->CCMR1, data_in_len);

  while(TLS3001_PWM_transfer_finish == false){
  }

  HAL_Delay(1);	//1ms delay.

  // Send Synch
  user_init_DMA_buffer(&OCxM_DMA_buffer, &TLS3001_SYNC[0], TLS3001_SYNC_LEN);
  TLS3001_send_packet(OCxM_DMA_buffer, &htim3.Instance->CCMR1, TLS3001_SYNC_LEN);

  while(TLS3001_PWM_transfer_finish == false){
  }

  HAL_Delay(50);	//50ms delay.



  /*
  // Enable the DMA stream
  if (HAL_DMA_Start_IT(&hdma_tim3_ch1_trig, OCxM_DMA_buffer, &htim3.Instance->CCMR1, data_in_len) != HAL_OK)
  {
	 return HAL_ERROR;
  }
  // Enable the TIM3 Update event DMA request
  __HAL_TIM_ENABLE_DMA(&htim3, TIM_DMA_CC1);
  HAL_TIM_GenerateEvent(&htim3, TIM_EGR_CC1G);

  HAL_Delay(10);

  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  user_PWM_start(&htim3, TIM_CHANNEL_1);
*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*
	  if((htim3.Instance->CR1 & TIM_CR1_CEN) == 0)
	  {
		  user_PWM_start(&htim3, TIM_CHANNEL_1);
	  }


	  if(TLS3001_PWM_transfer_finish == true)
	  {

		  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);	//This is not good since we stop the PWM (disabling the output) too soon.
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
		//uint16_t temp = (~TIM_CCMR1_OC1M_Msk | TIM_OCMODE_FORCED_INACTIVE);
		//TIM3->CCMR1 &= temp;
		  TLS3001_PWM_transfer_finish = false;
	  }
	  */

	  HAL_Delay(10);

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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void TLS3001_send_packet(uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
	  // Enable the DMA stream
	  if (HAL_DMA_Start_IT(&hdma_tim3_ch1_trig, SrcAddress, DstAddress, DataLength) != HAL_OK)
	  {
		 return HAL_ERROR;
	  }
	  // Enable the TIM3 Update event DMA request
	  __HAL_TIM_ENABLE_DMA(&htim3, TIM_DMA_CC1);
	  HAL_TIM_GenerateEvent(&htim3, TIM_EGR_CC1G);

	  HAL_Delay(10);

	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	  user_PWM_start(&htim3, TIM_CHANNEL_1);

	  TLS3001_PWM_transfer_finish = false;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{

	// Stop the counter
	//htim->Instance->CR1 &= ~(TIM_CR1_CEN);
	//


	//Alternative to above
	//Wait for the period to finish prior to disabling the
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	while(TIM3->CNT < TIM3->ARR){
	}

	uint16_t temp = (~TIM_CCMR1_OC1M_Msk | TIM_OCMODE_FORCED_INACTIVE);
	TIM3->CCMR1 &= temp;
	HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);

	TLS3001_PWM_transfer_finish = true;


}


void user_init_DMA_buffer(uint32_t **DMA_buffer, uint16_t *data_in_buffer, uint32_t data_len)
{
	  uint32_t tim_3_ccmr1_manch_0 = 0;
	  uint32_t tim_3_ccmr1_manch_1 = 0;

	  *DMA_buffer = (uint32_t *)calloc(data_len, sizeof(uint32_t));

	  /* Get the TIMx CCMR2 register value */
	  uint16_t tmpccmr1 = TIM3->CCMR1;
	  tim_3_ccmr1_manch_1 = (tmpccmr1 | TIM_OCMODE_PWM1);
	  tim_3_ccmr1_manch_0 = (tmpccmr1 | TIM_OCMODE_PWM2);

	  for (int i = 0; i < data_len; i++)
	  {
		  if(*(data_in_buffer+i) == 1)		//Manchester '1'
		  {
			  *(*DMA_buffer+i) = tim_3_ccmr1_manch_1;
		  }
		  else								//Manchester '0'
		  {
			  *(*DMA_buffer+i) = tim_3_ccmr1_manch_0;
		  }

	  }
}

// Function for faster start of PWM (had some delay issues with first pulse)
void user_PWM_start(TIM_HandleTypeDef *htim, uint32_t Channel)
{
	  /* Enable the Capture compare channel */
	  uint32_t tmp = TIM_CCER_CC1E << (Channel & 0x1FU); /* 0x1FU = 31 bits max shift */
	  /* Reset the CCxE Bit */
	  TIM3->CCER &= ~tmp;
	  /* Set or reset the CCxE Bit */
	  TIM3->CCER |= (uint32_t)(TIM_CCx_ENABLE << (Channel & 0x1FU)); /* 0x1FU = 31 bits max shift */

	  __HAL_TIM_ENABLE(htim);
}

void user_deinit_DMA_buffer(uint32_t *DMA_buffer)
{
	free(DMA_buffer);
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
