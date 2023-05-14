//=======================================================================================


//=======================================================================================

#include "myiic.h"

//===============================================
// i2c init
//===============================================

void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOH_CLK_ENABLE();
    GPIO_Initure.Pin=GPIO_PIN_4|GPIO_PIN_5;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull=GPIO_PULLUP;
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
    IIC_SDA(1);
    IIC_SCL(1);  
}

//===============================================
// i2c start
//===============================================

void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA(1);	  	  
	IIC_SCL(1);
	delay_us(4);
 	IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL(0);
}	 

//===============================================
// i2c stop
//===============================================

void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL(0);
	IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL(1); 
	IIC_SDA(1);
	delay_us(4);							   	
}

//===============================================
// i2c wait for ack
// 1: nack
// 0: ack
//===============================================

uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();
	IIC_SDA(1);
	delay_us(1);	   
	IIC_SCL(1);
	delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0); 
	return 0;  
} 

//===============================================
// i2c ack
//===============================================

void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}

//===============================================
// i2c nack
//===============================================	  

void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}			

//===============================================
// i2c tx byte
//===============================================		  

void IIC_Send_Byte(uint8_t txd)
{                        
  uint8_t t;   
	SDA_OUT(); 	    
  IIC_SCL(0);
  for(t=0;t<8;t++)
  {              
		IIC_SDA((txd & 0x80) >> 7);
		txd <<= 1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL(1);
		delay_us(2); 
		IIC_SCL(0);	
		delay_us(2);
  }	 
} 	    

//===============================================
// i2c rx byte
// 1: ack
// 0: nack
//===============================================		

uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
  for(i=0;i<8;i++ )
	{
    IIC_SCL(0); 
    delay_us(2);
		IIC_SCL(1);
    receive<<=1;
    if(READ_SDA)
			receive++;   
		delay_us(1); 
  }	
	
	if (!ack)
		IIC_NAck();
	else
		IIC_Ack();
	return receive;
}
