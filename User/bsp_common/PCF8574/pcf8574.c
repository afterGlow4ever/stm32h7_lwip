//=======================================================================================


//=======================================================================================

#include "pcf8574.h"

//=========================================
// pcf8574 init
//=========================================

uint8_t PCF8574_Init(void)
{
  uint8_t temp=0;
  GPIO_InitTypeDef GPIO_Initure;
	
  __HAL_RCC_GPIOB_CLK_ENABLE();   
  GPIO_Initure.Pin=GPIO_PIN_12;         
  GPIO_Initure.Mode=GPIO_MODE_INPUT;     
  GPIO_Initure.Pull=GPIO_PULLUP;        
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);   
	
	// call pcf8574
  IIC_Init();
  IIC_Start();    	 	   
	IIC_Send_Byte(PCF8574_ADDR);
	temp = IIC_Wait_Ack();
  IIC_Stop();
  PCF8574_WriteOneByte(0XFF);
	return temp;
}

//=========================================
// pcf8574 read byte
//=========================================

uint8_t PCF8574_ReadOneByte(void)
{				  
	uint8_t temp=0;		  	    																 
  IIC_Start();    	 	   
	IIC_Send_Byte(PCF8574_ADDR | 0X01); 
	IIC_Wait_Ack();	 
  temp = IIC_Read_Byte(0);		   
  IIC_Stop();
	return temp;
}

//=========================================
// pcf8574 write byte
//=========================================

void PCF8574_WriteOneByte(uint8_t data)
{				   	  	    																 
  IIC_Start();  
  IIC_Send_Byte(PCF8574_ADDR | 0X00);
	IIC_Wait_Ack();	    										  		   
	IIC_Send_Byte(data);						   
	IIC_Wait_Ack();      
  IIC_Stop();
	delay_us(10000);	 
}

//=========================================
// pcf8574 write bit
// bit: 0~7
// sta: 0/1
//=========================================

void PCF8574_WriteBit(uint8_t bit, uint8_t sta)
{
	uint8_t data;
	data = PCF8574_ReadOneByte();
	if(sta==0)
		data &= ~(1<<bit);     
	else 
		data |= 1<<bit;
	PCF8574_WriteOneByte(data);
}

//=========================================
// pcf8574 read bit
// bit: 0~7
// return: 0/1
//=========================================

uint8_t PCF8574_ReadBit(uint8_t bit)
{
	uint8_t data;
	data = PCF8574_ReadOneByte();
	if(data & (1<<bit))
		return 1;
	else 
		return 0;   
}  
