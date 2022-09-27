/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "adc_driver.h"
#include "adc_device.h"
#include <stdarg.h>
#include "mk_gpio_drv.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
#define O_WRONLY			(1 << 1)

/**
  * @brief  The application entry point.
  * @retval int
  */
int fd = 0;
int fdreg = 0;
int ret = 0;
int m = 0;
char *link_buf = 0;

struct device_t * test_dev;

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  
  LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9|LL_GPIO_PIN_10);
  /* USER CODE END 2 */
 
   // strcpy(jsonBuf,DEVICE_TREE_JSON);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if 1

    mk_init();//初始化MK组件
    //adc_d1_driver_init();//注册驱动
    //probe_device();//搜索驱动并注册设备

//-------------驱动二次开发
    mk_gpio_driver_init();//总线初始化

    init_driver();//分配总线

    init_led_device();//初始化驱动
    test_dev = search_first_device(DEVICE_TYPE_LED);
    //unregister_gpdev();
//-----------------------

#if 1
//----------------设备应用---------------------
    uint8_t buf[5];
//打开
    fdreg = vfs_open("/device/led/red/switch", O_RDONLY);
    if(fdreg > 0)
     {
        //读取
        ret = vfs_ioctl(fdreg, 1, NULL);
        LL_mDelay(1000);
        ret = vfs_ioctl(fdreg, 0, NULL);
        LL_mDelay(1000);
        //关闭
        ret = vfs_close(fdreg);
        if(ret < 0)
          LL_mDelay(1000);
     }

#endif

//-------------------驱动二次开发------------------
#if 0
  uint8_t bufa[5];
//打开
  fdreg = vfs_open("/driver/gpio/gpio9", O_RDONLY);
  if(fdreg > 0)
  {
  //读取
    ret = vfs_ioctl(fdreg,1,NULL);
    if(ret < 0)
      LL_mDelay(1000);
  //关闭
    ret = vfs_close(fdreg);
    if(ret < 0)
      LL_mDelay(1000);
  }
#endif
//------------------------------------------------
#if 0
//----------------设备应用---------------------
  uint8_t buf[5];
//打开
  fdreg = vfs_open("/device/adc/adc_test/ioctl", O_RDONLY);
  if(fdreg < 0)
    LL_mDelay(1000);
//读取
  ret = vfs_ioctl(fdreg, 5,NULL);
  if(ret < 0)
    LL_mDelay(1000);
//关闭
  ret = vfs_close(fdreg);
  if(ret < 0)
    LL_mDelay(1000);
#endif
//----------------驱动应用---------------------
#if 0
  uint8_t buf[5];
//打开
  fdreg = vfs_open("/driver/adc-d1/probe", O_RDONLY);
  if(fdreg < 0)
    LL_mDelay(1000);
//读取
  ret = vfs_write(fdreg, buf,5);
  if(ret < 0)
    LL_mDelay(1000);
//关闭
  ret = vfs_close(fdreg);
  if(ret < 0)
    LL_mDelay(1000);
#endif

#endif
  

  while (1)
  {

    /* USER CODE END WHILE */
    LL_mDelay(1000);
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
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 168, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(168000000);
  LL_SetSystemCoreClock(168000000);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);

  /**/
  LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_9|LL_GPIO_PIN_10);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOF, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
