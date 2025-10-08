/************************************************************************************//**
* \file         Demo/ARMCM4_STM32F4_F401xC_GCC/Boot/main.c
* \brief        Bootloader application source file.
* \ingroup      Boot_ARMCM4_STM32F4_F401xC_GCC
* \internal
*----------------------------------------------------------------------------------------
*                          C O P Y R I G H T
*----------------------------------------------------------------------------------------
*   Copyright (c) 2021  by Feaser    http://www.feaser.com    All rights reserved
*
*----------------------------------------------------------------------------------------
*                            L I C E N S E
*----------------------------------------------------------------------------------------
* This file is part of OpenBLT. OpenBLT is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* OpenBLT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License along with OpenBLT. It
* should be located in ".\Doc\license.html". If not, contact Feaser to obtain a copy.
*
* \endinternal
****************************************************************************************/

/****************************************************************************************
* Include files
****************************************************************************************/
#include "boot.h"                                /* bootloader generic header          */
#include "stm32f4xx.h"                           /* STM32 CPU and HAL header           */

/****************************************************************************************
* Function prototypes
****************************************************************************************/
static void Init(void);
static void SystemClock_Config(void);

#if (BOOT_BACKDOOR_HOOKS_ENABLE > 0)
extern blt_bool BackDoorEntryHook(void);
#endif


/************************************************************************************//**
** \brief     This is the entry point for the bootloader application and is called
**            by the reset interrupt vector after the C-startup routines executed.
** \return    Program return code.
**
****************************************************************************************/
int main(void)
{

  blt_int8u deferredInitRequestFlag = 0;

  /* initialize the microcontroller */
  Init();
  /* initialize the bootloader */
  BootInit();
#if (BOOT_COM_DEFERRED_INIT_ENABLE == 1)
  /* the bootloader is configured to NOT initialize the TCP/IP network stack by default
   * to bypass unnecessary delay times before starting the user program. the TCP/IP net-
   * work tack is now only initialized when: (a) no valid user program is detected, or
   * (b) a forced backdoor entry occurred (CpuUserProgramStartHook() returned BLT_FALSE).
   *
   * these demo bootloader and user programs have one extra feature implemented for
   * demonstration purposes. the demo user program can detect firmware update requests
   * from the TCP/IP network in which case it activates the bootloader. But...the
   * TCP/IP network stack will not be initialized in this situation. for this reason
   * the shared parameter module was integrated in both the bootloader and user program.
   * more information about the shared parameter module can be found here:
   *   https://www.feaser.com/en/blog/?p=216
   *
   * the shared parameter at the first index (0) contains a flag. this flag is set to
   * 1, right before the user program activates this bootloader, to explicitly request
   * the bootloader to initialize the TCP/IP network stack. this makes it possible for
   * a firmware update to proceed. the code here reads out this flag and performs the
   * TCP/IP network stack initialization when requested.
   */
  SharedParamsReadByIndex(0, &deferredInitRequestFlag);
  if (deferredInitRequestFlag == 1)
  {
    /* explicitly initialize all communication interface for which the deferred
     * initialization feature was enabled.
     */
    ComDeferredInit();
  }
#endif

  /* start the infinite program loop */
  while (1)
  {
    /* run the bootloader task */
    BootTask();

#if (BOOT_BACKDOOR_HOOKS_ENABLE > 0)
  /* attempt to start the user program when no backdoor entry is requested */
  if (BackDoorEntryHook() == BLT_FALSE)
  {
    /* this function does not return if a valid user program is present */
    CpuStartUserProgram();
  }
#endif
  }

  /* program should never get here */
  return 0;
} /*** end of main ***/


/************************************************************************************//**
** \brief     Initializes the microcontroller.
** \return    none.
**
****************************************************************************************/
static void Init(void)
{
  /* configure system clock */
  SystemClock_Config();
  /* HAL library initialization */
  HAL_Init();

} /*** end of Init ***/


/************************************************************************************//**
** \brief     System Clock Configuration. This code was created by CubeMX and configures
**            the system clock to match the configuration in the bootloader's
**            configuration (blt_conf.h), specifically the macros:
**            BOOT_CPU_SYSTEM_SPEED_KHZ and BOOT_CPU_XTAL_SPEED_KHZ.
**            Note that the Lower Layer drivers were selected in CubeMX for the RCC
**            subsystem.
** \return    none.
**
****************************************************************************************/
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Configure the main internal regulator output voltage. */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
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
    /* Clock configuration incorrect or hardware failure. Hang the system to prevent
     * damage.
     */
    ASSERT_RT(BLT_FALSE);
  }

  /* Initializes the CPU, AHB and APB buses clocks. */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    /* Clock configuration incorrect or hardware failure. Hang the system to prevent
     * damage.
     */
    ASSERT_RT(BLT_FALSE);
  }
} /*** end of SystemClock_Config ***/


/************************************************************************************//**
** \brief     Initializes the Global MSP. This function is called from HAL_Init()
**            function to perform system level initialization (GPIOs, clock, DMA,
**            interrupt).
** \return    none.
**
****************************************************************************************/
void HAL_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Power and SYSCFG clock enable. */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  /* GPIO ports clock enable. */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

#if (BOOT_COM_RS232_ENABLE > 0)
  /* UART clock enable. */
  __HAL_RCC_USART1_CLK_ENABLE();
#endif

  /* Configure GPIO pin for the LED. */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

#if (BOOT_CPU_USER_PROGRAM_START_HOOK > 0)
  /* Configure GPIO pin for the program start button. */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif

#if (BOOT_COM_RS232_ENABLE > 0)
  /* UART TX and RX GPIO pin configuration. */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
} /*** end of HAL_MspInit ***/


/************************************************************************************//**
** \brief     DeInitializes the Global MSP. This function is called from HAL_DeInit()
**            function to perform system level de-initialization (GPIOs, clock, DMA,
**            interrupt).
** \return    none.
**
****************************************************************************************/
void HAL_MspDeInit(void)
{
  /* Reset the RCC clock configuration to the default reset state. */
  HAL_RCC_DeInit();

  /* Reset GPIO pin for the LED to turn it off. */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /* Deinit used GPIOs. */
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);

#if (BOOT_CPU_USER_PROGRAM_START_HOOK > 0)
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
#endif


#if (BOOT_COM_RS232_ENABLE > 0)
  /* Deinit used GPIOs. */
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
  /* UART clock disable. */
  __HAL_RCC_USART1_CLK_DISABLE();
#endif

  /* GPIO ports clock disable. */
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOA_CLK_DISABLE();

  /* SYSCFG and PWR clock disable. */
  __HAL_RCC_PWR_CLK_DISABLE();
  __HAL_RCC_SYSCFG_CLK_DISABLE();
} /*** end of HAL_MspDeInit ***/


/************************************************************************************//**
** \brief     This function handles the SysTick interrupt. The HAL driver is initialized
**            before the bootloader disables the global interrupts and reconfigures the
**            SysTick. It is theoretically possible that the SysTick interrupt still
**            fires before the timer driver disables it. Therefore the handler is
**            implemented here. If not, then the default handler from the C startup 
**            code is used, which hangs the system.
** \return    none.
**
****************************************************************************************/
void SysTick_Handler(void)
{
  /* Nothing to do here. */
} /*** end of SysTick_Handler ***/


/*********************************** end of main.c *************************************/
