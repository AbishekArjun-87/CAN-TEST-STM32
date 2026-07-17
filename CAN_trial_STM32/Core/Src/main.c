/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern UART_HandleTypeDef hcom_uart[];

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

COM_InitTypeDef BspCOMInit;
FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void CAN_TRANSMIT_MESSAGE(void);
HAL_StatusTypeDef MCP9808_ReadTemp_x100(uint16_t addr, int16_t *temp_x100);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MCP9808_ADDR        (0x18 << 1)
#define MCP9808_ADDR_2        (0x19 << 1) // Adding the second sensor
#define MCP9808_REG_TEMP 0x05 //Defining the Register address inside the sensor
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
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_FDCAN_Start(&hfdcan1);
  HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_TX_COMPLETE,FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2  );
  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_BUS_OFF | FDCAN_IT_ERROR_WARNING, 0);

  /* USER CODE END 2 */

  /* Initialize led */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }


  if (HAL_I2C_IsDeviceReady(&hi2c1, MCP9808_ADDR, 3, 100) == HAL_OK)
  {
	  char msg1[]=" S 1 available \r \n";
	  	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg1,sizeof(msg1)-1,100); //Doing serial print
  }
  else
  {
	  char msg2[]=" S 1 unavailable\r \n";
	  	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg2,sizeof(msg2)-1,100); //Doing serial print
  }



  if (HAL_I2C_IsDeviceReady(&hi2c1, MCP9808_ADDR_2, 3, 100) == HAL_OK)
  {
	  char msg1[]=" S 2 available \r \n";
	  	  	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg1,sizeof(msg1)-1,100); //Doing serial print
  }
  else
  {
	  char msg2[]=" S 2 unavailable \r \n";
	  	  	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg2,sizeof(msg2)-1,100); //Doing serial print
  }


  char msg2[]=" Scanning the bus \r \n";
 	  	  	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg2,sizeof(msg2)-1,100); //Doing serial print

  for (uint8_t addr = 1; addr < 128; addr++)
  {
      if (HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 3, 100) == HAL_OK)
      {
          char msg[40];
          snprintf(msg, sizeof(msg), "Found device at 0x%02X\r\n", addr);
          HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)msg, strlen(msg), 100);
      }

  }
  char scanDone[] = "Scan complete, entering main loop\r\n";
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)scanDone, strlen(scanDone)-1, 100);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  CAN_TRANSMIT_MESSAGE();
	  HAL_Delay(500);
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 4;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 13;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */



HAL_StatusTypeDef MCP9808_ReadTemp_x100(uint16_t addr , int16_t *temp_x100)
{
    uint8_t data[2];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &hi2c1,
        addr,
        MCP9808_REG_TEMP,
        I2C_MEMADD_SIZE_8BIT,
        data,
        2,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }

    /*
     * MCP9808 temperature register:
     * data[0] bits:
     * bit 4 = sign bit
     * bits 3:0 = upper temperature bits
     * data[1] = lower temperature bits
     *
     * Raw temperature unit = 1/16 degC = 0.0625 degC
     */
    uint16_t raw = ((uint16_t)(data[0] & 0x1F) << 8) | data[1];

    // Sign extend 13-bit value
    if (raw & 0x1000)
    {
        raw |= 0xE000;
    }

    int16_t signed_raw = (int16_t)raw;

    // Convert from 1/16 degC to degC x100
    // temp_x100 = temperature * 100
    *temp_x100 = (signed_raw * 25) / 4;

    return HAL_OK;
}



static void CAN_TRANSMIT_MESSAGE()
{
	FDCAN_TxHeaderTypeDef TxHeader;
	TxHeader.Identifier= 0x786;
	TxHeader.IdType= FDCAN_STANDARD_ID;
	TxHeader.TxFrameType= FDCAN_DATA_FRAME;
	  TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	   TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	   TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	   TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	   TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	   TxHeader.MessageMarker = 0;

	   //uint8_t TxData[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

	   int16_t temp_1,temp_2;
	   uint8_t TxData[8]={0};


	   HAL_StatusTypeDef res1 = MCP9808_ReadTemp_x100(MCP9808_ADDR, &temp_1);
	    HAL_StatusTypeDef res2 = MCP9808_ReadTemp_x100(MCP9808_ADDR_2, &temp_2);

	      if (res1 == HAL_OK) //Sesnsor 1 and Sensor 2
	         {
	    	  char msg2[]=" Recording temperature 1\r \n";
	    	 	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg2,sizeof(msg2)-1,100); //Doing serial print

	    	 	    	   TxData[0] = (temp_1>>8) & 0xFF; 	//Sensor 1 upper byte
	    	 	    	   TxData[1] = (temp_1) & 0xFF;  		// Sensor 1 lower byte

	         }
	      else
	      {
	          char err1[40];
	          snprintf(err1, sizeof(err1), "Sensor 1 FAILED, status=%d\r\n", res1);
	          HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)err1, strlen(err1), 100);
	      }


	    	  if(res2 == HAL_OK)
		{
	    	  	  char msg[]=" Recording temperature 2\r \n";
	    	  	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg,sizeof(msg)-1,100); //Doing serial print

	    	  			TxData[2] = (temp_2>>8) & 0xFF; 	//Upper part of the CAN frame.
	    	  			TxData[3] = (temp_2) & 0xFF;  		//Lower part of the CAN frame.
	         }
	    	    else
	    	    {
	    	        char err2[40];
	    	        snprintf(err2, sizeof(err2), "Sensor 2 FAILED, status=%d\r\n", res2);
	    	        HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)err2, strlen(err2), 100);
	    	    }



	   if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK) //Message is added to FIFOQueue
	   	{																		//If condition to see if the Queue is full or not.
	   	    BSP_LED_On(LED_GREEN);   // stays on if a send ever fails
	   	}

}


void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan,uint32_t BufferIndexes)
{
	char msg[]="CAN message ACK \r \n";
	HAL_UART_Transmit(&hcom_uart[COM1],(uint8_t*)msg,sizeof(msg)-1,100); //Doing serial print
	BSP_LED_Toggle(LED_GREEN);
}
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	char msg[] = "CAN ERROR - no ACK / bus issue\n";
	HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)msg, sizeof(msg)-1, 100);
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
#ifdef USE_FULL_ASSERT
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
