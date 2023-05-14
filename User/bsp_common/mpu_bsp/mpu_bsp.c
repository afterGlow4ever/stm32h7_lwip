//=======================================================================================


//=======================================================================================

#include "mpu_bsp.h"

//=========================================
// mpu for eth
//=========================================

void MPU_EthernetConfigure(void)
{
	MPU_Region_InitTypeDef MPU_Initure;
	/* Configure the MPU attributes as Device not cacheable 
	for ETH DMA descriptors */
	MPU_Initure.Enable = MPU_REGION_ENABLE;
	MPU_Initure.BaseAddress = 0x30040000;
	MPU_Initure.Size = MPU_REGION_SIZE_256B;
	MPU_Initure.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_Initure.IsBufferable = MPU_ACCESS_BUFFERABLE;
	MPU_Initure.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_Initure.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.Number = MPU_REGION_NUMBER0;
	MPU_Initure.TypeExtField = MPU_TEX_LEVEL0;
	MPU_Initure.SubRegionDisable = 0x00;
	MPU_Initure.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_Initure);

	/* Configure the MPU attributes as Cacheable write through 
	for LwIP RAM heap which contains the Tx buffers */
	MPU_Initure.Enable = MPU_REGION_ENABLE;
	MPU_Initure.BaseAddress = 0x30044000;
	MPU_Initure.Size = MPU_REGION_SIZE_16KB;
	MPU_Initure.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_Initure.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_Initure.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_Initure.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.Number = MPU_REGION_NUMBER1;
	MPU_Initure.TypeExtField = MPU_TEX_LEVEL0;
	MPU_Initure.SubRegionDisable = 0x00;
	MPU_Initure.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_Initure);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

//=========================================
// cache config
//=========================================

void Cache_EthernetConfigure(void)
{
	SCB_EnableICache();
	SCB_EnableDCache();  
	//cache write-through
	SCB->CACR|=1<<2;			
}
