# ifndef __AD9833_H
#define __AD9833_H


void AD9833_Init(void);
static void AD9833_Delay(void);
void AD9833_WaveSetting(double Freq,unsigned int Freq_SFR,unsigned int WaveMode,unsigned int Phase );


#endif


