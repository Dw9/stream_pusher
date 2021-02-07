#include <iostream>
#include <signal.h>
#include "CapSndPcmSave.h"
#include "rtspthread.h"
#include "AACEncoderT.h"

#include <stdio.h>
char device[2048] = {0};
const char* device_format = "plughw:%d,%d";



static void cap_device_list(void)
{
    snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	snd_ctl_t *handle;
	int card, err, dev, idx;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);

	card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		printf("no soundcards found...");
		return;
	}
	printf("**** List of %s Hardware Devices ****\n",
	       snd_pcm_stream_name(stream));
	while (card >= 0) {
		char name[32];
		sprintf(name, "hw:%d", card);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			printf("control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			printf("control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}
		dev = -1;
		while (1) {
			unsigned int count;
			if (snd_ctl_pcm_next_device(handle, &dev)<0)
				printf("snd_ctl_pcm_next_device");
			if (dev < 0)
				break;
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				if (err != -ENOENT)
					printf("control digital audio info (%i): %s", card, snd_strerror(err));
				continue;
			}
            snprintf(device,sizeof(device),device_format,card,dev);
            std::cout<<device<<endl;
			printf("card %i: %s [%s], device %i: %s [%s]\n",
				card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
				dev,
				snd_pcm_info_get_id(pcminfo),
				snd_pcm_info_get_name(pcminfo));
			count = snd_pcm_info_get_subdevices_count(pcminfo);
			printf("  Subdevices: %i/%i\n",snd_pcm_info_get_subdevices_avail(pcminfo), count);
			for (idx = 0; idx < (int)count; idx++) {
				snd_pcm_info_set_subdevice(pcminfo, idx);
				if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
					printf("control digital audio playback info (%i): %s", card, snd_strerror(err));
				} else {
					printf("  Subdevice #%i: %s\n",
						idx, snd_pcm_info_get_subdevice_name(pcminfo));
				}
			}
		}
		snd_ctl_close(handle);
	next_card:
		if (snd_card_next(&card) < 0) {
			printf("snd_card_next");
			break;
		}
	}

}


int main(int argc, char *argv[])
{

    cap_device_list();

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
