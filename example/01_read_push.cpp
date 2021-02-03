
#include <iostream>

extern "C" void rtsp_push_test(const char* host, const char* file,const char* filename,int sample,int channel);



int main(void){


    const char* filename = "/home/vroot/rootfs/mnt/media/8k.wav";
    rtsp_push_test("81.70.196.111", "live/603",filename,8000,1);


}