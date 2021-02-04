#ifndef __CapSndPcmSAVE_H
#define __CapSndPcmSAVE_H
#include <alsa/asoundlib.h>
#include "kfifo.h"
#include "faac.h"

#define BUFSIZE 4096
class CapSndPcmSave{

public:
    struct kfifo *FIFO4Save;
    struct kfifo *FIFO4Push;
public:
    virtual int   InitCap(const char* device_name);
    virtual void  CapPcmData();
    virtual void  CloseDevice();
    virtual void  SetFlag();
    CapSndPcmSave(unsigned long nSampleRate);
    virtual ~CapSndPcmSave();


private:
    snd_pcm_t *capture_handle = nullptr;
    unsigned char buf[BUFSIZE];
    unsigned int  format = 0;
    bool is_stop = false;
    pthread_mutex_t _fifo_lock_s;
    pthread_mutex_t _fifo_lock_p;
    unsigned int mSampleRate; //采样频率

private:
    virtual int open_stream(snd_pcm_t **handle, const char *name, int dir);

};
#endif //