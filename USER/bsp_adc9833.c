/*
*********************************************************************************************************
*
*	模块名称 : AD9833 驱动模块(单通道带16位ADC)
*	文件名称 : bsp_AD9833.c
*	说    明 : AD9833模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口和软件SPI接口。
*			  通过宏切换。
*
*********************************************************************************************************
*/
#include "sys.h"

//#include "bsp.h"

#define SOFT_SPI		/* 定义此行表示使用GPIO模拟SPI接口 */
//#define HARD_SPI		/* 定义此行表示使用CPU的硬件SPI接口 */

/* AD9833晶振频率 */
#define AD9833_SYSTEM_CLOCK 25000000UL 

/*	

   AD9833/ DAC8563模块    STM32F407开发板
	  GND   ------  GND    
	  VCC   ------  3.3V	  
	  FSYNC  ------ PF7      	  
      SCLK ------  PB3
      SDATA------  PB5
*/

/*
	AD9833基本特性:
	1、供电2.3 - 5.5V;  【本例使用3.3V】
	4、参考电压2.5V，使用内部参考

	对SPI的时钟速度要求: 高达40MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* 软件SPI */
	/* 定义GPIO端口 */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOB
	#define PORT_SCLK	GPIOB
	#define PIN_SCLK	GPIO_PIN_3
	
	#define RCC_SDATA 	RCC_AHB1Periph_GPIOB
	#define PORT_SDATA	GPIOB
	#define PIN_SDATA	GPIO_PIN_5
	
	/* 片选 */
	#define RCC_FSYNC 	RCC_AHB1Periph_GPIOF
	#define PORT_FSYNC	GPIOF
	#define PIN_FSYNC	GPIO_PIN_7

	/* 定义口线置0和置1的宏 */
	#define FSYNC_0()	PORT_FSYNC->BSRRH = PIN_FSYNC
	#define FSYNC_1()	PORT_FSYNC->BSRRL = PIN_FSYNC

	#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
	#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

	#define SDATA_0()		PORT_SDATA->BSRRH = PIN_SDATA
	#define SDATA_1()		PORT_SDATA->BSRRL = PIN_SDATA

#endif

#ifdef HARD_SPI		/* 硬件SPI (未做) */
	;
#endif



/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD9833
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 AD9833
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitAD9833(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	FSYNC_1();	/* FSYNC = 1 */

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_SDATA | RCC_FSYNC, ENABLE);

	/* 配置几个推挽输出IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = PIN_SCLK;
	GPIO_Init(PORT_SCLK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_SDATA;
	GPIO_Init(PORT_SDATA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_FSYNC;
	GPIO_Init(PORT_FSYNC, &GPIO_InitStructure);
#endif

}

/*
*********************************************************************************************************
*	函 数 名: AD9833_Write_16Bits      
*	功能说明: 向SPI总线发送16个bit数据   发送控制字
*	形    参: _cmd : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_Write_16Bits(uint16_t _cmd)
{
	uint8_t i;
	SCLK_1();
	FSYNC_0();
	
	/*　AD9833  SCLK时钟高达40M，因此可以不延迟 */
	for(i = 0; i < 16; i++)
	{
		if (_cmd & 0x8000)
		{
			SDATA_1();
		}
		else
		{
			SDATA_0();
		}
		SCLK_0();
		_cmd <<= 1;
		SCLK_1();
	}
	
	FSYNC_1();
}

/*
*********************************************************************************************************
*	函数名：AD9833_SelectWave
*	功能说明：软件位控制

*	IOUT正弦波 ，SIGNBITOUT方波 ，写FREQREG0 ，写PHASE0
*	ad9833_write_16bit(0x2028) 一次性写FREQREG0
*	ad9833_write_16bit(0x0038) 单独改写FREQREG0的LSB
*	ad9833_write_16bit(0x1038) 单独改写FREQREG0的MSB
 
*	IOUT三角波 ，写PHASE0
*	ad9833_write_16bit(0x2002)一次性写FREQREG0
*	ad9833_write_16bit(0x0002)单独改写FREQREG0的LSB
*	ad9833_write_16bit(0x1008)单独改写FREQREG0的MSB

*   形参：_Type -- 波形类型 
*   返回值 ：无

*********************************************************************************************************
*/
void AD9833_SelectWave(uint8_t _Type) 
{
	FSYNC_1();  //宏定义
	SCLK_1();
	if(_Type == 0)
	{
		AD9833_Write_16Bits(0x2028); /*频率寄存器输出方波*/
	}
	else if(_Type == 1)
	{
		AD9833_Write_16Bits(0x2002); /*频率寄存器输出三角波*/
	}
	else if(_Type == 2)
	{
		AD9833_Write_16Bits(0x2000); /*频率寄存器输出正弦波*/
	}
	else if(_Type == 3)
	{
		AD9833_Write_16Bits(0x00C0); /*无输出*/
	}
 

}

/****************************************************************
函数名称: AD9833_SetFreq
功能: 设置频率值   
参数: _freq
freq -- 频率值 (Freq_value(value)=Freq_data(data)*FCLK/2^28) 
返回值: 无
*****************************************************************/
void AD9833_SetFreq(uint32_t _freq)
{
	uint32_t freq;
	uint16_t lsb_14bit;
	uint16_t msb_14bit;
	uint8_t freq_number = 0;

	freq = (uint32_t)(268435456.0 / AD9833_SYSTEM_CLOCK * _freq);
	lsb_14bit = (uint16_t)freq;
	msb_14bit = (uint16_t)(freq >> 14);
	if(freq_number == FREQ_0)
	{
		lsb_14bit &= ~(1U<<15);//0111 1111 1111 1111 先把第15位清0,其他位不变
		lsb_14bit |= 1<<14;    //0100 0000 0000 0000 再把第14位置1,其他位不变 结果就是01xx xxxx xxxx xxxx
		msb_14bit &= ~(1U<<15); //同上
		msb_14bit |= 1<<14;
	}
	else
	{
		lsb_14bit &= ~(1<<14); //1011 1111 1111 1111 先把第14位清0,其他位不变
		lsb_14bit |= 1U<<15;   //1000 0000 0000 0000 再把第15位置1,其他位不变 结果就是10xx xxxx xxxx xxxx
		msb_14bit &= ~(1<<14); //同上
		msb_14bit |= 1U<<15;
	}

	AD9833_Write_16Bits(lsb_14bit);
	AD9833_Write_16Bits(msb_14bit);
}