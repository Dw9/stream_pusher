

#ifndef __RTSPTHREAD_H__
#define __RTSPTHREAD_H__

#include "thread.h"
#include "kfifo.h"

#include <string>

using namespace std;

class RtspThread: public Thread
{
    public:
        // rtsp://[host]:[port]/[app]/[streamid]
        RtspThread(struct kfifo* infifo, string host, int port, string app, string streamid, int sample, int channel);

        ~RtspThread();

        void *run();

    private:
        bool _is_runing;

        struct kfifo *inFIFO;

        string _host;
        int _port;
        string _app, _streamid;
        int _sample;
        int _channel;
};

#endif