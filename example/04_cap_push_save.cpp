#include <iostream>
#include <signal.h>
#include "CapSndPcmSave.h"
#include "rtspthread.h"
#include "AACEncoderT.h"


char device[2048] = {0};
const char* device_format = "plughw:%c,%c";

void checkDevice(){
    char device_num[2];

    FILE* fp;
    char pBuffer[128];
    fp = popen("arecord -l |grep card | awk  '{print $2}'","r"); 
    fgets(pBuffer,sizeof(pBuffer),fp);
    device_num[0] = pBuffer[0]; //card
    std::cout<<device_num[0]<<endl;

    memset(pBuffer,0,sizeof(pBuffer));
    fp = popen("arecord -l |grep device | awk  '{print $6}'","r"); 
    fgets(pBuffer,sizeof(pBuffer),fp);
    device_num[1] = pBuffer[0]; //device
    std::cout<<device_num[1]<<endl;

    snprintf(device,sizeof(device),device_format,device_num[0],device_num[1]);
    std::cout<<device<<endl;


}

int main(int argc, char *argv[])
{

    checkDevice();
    //设置需要采集pcm参数
    unsigned long   nSampleRate = 32000;
    unsigned int    nChannels	= 2;
    unsigned int    nPCMBitSize = 16;
    //采集设备 arecord -l card 2: sndrpii2scard [snd_rpi_i2s_card], device 0


    const char* rtspHost   = "frp.oopy.org";
    const char* rtspApp    = "live";
    const char* stream_id  = "603";

    CapSndPcmSave capHandle(nSampleRate);
    capHandle.InitCap(device); //初始化采集pcm类

    RtspThread rtsphanle(capHandle.FIFO4Push, rtspHost, 0, rtspApp, stream_id, nSampleRate, nChannels);
    rtsphanle.start();


    AACEncoderT act(capHandle.FIFO4Save,nSampleRate,nChannels,nPCMBitSize);
    act.start(); //初始化编码类，等待采集数据

    capHandle.CapPcmData();	 //开始采集数据

    
	return 0;
}
