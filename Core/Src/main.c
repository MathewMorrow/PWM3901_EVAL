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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "micros.h"
#include "pmw3901.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

uint8_t sprintfBuffer[512] = {'\0'};
uint8_t runPMW = 0;
uint8_t streamPMW = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
void printCLI(void);
void streamData();

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

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  microsInit(&htim5);

  UART_Receive_IT_Enable(&huart2);

  PMW3901_init(&hspi1, PIN_CS_GPIO_Port, PIN_CS_Pin, PIN_INTERRUPT_GPIO_Port, PIN_INTERRUPT_Pin);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	if(runPMW && PMW3901_IsDataReady())
	{
		PMW3901_ReadMotion();
//		PMW3901_ReadMotionBulk();
	}

	if(getRxCount(&huart2)) printCLI();
	streamData();

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 15;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 84-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PIN_CS_GPIO_Port, PIN_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PIN_INTERRUPT_Pin */
  GPIO_InitStruct.Pin = PIN_INTERRUPT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PIN_INTERRUPT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PIN_CS_Pin */
  GPIO_InitStruct.Pin = PIN_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PIN_CS_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void printCLI(void)
{
	char read_Value = usart_Read(&huart2);
	printf("%c\n\r", read_Value);

	if(read_Value == 'h')
	{
		sprintf(sprintfBuffer,"\n\r"
				"1 - Init\n\r"
				"2 - Set motion\n\r"
				"3 - Slow Read\n\r"
				"4 - Bulk Read\n\r"
				"5 - Stream\n\r"
				"9 - Power On Reset\n\r"
				);
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}
	else if(read_Value == '1')
	{
		uint8_t rslt = PMW3901_init(&hspi1, PIN_CS_GPIO_Port, PIN_CS_Pin, PIN_INTERRUPT_GPIO_Port, PIN_INTERRUPT_Pin);
		if(rslt)
		{
			sprintf(sprintfBuffer,"Init FAILED\n\r");
		}
		else
		{
			sprintf(sprintfBuffer,"Init SUCCESS\n\r");
		}
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}
	else if(read_Value == '2')
	{
		uint8_t rslt = PMW3901_writeReg(PWM_REG_MOTION, 0x011);
		if(rslt)
		{
			sprintf(sprintfBuffer,"Write FAILED\n\r");
		}
		else
		{
			sprintf(sprintfBuffer,"Write SUCCESS\n\r");
		}
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}
	else if(read_Value == '3')
	{
		PMW3901_ReadMotion();
		sprintf(sprintfBuffer,
				"\n\r"
				"Motion: "BYTE_TO_BINARY_PATTERN "\n\r"
				"deltax: \n\r"
				"deltay: \n\r"
				"Squal: \n\r"
				,BYTE_TO_BINARY(pmw3901.motion)
				, pmw3901.deltaX
				, pmw3901.deltaY
				, pmw3901.squal
				);
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}
	else if(read_Value == '4')
	{
		PMW3901_ReadMotionBulk();
		PMW3901_init(&hspi1, PIN_CS_GPIO_Port, PIN_CS_Pin, PIN_INTERRUPT_GPIO_Port, PIN_INTERRUPT_Pin);
		sprintf(sprintfBuffer,
				"\n\r"
				"Valid: \n\r"
				"deltax: \n\r"
				"deltay: \n\r"
				"Squal: \n\r"
				"Observ: \n\r"
				"Rawavg: \n\r"
				"RawMx: \n\r"
				"RawMn: \n\r"
				"Shutter: \n\r"
				,pmw3901.isValid
				, pmw3901.deltaX
				, pmw3901.deltaY
				, pmw3901.squal
				,pmw3901.observation
				,pmw3901.rawAverage
				,pmw3901.rawMax
				,pmw3901.rawMin
				,pmw3901.shutter
				,pmw3901.squal
				);
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}
	else if (read_Value = '5')
	{
		uint8_t runPMW = 1;
		uint8_t streamPMW = 1;
	}
	else if(read_Value = '9')
	{
		PMW3901_PowerOnReset();
	}
	else
	{
		sprintf(sprintfBuffer, "Invalid Command\n\r");
		HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
	}

	printf("END\n\r");
}

void streamData()
{
	if(streamPMW)
	{
		static uint32_t lastMillis = 0;
		if(HAL_GetTick() - lastMillis >= 5)
		{
			lastMillis = HAL_GetTick();
			sprintf(sprintfBuffer, "%i %i %i\n\r", pmw3901.deltaX, pmw3901.deltaY, pmw3901.squal);
			HAL_UART_Transmit(&huart2, sprintfBuffer, strlen(sprintfBuffer), 1000);
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
//	if(huart = &huart2)
//	{
//		for(int i = 0; i<512; i++)
//		{
//			sprintfBuffer[i] = '\0';
//		}
//	}
	for(int i = 0; i<512; i++)
	{
		sprintfBuffer[i] = '\0';
	}
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
