#include <stdint.h>
#include <iostream>
class AACCycleRecord
{
public:

	virtual void InitAACHeader();
	virtual int  WriteAACHeader( int PacketSize);
    virtual int  WriteAACFile(unsigned char* pbAACBuffer,int buffSize);

    virtual unsigned long long GetDiskSize(const char* path);
    virtual uint64_t hidev_gettime_inms();
    void getCurrTime(std::string& str_time);

    virtual int InitFile();
    virtual void EndFile();

    


	AACCycleRecord();
	virtual ~AACCycleRecord();

public:
    FILE* fout_aac;
    int64_t createTime; //  创建文件时间
    bool isFirst;
    int64_t  recordTimeS;     //aac file time(s)
private:

    char aac_adts_header[7];  //adts hearder
    int  chanCfg;   //MPEG-4 Audio Channel Configuration. 1 Channel front-center
    int  profile;   //AAC LC
    int  freqIdx;  //32000HZ 
    int  leftSpace; // 小于此内存后开始循环录制


};
