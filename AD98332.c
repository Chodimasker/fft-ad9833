#include "stm32f1xx_hal.h"
#include "sys.h"
//IO??
#define AD9833_SDATA  PBout(6) 	//SDATA
#define AD9833_SCLK   PBout(7) 	//SCLK
#define AD9833_FSYNC  PBout(8)  //FSYNC

#define SDATA_PIN GPIO_PIN_6
#define SCLK_PIN GPIO_PIN_7
#define FSYNC_PIN GPIO_PIN_8

/*
*********************************************************************************************************
*	? ? ?: AD9833_Init
*	????: ???AD9833(���������ŵ�����)
*	?    ?: ?
*	? ? ?: ?
*********************************************************************************************************
*/

// SDATA B6, SCLK B7, FSYNC B8
void AD9833_Init2(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();  
    
    GPIO_Initure.Pin=SDATA_PIN|SCLK_PIN|FSYNC_PIN;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //????
    GPIO_Initure.Pull=GPIO_PULLUP;          //??
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//??
   
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	HAL_GPIO_WritePin(GPIOB,SDATA_PIN,GPIO_PIN_RESET);	//PD1?0
	HAL_GPIO_WritePin(GPIOB,SCLK_PIN,GPIO_PIN_RESET);	//PD3?0
	HAL_GPIO_WritePin(GPIOB,FSYNC_PIN,GPIO_PIN_RESET);	//PD5?0
}

/*
*********************************************************************************************************
*	�� �� ��: AD9833_Delay
*	����˵��: ʱ����ʱ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AD9833_Delay2(void)
{
	uint16_t i;
	for (i = 0; i < 1; i++);
}

/*
*********************************************************************************************************
*	�� �� ��: AD9833_Write
*	����˵��: ��SPI���߷���16��bit����
*	��    ��: TxData : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD9833_Write2(unsigned int TxData)
{
	int i;
	AD9833_SCLK=1;
	AD9833_FSYNC=1;
	AD9833_FSYNC=0;
	//?16???
	for(i=0;i<16;i++)
	{
		if (TxData & 0x8000)
			AD9833_SDATA=1;
		else
			AD9833_SDATA=0;
		AD9833_Delay2();
		AD9833_SCLK=0;
		AD9833_Delay2();
		AD9833_SCLK=1;
		TxData<<=1;
	}
	AD9833_FSYNC=1;
}

#define TRI_WAVE 	0  		//���ǲ�
#define SIN_WAVE 	1		//���Ҳ�
#define SQU_WAVE 	2		//����


/*
*********************************************************************************************************
*	�� �� ��: AD9833_AmpSet
*	����˵��: �ı�����źŷ���ֵ
*	��    ��: 1.amp ������ֵ  0- 255
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 

/*  ***********��Ϊû��CS���ţ��������ò��˷�ֵ
void AD9833_AmpSet(unsigned char amp)
{
	unsigned char i;
	unsigned int temp;
   	
	CS_0();
	temp =0x1100|amp;
	for(i=0;i<16;i++)
	{
		AD9833_SCLK=0;
	   if(temp&0x8000)
			AD9833_SDATA=1;
	   else
			AD9833_SDATA=0;
		temp<<=1;
		AD9833_SCLK=1;
	    AD9833_Delay();
	}
	
   	CS_1();
}

*/

/*
*********************************************************************************************************
*	�� �� ��: AD9833_WaveSeting
*	����˵��: ��SPI���߷���16��bit����
*	��    ��: 1.Freq: Ƶ��ֵ, 0.1 hz - 12Mhz
			  2.Freq_SFR: 0 �� 1
			  3.WaveMode: TRI_WAVE(���ǲ�),SIN_WAVE(���Ҳ�),SQU_WAVE(����)
			  4.Phase : ���εĳ���λ
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 

void AD9833_WaveSetting2(double Freq,unsigned int Freq_SFR,unsigned int WaveMode,unsigned int Phase )
{

		int frequence_LSB,frequence_MSB,Phs_data;
		double   frequence_mid,frequence_DATA;
		long int frequence_hex;

		/*********************************����Ƶ�ʵ�16����λ***********************************/
		frequence_mid=268435456/25;//??25M??
		//���ʱ��Ƶ�ʲ�Ϊ25Hz,�޸ĸô���Ƶ��ֵ����λMHz ,AD9833���֧��25MHz
		frequence_DATA=Freq;
		frequence_DATA=frequence_DATA/1000000;
		frequence_DATA=frequence_DATA*frequence_mid;
		frequence_hex=frequence_DATA;  //??frequence_hex��һ��32λ�ܴ����,��Ҫ��ֳɳ�����14λ���д���;
		frequence_LSB=frequence_hex; //frequence_hex��16λ�͸�frequence_LSB
		frequence_LSB=frequence_LSB&0x3fff;//ȥ�����2λ,16���14λ
		frequence_MSB=frequence_hex>>14; //frequence_hex��16�͸�frequence_HSB
		frequence_MSB=frequence_MSB&0x3fff;//16��14

		Phs_data=Phase|0xC000;	//��λֵ
		AD9833_Write2(0x0100); //��λAD9833,��RESETλΪ1
		AD9833_Write2(0x2100); //ѡ������һ��д��,B28λ��RESETλΪ1

		if(Freq_SFR==0)				  //���������õ�����Ƶ�ʼĴ���0
		{
		 	frequence_LSB=frequence_LSB|0x4000;
		 	frequence_MSB=frequence_MSB|0x4000;
			 //ʹ��Ƶ�ʼĴ���0�������s
			AD9833_Write2(frequence_LSB); //L14ѡ��Ƶ�ʼĴ���0�ĵ�14��������
			AD9833_Write2(frequence_MSB); //H14 Ƶ�ʼĴ����ĸ�14λ��������
			AD9833_Write2(Phs_data);	//������λ
			//AD9833_Write(0x2000); /**����FSELECT=0оƬ���빤��״̬��Ƶ�ʼĴ���0�������**/
	    }
		if(Freq_SFR==1)				//���������õ�Ƶ�ʼĴ���1
		{
			 frequence_LSB=frequence_LSB|0x8000;
			 frequence_MSB=frequence_MSB|0x8000;
			//ʹ��1
			AD9833_Write2(frequence_LSB); 
			AD9833_Write2(frequence_MSB);
			AD9833_Write2(Phs_data);	
			//AD9833_Write(0x2800); /**����FSELECTλΪ0������FSELECTλΪ1����ʹ��Ƶ�ʼĴ���1��ֵ��оƬ���빤��״̬,Ƶ�ʼĴ���1�������**/ 
		}

		if(WaveMode==TRI_WAVE) //������ǲ�
		 	AD9833_Write2(0x2002); 
		if(WaveMode==SQU_WAVE)	//�������
			AD9833_Write2(0x2028); 
		if(WaveMode==SIN_WAVE)	//�������
			AD9833_Write2(0x2000); 

}



