# fft-ad9833
使用adc采样信号，官方fft库计算频率，ad9833产生相应频率的自定义波
使用正点原子Hal库，目标上限频率为100kHz，采样频率在Get_Adc函数中调整采样时间为28.5cycle+12.5cycle -> 292kHz
