/*
*********************************************************************************************************
*
*	ģ������ : AD9833 ����ģ��(��ͨ����16λADC)
*	�ļ����� : bsp_AD9833.c
*	˵    �� : AD9833ģ���CPU֮�����SPI�ӿڡ�����������֧��Ӳ��SPI�ӿں����SPI�ӿڡ�
*			  ͨ�����л���
*
*********************************************************************************************************
*/
#include "sys.h"

//#include "bsp.h"

#define SOFT_SPI		/* ������б�ʾʹ��GPIOģ��SPI�ӿ� */
//#define HARD_SPI		/* ������б�ʾʹ��CPU��Ӳ��SPI�ӿ� */

/* AD9833����Ƶ�� */
#define AD9833_SYSTEM_CLOCK 25000000UL 

/*	

   AD9833/ DAC8563ģ��    STM32F407������
	  GND   ------  GND    
	  VCC   ------  3.3V	  
	  FSYNC  ------ PF7      	  
      SCLK ------  PB3
      SDATA------  PB5
*/

/*
	AD9833��������:
	1������2.3 - 5.5V;  ������ʹ��3.3V��
	4���ο���ѹ2.5V��ʹ���ڲ��ο�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�40MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* ���SPI */
	/* ����GPIO�˿� */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOB
	#define PORT_SCLK	GPIOB
	#define PIN_SCLK	GPIO_PIN_3
	
	#define RCC_SDATA 	RCC_AHB1Periph_GPIOB
	#define PORT_SDATA	GPIOB
	#define PIN_SDATA	GPIO_PIN_5
	
	/* Ƭѡ */
	#define RCC_FSYNC 	RCC_AHB1Periph_GPIOF
	#define PORT_FSYNC	GPIOF
	#define PIN_FSYNC	GPIO_PIN_7

	/* ���������0����1�ĺ� */
	#define FSYNC_0()	PORT_FSYNC->BSRRH = PIN_FSYNC
	#define FSYNC_1()	PORT_FSYNC->BSRRL = PIN_FSYNC

	#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
	#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

	#define SDATA_0()		PORT_SDATA->BSRRH = PIN_SDATA
	#define SDATA_1()		PORT_SDATA->BSRRL = PIN_SDATA

#endif

#ifdef HARD_SPI		/* Ӳ��SPI (δ��) */
	;
#endif



/*
*********************************************************************************************************
*	�� �� ��: bsp_InitAD9833
*	����˵��: ����STM32��GPIO��SPI�ӿڣ��������� AD9833
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitAD9833(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	FSYNC_1();	/* FSYNC = 1 */

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_SDATA | RCC_FSYNC, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

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
*	�� �� ��: AD9833_Write_16Bits      
*	����˵��: ��SPI���߷���16��bit����   ���Ϳ�����
*	��    ��: _cmd : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD9833_Write_16Bits(uint16_t _cmd)
{
	uint8_t i;
	SCLK_1();
	FSYNC_0();
	
	/*��AD9833  SCLKʱ�Ӹߴ�40M����˿��Բ��ӳ� */
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
*	��������AD9833_SelectWave
*	����˵�������λ����

*	IOUT���Ҳ� ��SIGNBITOUT���� ��дFREQREG0 ��дPHASE0
*	ad9833_write_16bit(0x2028) һ����дFREQREG0
*	ad9833_write_16bit(0x0038) ������дFREQREG0��LSB
*	ad9833_write_16bit(0x1038) ������дFREQREG0��MSB
 
*	IOUT���ǲ� ��дPHASE0
*	ad9833_write_16bit(0x2002)һ����дFREQREG0
*	ad9833_write_16bit(0x0002)������дFREQREG0��LSB
*	ad9833_write_16bit(0x1008)������дFREQREG0��MSB

*   �βΣ�_Type -- �������� 
*   ����ֵ ����

*********************************************************************************************************
*/
void AD9833_SelectWave(uint8_t _Type) 
{
	FSYNC_1();  //�궨��
	SCLK_1();
	if(_Type == 0)
	{
		AD9833_Write_16Bits(0x2028); /*Ƶ�ʼĴ����������*/
	}
	else if(_Type == 1)
	{
		AD9833_Write_16Bits(0x2002); /*Ƶ�ʼĴ���������ǲ�*/
	}
	else if(_Type == 2)
	{
		AD9833_Write_16Bits(0x2000); /*Ƶ�ʼĴ���������Ҳ�*/
	}
	else if(_Type == 3)
	{
		AD9833_Write_16Bits(0x00C0); /*�����*/
	}
 

}

/****************************************************************
��������: AD9833_SetFreq
����: ����Ƶ��ֵ   
����: _freq
freq -- Ƶ��ֵ (Freq_value(value)=Freq_data(data)*FCLK/2^28) 
����ֵ: ��
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
		lsb_14bit &= ~(1U<<15);//0111 1111 1111 1111 �Ȱѵ�15λ��0,����λ����
		lsb_14bit |= 1<<14;    //0100 0000 0000 0000 �ٰѵ�14λ��1,����λ���� �������01xx xxxx xxxx xxxx
		msb_14bit &= ~(1U<<15); //ͬ��
		msb_14bit |= 1<<14;
	}
	else
	{
		lsb_14bit &= ~(1<<14); //1011 1111 1111 1111 �Ȱѵ�14λ��0,����λ����
		lsb_14bit |= 1U<<15;   //1000 0000 0000 0000 �ٰѵ�15λ��1,����λ���� �������10xx xxxx xxxx xxxx
		msb_14bit &= ~(1<<14); //ͬ��
		msb_14bit |= 1U<<15;
	}

	AD9833_Write_16Bits(lsb_14bit);
	AD9833_Write_16Bits(msb_14bit);
}