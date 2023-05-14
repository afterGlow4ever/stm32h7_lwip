//===============================================
//
//	File: LAN8720a.c
//	Author: afterGlow,4ever
//	Date: 04282023
//	Version: v1.0
//
// 	This file is including eth end lan8720a basic operations.
//
//===============================================
  
#include "eth.h"
#include "pcf8574.h"

extern ETH_HandleTypeDef EthHandle;

//===============================================
// eth bsp 
// called in low_level_init
//===============================================

static void ETH_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    
    ETH_MDIO_GPIO_CLK_ENABLE();
    ETH_MDC_GPIO_CLK_ENABLE();
    ETH_RMII_REF_CLK_GPIO_CLK_ENABLE();
    ETH_RMII_CRS_DV_GPIO_CLK_ENABLE();
    ETH_RMII_RXD0_GPIO_CLK_ENABLE();
    ETH_RMII_RXD1_GPIO_CLK_ENABLE();
    ETH_RMII_TX_EN_GPIO_CLK_ENABLE();
    ETH_RMII_TXD0_GPIO_CLK_ENABLE();    
    ETH_RMII_TXD1_GPIO_CLK_ENABLE();

    INTX_DISABLE();                         
    PCF8574_WriteBit(ETH_RESET_IO,1);     
		delay_ms(100);
    PCF8574_WriteBit(ETH_RESET_IO,0);    
		delay_ms(100);
    INTX_ENABLE();                       
	
    GPIO_InitStructure.Pin = ETH_MDIO_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = ETH_MDIO_AF;
    HAL_GPIO_Init(ETH_MDIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_MDC_PIN;
    GPIO_InitStructure.Alternate = ETH_MDC_AF;
    HAL_GPIO_Init(ETH_MDC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_REF_CLK_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_REF_CLK_AF;
    HAL_GPIO_Init(ETH_RMII_REF_CLK_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_CRS_DV_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_CRS_DV_AF;
    HAL_GPIO_Init(ETH_RMII_CRS_DV_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_RXD0_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_RXD0_AF;
    HAL_GPIO_Init(ETH_RMII_RXD0_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_RXD1_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_RXD1_AF;
    HAL_GPIO_Init(ETH_RMII_RXD1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_TX_EN_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TX_EN_AF;
    HAL_GPIO_Init(ETH_RMII_TX_EN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_TXD0_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TXD0_AF;
    HAL_GPIO_Init(ETH_RMII_TXD0_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = ETH_RMII_TXD1_PIN;
    GPIO_InitStructure.Alternate = ETH_RMII_TXD1_AF;
    HAL_GPIO_Init(ETH_RMII_TXD1_PORT, &GPIO_InitStructure);          
}  

void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    ETH_GPIO_Config();
  
    HAL_NVIC_SetPriority(ETH_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
  
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();    
}  

//===============================================
// lan8720a init 
// called in low_level_init
//===============================================

HAL_StatusTypeDef LAN8720_Init(ETH_HandleTypeDef *heth)
{
    uint32_t phyreg = 0;
    uint32_t TIME_Out = 0;
    
    // sw reset
    if(HAL_ETH_WritePHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, PHY_RESET) != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    //wait for reset ok
    HAL_Delay(PHY_RESET_DELAY);
    
    // start auto-negotiation
     if((HAL_ETH_ReadPHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, &phyreg)) != HAL_OK)
    {
      return HAL_ERROR;   
    }     
    phyreg = phyreg | PHY_AUTONEGOTIATION;
    if((HAL_ETH_WritePHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, phyreg)) != HAL_OK)
    {
      return HAL_ERROR;   
    }     

    // wait operation done
    HAL_Delay(0xFFF);
    do
    {     
      HAL_ETH_ReadPHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BSR, &phyreg);
      TIME_Out++;
      if(TIME_Out > PHY_READ_TO) 
        return HAL_TIMEOUT;
    } while (((phyreg & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));
    
    return HAL_OK;    
}

//===============================================
// get link status
//===============================================

uint32_t LAN8720_GetLinkState(ETH_HandleTypeDef *heth)
{
    uint32_t phyreg = 0;   
  
    if(HAL_ETH_ReadPHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_SR, &phyreg) == HAL_OK)
        return phyreg;
    return 0;
}

//===============================================
// eth handler
//===============================================

void ETH_IRQHandler(void)
{
  uint32_t ulReturn;
  HAL_ETH_IRQHandler(&EthHandle);
  ulReturn = taskENTER_CRITICAL_FROM_ISR();
  HAL_ETH_IRQHandler(&EthHandle);
  taskEXIT_CRITICAL_FROM_ISR(ulReturn);  
}

//===============================================
// eth callback
//===============================================

extern xSemaphoreHandle s_xSemaphore;
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(s_xSemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
  ;
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
  ;
}
