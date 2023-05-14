#ifndef __PCF8574_H
#define __PCF8574_H

#include "myiic.h"
#include "core_delay.h"

//=========================================
// pcf8574 param
//=========================================

#define PCF8574_INT  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) //PCF8574 INT
#define PCF8574_ADDR 	0X40	//left shift 1 bit

//=========================================
// pcf8574 io mapping
//=========================================

#define BEEP_IO         0		//��������������  	P0
#define AP_INT_IO       1   	//AP3216C�ж�����	P1
#define DCMI_PWDN_IO    2    	//DCMI�ĵ�Դ��������	P2
#define USB_PWR_IO      3    	//USB��Դ��������	P3
#define EX_IO      		4    	//��չIO,�Զ���ʹ�� 	P4
#define MPU_INT_IO      5   	//MPU9250�ж�����	P5
#define RS485_RE_IO     6    	//RS485_RE����		P6
#define ETH_RESET_IO    7    	//��̫����λ����		P7

uint8_t PCF8574_Init(void); 
uint8_t PCF8574_ReadOneByte(void); 
void PCF8574_WriteOneByte(uint8_t data);
void PCF8574_WriteBit(uint8_t bit, uint8_t sta);
uint8_t PCF8574_ReadBit(uint8_t bit);

#endif