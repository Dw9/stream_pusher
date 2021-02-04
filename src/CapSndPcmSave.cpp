#include "CapSndPcmSave.h"
#include <errno.h>
#include <stdlib.h>
#include "AACCycleRecord.h"

bool lostDevice = false; //掉线检测


int CapSndPcmSave::open_stream(snd_pcm_t **handle, const char *name, int dir)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	const char *dirname = (dir == SND_PCM_STREAM_PLAYBACK) ? "PLAYBACK" : "CAPTURE";
	int err;

	if ((err = snd_pcm_open(handle, name, (snd_pcm_stream_t)dir, 0)) < 0) {
		fprintf(stderr, "%s (%s): cannot open audio device (%s)\n", 
			name, dirname, snd_strerror(err));
		return err;
	}
	   
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot allocate hardware parameter structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
			 
	if ((err = snd_pcm_hw_params_any(*handle, hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot initialize hardware parameter structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_access(*handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "%s (%s): cannot set access type(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_format(*handle, hw_params, (snd_pcm_format_t)format)) < 0) {
		fprintf(stderr, "%s (%s): cannot set sample format(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &mSampleRate, NULL)) < 0) {
		fprintf(stderr, "%s (%s): cannot set sample mSampleRate(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_channels(*handle, hw_params, 2)) < 0) {
		fprintf(stderr, "%s (%s): cannot set channel count(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params(*handle, hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot set parameters(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	snd_pcm_hw_params_free(hw_params);

	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot allocate software parameters structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_current(*handle, sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot initialize software parameters structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_set_avail_min(*handle, sw_params, BUFSIZE)) < 0) {
		fprintf(stderr, "%s (%s): cannot set minimum available count(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_set_start_threshold(*handle, sw_params, 0U)) < 0) {
		fprintf(stderr, "%s (%s): cannot set start mode(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params(*handle, sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot set software parameters(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	return 0;
}

int CapSndPcmSave::InitCap(const char* device_name){

    int err = 0;
    // if ((err = open_stream(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK)) < 0)
	// 	return err;

	if ((err = open_stream(&capture_handle,device_name , SND_PCM_STREAM_CAPTURE)) < 0)
		return err;

	// if ((err = snd_pcm_prepare(playback_handle)) < 0) {
	// 	fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
	// 		 snd_strerror(err));
	// 	return err;
	// }
	
	if ((err = snd_pcm_start(capture_handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
			 snd_strerror(err));
		return err;
	}
    return 0;

}

void CapSndPcmSave::CapPcmData(){
    int avail = 0;
    int err = 0;
    int put_size = 0;
    int getLen = 0;
    int ret =0;
	int64_t total_read_filure = 0;

	while (1) {
		
		// if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
		// 	fprintf(stderr, "poll failed(%s)\n", strerror(errno));
		// 	std::cout<<"snd_pcm_wait error"<<std::endl;
		// 	continue;
		// }	           

		avail = snd_pcm_avail_update(capture_handle);
		if (avail > 0) {
			total_read_filure = 0;
			lostDevice = false;
			if (avail > BUFSIZE)
				avail = BUFSIZE;

			snd_pcm_readi(capture_handle, buf, avail);
            kfifo_put_chunk(FIFO4Push, buf, avail*4);
			kfifo_put_chunk(FIFO4Save, buf, avail*4);
	    } else {
			usleep(10);
			total_read_filure += 1;
			//std::cout<<"total_read_filure: "<<total_read_filure<<std::endl;
			if(total_read_filure > 10000 && lostDevice == false){  //设备掉线
				std::cout<<"capPcm: lost device  -----> wirteFile"<<std::endl;
				lostDevice = true; 
				usleep(1000);
			}
			

		}

        if(is_stop)
        {
            printf("stop cap.\n");
            break;
        }
    }
    printf("\n CLOSE:::::::::::\n");

}

void CapSndPcmSave::SetFlag()
{
    is_stop = true;
}

void CapSndPcmSave::CloseDevice(){

    //snd_pcm_close(playback_handle);
	snd_pcm_close(capture_handle);
}


CapSndPcmSave::CapSndPcmSave(unsigned long nSampleRate):
capture_handle(nullptr),
format(SND_PCM_FORMAT_S16_LE),
is_stop(false),
mSampleRate(32000)
{
    memset(buf,0,sizeof(buf));
    // 申请FIFO
    pthread_mutex_init(&_fifo_lock_s, NULL);
    FIFO4Save = kfifo_alloc(1024*1024*8, 0, &_fifo_lock_s);

	pthread_mutex_init(&_fifo_lock_p, NULL);
    FIFO4Push = kfifo_alloc(1024*1024*8, 0, &_fifo_lock_p);

}
CapSndPcmSave:: ~CapSndPcmSave(){

}