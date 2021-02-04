#ifndef __AACENCODERT_H
#define __AACENCODERT_H
#include "thread.h"

class AACEncoderT: public Thread
{

public:
    void *_hEncoder = nullptr;
    unsigned long _ulInputSamples = 0;
    unsigned long _ulMaxInputBytes = 0;
    unsigned long _ulMaxOutputBytes = 0;

private:
    struct kfifo *pcmFIFO;
    unsigned long   nSampleRate = 32000;
    unsigned int    nChannels	= 2;
    unsigned int    nPCMBitSize = 16;


public:
    AACEncoderT(struct kfifo* infifo,unsigned long nSampleRate,unsigned int nChannels,unsigned int  nPCMBitSize);
    virtual ~AACEncoderT(void);
    bool Init(int iSampleRate, int iAudioChannel, int iAudioSampleBit);
    int EncData(int32_t* intputBuffer,unsigned int intLen,unsigned char* outputBuffer);

private:
    void *run();
};


#endif
