/************************************************************************************
* \file         Demo/ARMCM4_STM32F4_F401xC_GCC/app/main.c
* \brief        Bootloader application source file.
* \ingroup      Boot_ARMCM4_STM32F4_F401xC_GCC
* \internal
*-----------------------------------------------------------------------------------*/

#include "header.h"

#define USART_CHANNEL   USART2
#define USART_BAUDRATE  9600U

static void Init(void);
static void SystemClock_Config(void);

static void USARTPrint(uint8_t* buffer);

int main(void)
{
  Init();

  char msg[] = "Hello world from App.\r\n";
  USARTPrint((uint8_t *)msg);
  while (1);

  return 0;
}

static void Init(void)
{
  SystemClock_Config();
  HAL_Init();
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_CR_HSEON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    while(1);
  }
} 

static void GPIOInit()
{
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void GPIODeInit()
{ 

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);

  __HAL_RCC_GPIOA_CLK_DISABLE();
}


static void USARTInit(void)
{
  __HAL_RCC_USART2_CLK_ENABLE();

  LL_USART_InitTypeDef USART_InitStruct;
  
  LL_USART_Disable(USART_CHANNEL);

  USART_InitStruct.BaudRate = USART_BAUDRATE;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  
  LL_USART_Init(USART_CHANNEL, &USART_InitStruct);
  LL_USART_Enable(USART_CHANNEL);
}

static void USARTDeInit(void)
{
  LL_USART_Disable(USART_CHANNEL);  
  LL_USART_DeInit(USART_CHANNEL);

  __HAL_RCC_USART2_CLK_DISABLE();
}


static void USARTPrint(uint8_t* buffer)
{
	uint8_t *ptr = buffer;
	while(*ptr != 0) {		
    LL_USART_TransmitData8(USART_CHANNEL, *ptr++);

		while (LL_USART_IsActiveFlag_TXE(USART_CHANNEL) == 0);
	}
}

void HAL_MspInit(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  GPIOInit();
  USARTInit();  
}

void HAL_MspDeInit(void)
{  
  USARTDeInit();
  GPIODeInit();
  
  __HAL_RCC_PWR_CLK_DISABLE();
  __HAL_RCC_SYSCFG_CLK_DISABLE();
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}
