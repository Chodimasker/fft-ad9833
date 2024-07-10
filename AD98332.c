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
*	????: ???AD9833(相关输出引脚的配置)
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
*	函 数 名: AD9833_Delay
*	功能说明: 时钟延时
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD9833_Delay2(void)
{
	uint16_t i;
	for (i = 0; i < 1; i++);
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_Write
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: TxData : 数据
*	返 回 值: 无
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

#define TRI_WAVE 	0  		//三角波
#define SIN_WAVE 	1		//正弦波
#define SQU_WAVE 	2		//方波


/*
*********************************************************************************************************
*	函 数 名: AD9833_AmpSet
*	功能说明: 改变输出信号幅度值
*	形    参: 1.amp ：幅度值  0- 255
*	返 回 值: 无
*********************************************************************************************************
*/ 

/*  ***********因为没有CS引脚，所以设置不了幅值
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
*	函 数 名: AD9833_WaveSeting
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: 1.Freq: 频率值, 0.1 hz - 12Mhz
			  2.Freq_SFR: 0 或 1
			  3.WaveMode: TRI_WAVE(三角波),SIN_WAVE(正弦波),SQU_WAVE(方波)
			  4.Phase : 波形的初相位
*	返 回 值: 无
*********************************************************************************************************
*/ 

void AD9833_WaveSetting2(double Freq,unsigned int Freq_SFR,unsigned int WaveMode,unsigned int Phase )
{

		int frequence_LSB,frequence_MSB,Phs_data;
		double   frequence_mid,frequence_DATA;
		long int frequence_hex;

		/*********************************计算频率的16进制位***********************************/
		frequence_mid=268435456/25;//??25M??
		//如果时钟频率不为25Hz,修改该处的频率值，单位MHz ,AD9833最大支持25MHz
		frequence_DATA=Freq;
		frequence_DATA=frequence_DATA/1000000;
		frequence_DATA=frequence_DATA*frequence_mid;
		frequence_hex=frequence_DATA;  //??frequence_hex是一个32位很大的数,需要拆分成成两个14位进行处理;
		frequence_LSB=frequence_hex; //frequence_hex低16位送给frequence_LSB
		frequence_LSB=frequence_LSB&0x3fff;//去除最高2位,16变成14位
		frequence_MSB=frequence_hex>>14; //frequence_hex高16送给frequence_HSB
		frequence_MSB=frequence_MSB&0x3fff;//16变14

		Phs_data=Phase|0xC000;	//相位值
		AD9833_Write2(0x0100); //复位AD9833,即RESET位为1
		AD9833_Write2(0x2100); //选择数据一次写入,B28位和RESET位为1

		if(Freq_SFR==0)				  //吧数据设置到设置频率寄存器0
		{
		 	frequence_LSB=frequence_LSB|0x4000;
		 	frequence_MSB=frequence_MSB|0x4000;
			 //使用频率寄存器0输出数据s
			AD9833_Write2(frequence_LSB); //L14选择频率寄存器0的低14数据输入
			AD9833_Write2(frequence_MSB); //H14 频率寄存器的高14位数据输入
			AD9833_Write2(Phs_data);	//设置相位
			//AD9833_Write(0x2000); /**设置FSELECT=0芯片进入工作状态，频率寄存器0输出波形**/
	    }
		if(Freq_SFR==1)				//把数据设置到频率寄存器1
		{
			 frequence_LSB=frequence_LSB|0x8000;
			 frequence_MSB=frequence_MSB|0x8000;
			//使用1
			AD9833_Write2(frequence_LSB); 
			AD9833_Write2(frequence_MSB);
			AD9833_Write2(Phs_data);	
			//AD9833_Write(0x2800); /**设置FSELECT位为0，设置FSELECT位为1，即使用频率寄存器1的值，芯片进入工作状态,频率寄存器1输出波形**/ 
		}

		if(WaveMode==TRI_WAVE) //输出三角波
		 	AD9833_Write2(0x2002); 
		if(WaveMode==SQU_WAVE)	//输出方波
			AD9833_Write2(0x2028); 
		if(WaveMode==SIN_WAVE)	//输出正弦
			AD9833_Write2(0x2000); 

}



