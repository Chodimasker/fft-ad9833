/*
*********************************************************************************************************
*
*	模块名称 : AD9833 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_adc9833.c
*
*********************************************************************************************************
*/
#ifndef _BSP_AD9833_H
#define _BSP_AD9833_H

/*定义波形的寄存器值*/
#define Triangle_Wave  0x2002
#define Sine_Wave      0x2028
#define square wave   0x2000

#define FREQ_0 0
#define FREQ_1 1

void bsp_InitAD9833(void);
void AD9833_Write_16Bits(uint16_t _cmd);

/* AD9833函数声明 */
void AD9833_Write_16Bits(uint16_t _cmd) ;//写一个字到AD9833
void AD9833_SetFreq(uint32_t _freq) ;
void AD9833_SelectWave(uint8_t _Type) ;

#endif
