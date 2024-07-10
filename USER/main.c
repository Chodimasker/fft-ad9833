#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "usmart.h"
#include "adc.h"
#include "stm32_dsp.h"
#include "AD9833.h"
#include "math.h"
#include "stdlib.h"
#include "AD98332.h"
#include "stdbool.h"
#include "key.h"

#define TRI_WAVE 	0  		//三角波
#define SIN_WAVE 	1		//正弦波
#define SQU_WAVE 	2		//方波

	#define NPT 1024
	uint32_t adc_buf[NPT]={0};
	long lBufInArray[NPT];
	long lBufOutArray[NPT/2];
	long lBufMagArray[NPT/2];

	long mag_ref = 150;
	long mag_save[2];
	
void SaveMag(long mag)
{
	if(mag_save[0] == NULL)
	{
		mag_save[0] = mag;
	}
	else
	{
		mag_save[1] = mag;
	}
}
	
void GetPowerMag()
{
    signed short lX,lY;
    float X,Y,Mag;
    unsigned short i;
    for(i=0; i<NPT/2; i++)
    {
        lX  = (lBufOutArray[i] << 16) >> 16;
        lY  = (lBufOutArray[i] >> 16);
        
        //除以32768后乘65536是符合浮点计算规律
        X = NPT * ((float)lX) / 32768;
        Y = NPT * ((float)lY) / 32768;
        Mag = sqrt(X * X + Y * Y) / NPT;
        if(i == 0)
            lBufMagArray[i] = (unsigned long)(Mag * 32768);
        else
            lBufMagArray[i] = (unsigned long)(Mag * 65536);
    }
		//OUTPUT_MAG[0] = OUTPUT_MAG[0]/2;	//直流分量不需要乘以2
}


int main(void)
{ 
	//FFT
	uint32_t i;

	//AD9833
	AD9833_Init();
	AD9833_Init2();

    HAL_Init();                    	 	//锟斤拷始锟斤拷HAL锟斤拷    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//锟斤拷锟斤拷时锟斤拷,72M
	delay_init(72);               		//锟斤拷始锟斤拷锟斤拷时锟斤拷锟斤拷
	uart_init(115200);					//锟斤拷始锟斤拷锟斤拷锟斤拷
	usmart_dev.init(84); 		  	  	//其实可以不用usmart
	LED_Init();							
 	LCD_Init();						
	MY_ADC_Init();                 

	LCD_Clear(BLACK);
	POINT_COLOR = BLUE;
	
	KEY_Init();
	u8 key = 0;
	u8 type = 3 ;
	u8 KEY_FLAG = 1;
	/*
	LCD_ShowString(90,80,180,16,16,"WELCOME");
	LCD_ShowString(60,96,180,16,16,"DEFAULT:WKUP-SS");
	LCD_ShowString(60,112,180,16,16,"KEY0-ST,KEY1-TT");
*/
		while(KEY_FLAG)
	{
		key=KEY_Scan(0);				//得到键值
			switch(key)
			{				 
				case KEY0_PRES:
					type = 1;
					KEY_FLAG = 0;
					break;
				case KEY1_PRES:
					type = 2;
					KEY_FLAG = 0;
					break;
				case WKUP_PRES:				
					type = 3;
					KEY_FLAG = 0;
					break;
				default:
					delay_ms(10);	
			} 
	}
	
		//AD9833_WaveSetting(50000,0,SIN_WAVE,0);
		//AD9833_WaveSetting2(100000,0,SIN_WAVE,0);

		delay_ms(100);
	
				for(i=0;i<NPT;i++)
		{
			adc_buf[i]=Get_Adc(1);	//12000/1024=11.71875k
		}	

		for(i=0;i<NPT;i++)
		{
			//delay_ms(1);	
			lBufInArray[i] = ((signed short)(adc_buf[i]-2048)) << 16;		
			//前端加入直流偏置后软件需要减去2048，达到测量负周期的目的（需要看具体配置
		}
		
			cr4_fft_1024_stm32(lBufOutArray, lBufInArray, NPT);

		//adc的采样频率为292kHz，实际频率为f= k * 292kHz /1024，其中k为lBufOut的下标i
		
		GetPowerMag();
		
		for(i=0;i<NPT/2;i++)
		{
			if(lBufMagArray[i] > 20)
			{
				printf("lBufMagArray[%d] = %ld \r\n",i ,lBufMagArray[i]);
			}
		}

				//频率映射表
		for(i=0;i<NPT/2;i++)
		{
			if(lBufMagArray[i] > mag_ref)
			{
				if(203 <= i && i <= 213)
				{
					++i;
					SaveMag(20000);
				}
				else if (306 < i && i <= 316)	
				{
					++i;
					SaveMag(30000);
				}
				else if (409 <= i && i <= 420) 
				{
						++i;
						SaveMag(40000);
				}
				else if (502 <= i && i <= 512)
				{
					++i;
					SaveMag(50000);
				}
				else if (397 <= i && i < 409)
				{
					++i;
					SaveMag(60000);
				}
				else if (288 <= i && i <= 304)
				{
					++i;
					SaveMag(70000);
				}
				else if ( 190 <= i && i < 203)
				{
					++i;
					SaveMag(80000);
				}
				else if (81 <= i && i <= 100)
				{
					++i;
					SaveMag(90000);
				}
				else if ( 0 < i && i <= 22)
				{
					++i;
					SaveMag(100000);
				}
				
				else if (  253 <= i && i <= 263)
				{
					++i;
					SaveMag(25000);
				}
				else if ( 358 <= i && i <= 366)
				{
					++i;
					SaveMag(35000);
				}
				else if ( 460 < i && i <= 470)
				{
					++i;
					SaveMag(45000);
				}
				else if ( 452 <= i && i <= 460)
				{
					++i;
					SaveMag(55000);
				}
				else if ( 344 <= i && i <= 362)
				{
					++i;
					SaveMag(65000);
				}
				else if ( 244 <= i && i <= 254)
				{
					++i;
					SaveMag(75000);
				}
				else if ( 134 <= i && i <= 150)
				{
					++i;
					SaveMag(85000);
				}
				else if ( 25 <= i && i <= 45)
				{
					++i;
					SaveMag(95000);
				}				
				
				else if ( 49 <= i && i <= 57)
				{
					++i;
					SaveMag(5000);
				}
				else if ( 100 <= i && i <= 108)
				{
					++i;
					SaveMag(10000);
				}
				else if ( 151 <= i && i <= 158)
				{
					++i;
					SaveMag(15000);
				}
			}	
		}	//频率映射
		
		for(i=0;i<2;i++)
		{
			printf("MagSave[%d]= %ld \r\n",i,mag_save[i]);
		}
		
		printf("done");

		/*
		for(i=0;i<NPT;i++)
		{
				printf("lBufInArray[%d] = %ld \r\n",i ,lBufInArray[i]);
		}
		for(i=0;i<NPT;i++)
		{
				printf("lBufOutArray[%d] = %ld \r\n",i ,lBufOutArray[i]);
		}
		*/
				if(type == 1)
		{
			AD9833_WaveSetting(mag_save[0],0,SIN_WAVE,0);
			AD9833_WaveSetting2(mag_save[1],0,TRI_WAVE,0);
		}
		else if(type == 2)
		{
			AD9833_WaveSetting(mag_save[0],0,TRI_WAVE,0);
			AD9833_WaveSetting2(mag_save[1],0,TRI_WAVE,0);
		}			
		else
		{
			AD9833_WaveSetting(mag_save[0],0,SIN_WAVE,0);
			AD9833_WaveSetting2(mag_save[1],0,SIN_WAVE,0);
		}
		
    while(1)
	{
		

		

		
		//-----------AD9833------------
		//看.c文件对应的参数输入即可
		//第一个是频率
		//有两个状态寄存器0和1，第二个参数就是写入的寄存器
		//第三个是输出波，第四个是相位调整
		// SDATA B12, SCLK B13, FSYNC B14
		
		/*
		for(i=0;i<NPT/2;i++)
		{
			if(lBufMagArray[i] > mag_ref)
			{
				uint32_t freq = i * 1.2 * 1000000 / NPT;
				AD9833_WaveSetting(freq,0,SIN_WAVE,0);
				delay_ms(3000);
			}
		}
		
		for(i=20000;i<=100000;i+=1000)
		{
			AD9833_WaveSetting(i,0,SIN_WAVE,0);
			if(i>=100000)
			{
				i=20000;
				break;
			}
			delay_ms(10);
		}
		*/

		
	}
}

