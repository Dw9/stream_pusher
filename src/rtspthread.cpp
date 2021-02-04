
#include "rtspthread.h"

extern "C" void rtsp_push_test(const char* host, const char* file,void* fifo,int sample,int channel);
extern "C" int rtsp_push_sendrtp_stop(void);


RtspThread::RtspThread(struct kfifo* infifo, string host, int port, string app, string streamid, int sample, int channel)
:_is_runing(false), inFIFO(infifo), _host(host), _port(port), _app(app), _streamid(streamid),
     _sample(sample), _channel(channel)
{
}

RtspThread::~RtspThread()
{
    rtsp_push_sendrtp_stop();
    _is_runing = false;
    join();
}

void* RtspThread::run()
{
    _is_runing = true;
    // 此函数里面有while循环 会阻塞在这里
    rtsp_push_test(_host.c_str(),(_app+"/"+_streamid).c_str(),(void*)inFIFO, _sample, _channel);
    return NULL;
}