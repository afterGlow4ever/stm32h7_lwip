//===============================================
//
//	File: main.c
//	Author: afterGlow,4ever
//	Date: 04282022
//	Version: v1.0
//
// 	This file is including main function.
//
//===============================================

#include "main.h"

int Key_Value = 4;
extern uint8_t flag;

#define START_TASK_PRIO		1
#define START_STK_SIZE 		2048 
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

#define KEY_TASK_PRIO		2
#define KEY_STK_SIZE 		1024  
TaskHandle_t KeyTask_Handler;
void key_task(void *pvParameters);

#define LED1_TASK_PRIO		3	
#define LED1_STK_SIZE 		128  
TaskHandle_t LED1Task_Handler;
void led1_task(void *pvParameters);

int main(void)
{
	HAL_Init();

	SystemClock_Config();//400MHz
	delay_init();
	Cache_EthernetConfigure();	
	MPU_EthernetConfigure();	
	UART1_BSP_Init(115200);
	LED_BSP_Init();
	KEY_BSP_Init();
	if(PCF8574_Init())
	{
		printf("pcf8574 error.\n\r");
	}
	
	printf("Stm32h7 lwip test begins.\n\r");		
	
	xTaskCreate((TaskFunction_t )start_task,          
							(const char*    )"start_task",        
							(uint16_t       )START_STK_SIZE,      
							(void*          )NULL,                
							(UBaseType_t    )START_TASK_PRIO,     
							(TaskHandle_t*  )&StartTask_Handler); 
	vTaskStartScheduler();    
}

void start_task(void *pvParameters)
{
  taskENTER_CRITICAL();        

	Netif_Config();
	User_notification(&gnetif);	
	
#ifdef STM32_TCP_CLIENT_NETCONN
	tcp_client_init();
#endif

#ifdef STM32_TCP_SERVER_NETCONN
	tcp_server_init();
#endif

#ifdef STM32_UDP_NETCONN
	udp_normal_init();
#endif
	
#ifdef STM32_TCP_CLIENT_SOCKET 
	tcp_client_init();
#endif

#ifdef STM32_TCP_SERVER_SOCKET 
	tcp_server_init();
#endif

#ifdef STM32_UDP_VC_SOCKET
	udp_virtual_connect_init();
#endif

#ifdef STM32_UDP_SOCKET
	udp_normal_init();
#endif

	xTaskCreate((TaskFunction_t )key_task,     	
							(const char*    )"key_task",   	
							(uint16_t       )KEY_STK_SIZE, 
							(void*          )NULL,				
							(UBaseType_t    )KEY_TASK_PRIO,	
							(TaskHandle_t*  )&KeyTask_Handler);   
	xTaskCreate((TaskFunction_t )led1_task,     
							(const char*    )"led1_task",   
							(uint16_t       )LED1_STK_SIZE, 
							(void*          )NULL,
							(UBaseType_t    )LED1_TASK_PRIO,
							(TaskHandle_t*  )&LED1Task_Handler);        

	vTaskDelete(StartTask_Handler);
	taskEXIT_CRITICAL();        
}

void key_task(void *pvParameters)
{
	while(1)
	{
#ifdef STM32_TCP_CLIENT_NETCONN
		tcp_client_loop();
#endif

#ifdef STM32_TCP_SERVER_NETCONN
		tcp_server_loop();
#endif

#ifdef STM32_UDP_NETCONN
		udp_normal_loop();
#endif		

#ifdef STM32_TCP_CLIENT_SOCKET
		tcp_client_loop();
#endif

#ifdef STM32_TCP_SERVER_SOCKET
		tcp_server_loop();
#endif

#ifdef STM32_UDP_VC_SOCKET
		udp_virtual_connect_loop();
#endif	

#ifdef STM32_UDP_SOCKET
		udp_normal_loop();
#endif
	}
}   

void led1_task(void *pvParameters)
{
	while(1)
	{
		LED1_Toggle;
		vTaskDelay(1000);
	}	
}

/**
  * @brief  System Clock 配置
  *         system Clock 配置如下: 
	*            System Clock source  = PLL (HSE)
	*            SYSCLK(Hz)           = 400000000 (CPU Clock)
	*            HCLK(Hz)             = 200000000 (AXI and AHBs Clock)
	*            AHB Prescaler        = 2
	*            D1 APB3 Prescaler    = 2 (APB3 Clock  120MHz)
	*            D2 APB1 Prescaler    = 2 (APB1 Clock  120MHz)
	*            D2 APB2 Prescaler    = 2 (APB2 Clock  120MHz)
	*            D3 APB4 Prescaler    = 2 (APB4 Clock  120MHz)
	*            HSE Frequency(Hz)    = 25000000
	*            PLL_M                = 5
	*            PLL_N                = 160
	*            PLL_P                = 2
	*            PLL_Q                = 4
	*            PLL_R                = 2
	*            VDD(V)               = 3.3
	*            Flash Latency(WS)    = 4
  * @param  None
  * @retval None
  */
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** 启用电源配置更新
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** 配置主内稳压器输出电压
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** 初始化CPU、AHB和APB总线时钟
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		while(1);
  }
  /** 初始化CPU、AHB和APB总线时钟
  */
	/* 选择PLL作为系统时钟源并配置总线时钟分频器 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK  | \
																 RCC_CLOCKTYPE_HCLK    | \
																 RCC_CLOCKTYPE_D1PCLK1 | \
																 RCC_CLOCKTYPE_PCLK1   | \
                                 RCC_CLOCKTYPE_PCLK2   | \
																 RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4)!= HAL_OK)
  {
    while(1) { ; }
  }
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
		while(1);
  }
}
/****************************END OF FILE***************************/

