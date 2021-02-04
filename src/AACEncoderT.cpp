
#include <unistd.h>
#include <cstdlib>
#include "AACEncoderT.h"
#include "AACCycleRecord.h"
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "kfifo.h"
#include <faac.h>
#ifdef __cplusplus
}
#endif

extern bool lostDevice; //掉线检测标志

AACEncoderT::AACEncoderT(struct kfifo* infifo,unsigned long nSampleRate,unsigned int nChannels,unsigned int  nPCMBitSize)
:_hEncoder(nullptr),_ulInputSamples(0),_ulMaxInputBytes(0),_ulMaxOutputBytes(0),pcmFIFO(infifo),
nSampleRate(nSampleRate),nChannels(nChannels),nPCMBitSize(nPCMBitSize)
{

}


AACEncoderT::~AACEncoderT() {
    if (_hEncoder != nullptr) {
        faacEncClose(_hEncoder);
        _hEncoder = nullptr;
    }
    join();
}

bool AACEncoderT::Init(int iSampleRate, int iChannels, int iSampleBit) {
    if (iSampleBit != 16) {
        return false;
    }
    // (1) Open FAAC engine
    _hEncoder = faacEncOpen(iSampleRate, iChannels, &_ulInputSamples,
            &_ulMaxOutputBytes);
    if (_hEncoder == NULL) {
        return false;
    }
    _ulMaxInputBytes = _ulInputSamples * iSampleBit / 8;

    // (2.1) Get current encoding configuration
    faacEncConfigurationPtr pConfiguration = faacEncGetCurrentConfiguration(_hEncoder);
    if (pConfiguration == NULL) {
        faacEncClose(_hEncoder);
        return false;
    }

    pConfiguration->aacObjectType = LOW;

    //设定AAC单通道比特率
    pConfiguration->bitRate   = 0;    // or 0 or 48000
    pConfiguration->bandWidth = 0;  //or 0 or 32000 or 64000

    /*下面可以选择设置*/
    pConfiguration->allowMidside = 1;
    pConfiguration->useLfe = 0;
    pConfiguration->useTns = 0;
    //AAC品质
    pConfiguration->quantqual = 100;
    //outputformat 0 = Raw; 1 = ADTS
    pConfiguration->outputFormat = 1;
    pConfiguration->shortctl = SHORTCTL_NORMAL;
    pConfiguration->inputFormat = FAAC_INPUT_16BIT;
    // 0 = Raw; 1 = ADTS
    pConfiguration->outputFormat = 1;
    pConfiguration->useTns = 0;
    // AAC object types
    pConfiguration->aacObjectType = LOW;
    pConfiguration->mpegVersion = MPEG4;


    // 重置编码器的配置信息
    faacEncSetConfiguration(_hEncoder, pConfiguration);

    // (2.2) Set encoding configuration
    if(!faacEncSetConfiguration(_hEncoder, pConfiguration)){
        faacEncClose(_hEncoder);
        return false;
    }
    return true;
}

int AACEncoderT::EncData(int32_t* intputBuffer,unsigned int intLen,unsigned char* outputBuffer) {

    return faacEncEncode(_hEncoder, intputBuffer, intLen, outputBuffer, _ulMaxOutputBytes);

}


void* AACEncoderT::run(){



    Init(nSampleRate,nChannels,nPCMBitSize);

    unsigned int     nPCMBufferSize = _ulMaxInputBytes;
    unsigned char*   pbPCMBuffer = new unsigned char[nPCMBufferSize];
    unsigned char*   pbAACBuffer = new unsigned char[_ulMaxOutputBytes];

    AACCycleRecord aacHandle;
	aacHandle.InitAACHeader();
    int ret = 0;
    int readLen = 0;
    int EncodeLen = 0;
    int writeFlag = 0;   // 负数 表示文件不可写  0可写 

    while(1){
        
        if(kfifo_len(pcmFIFO)> nPCMBufferSize)
        {
            
            memset(pbPCMBuffer,0,nPCMBufferSize);
            readLen = kfifo_get_chunk(pcmFIFO, pbPCMBuffer, nPCMBufferSize);
			if(readLen != nPCMBufferSize){
				std::cout<<"read error"<<std::endl;
				continue;
			}
            memset(pbAACBuffer,0,_ulMaxOutputBytes);
			// 编码
			EncodeLen = EncData((int32_t*)pbPCMBuffer,(unsigned int)(readLen/2),pbAACBuffer);
			if(EncodeLen)
            {  //编码失败不进入
                if(aacHandle.isFirst)  //文件初始，判断是否可写，创建文件写文件头
                {
                    //std::cout<<"ready to wirte header"<<std::endl;
                    aacHandle.createTime = aacHandle.hidev_gettime_inms();
                    writeFlag = aacHandle.InitFile();
                    if(writeFlag == 0){
                        aacHandle.WriteAACHeader(EncodeLen);
                        aacHandle.isFirst = false;
                        
                    }else{
                        std::cout<<"open file failed or mkdir failed or  no space left \n"<<std::endl;
                    }   
                }

                if(writeFlag<0) continue;
                ret = aacHandle.WriteAACFile(pbAACBuffer,EncodeLen);
                if(ret != EncodeLen){
                    std::cout<<"WriteAACFile Error"<<std::endl;
                }

                if((aacHandle.hidev_gettime_inms() - aacHandle.createTime)/1000.0 > aacHandle.recordTimeS ){ //按照预定时间，文件自动分段
                    aacHandle.EndFile();
                }
            
            }     
        
        }
        else{
            if((lostDevice == true) && (aacHandle.isFirst == false)){
                std::cout<<"wirteFile: lost device"<<std::endl;
                aacHandle.EndFile();
            }
            usleep(10);
        }

    }

    delete[] pbAACBuffer;
    delete[] pbPCMBuffer;

    printf("faac complete aac encode!!\n");
	pthread_exit(NULL);
}







