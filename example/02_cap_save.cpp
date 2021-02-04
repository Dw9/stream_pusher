#include <iostream>
#include <signal.h>
#include "CapSndPcmSave.h"
#include "AACEncoderT.h"

int main(int argc, char *argv[])
{
    //设置需要采集pcm参数
    unsigned long   nSampleRate = 32000;
    unsigned int    nChannels	= 2;
    unsigned int    nPCMBitSize = 16;
    //采集设备 arecord -l card 2: sndrpii2scard [snd_rpi_i2s_card], device 0
	const char* device = "plughw:2,0";

    CapSndPcmSave capHandle(nSampleRate);
	capHandle.InitCap(device); //初始化采集pcm类

    AACEncoderT act(capHandle.FIFO4Save,nSampleRate,nChannels,nPCMBitSize);
    act.start(); //初始化编码类，等待采集数据
	
    capHandle.CapPcmData();	 //开始采集数据

    
	return 0;
}
