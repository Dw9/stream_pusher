#include "AACCycleRecord.h"
#include <iostream>
#include <string.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <sys/statfs.h>

using namespace std;


#define INIFOLDERPATH "/home/pi/aac_file"
#define DSTORAGE  "/home/pi"  //树莓派home目录



void AACCycleRecord::EndFile(){

    fclose(fout_aac);
    fout_aac = NULL;
    isFirst = true;
    return;
}

int AACCycleRecord::InitFile(){

    if(NULL == opendir(INIFOLDERPATH)){        //文件夹是否存在
        if(mkdir(INIFOLDERPATH,0777) == -1){
            return -1;
        }
    }

    if(GetDiskSize(DSTORAGE)< leftSpace){ //检查磁盘空间
        //cout<<"no space disk left"<<endl;
        // char cmd[64];
        // sprintf(cmd,"cd %s && ls -tr | head -5 | xargs  -n1 rm -rf {} ",(char*)INIFOLDERPATH);
        // system((const char*)cmd);
        return -1;
    }    

    string fileName;   //创建文件，并打开
    getCurrTime(fileName);
    char filePath[64];
    sprintf(filePath,"%s/%s.aac",INIFOLDERPATH,(char*)fileName.c_str());
    fout_aac = fopen(filePath, "wb");
    if(fout_aac == NULL){      
        return -1;
    }
    return 0;
}

void AACCycleRecord::InitAACHeader(){


    aac_adts_header[0] = (char)0xFF;      // 11111111     = syncword
    aac_adts_header[1] = (char)0xF1;      // 1111 1 00 1  = syncword MPEG-2 Layer CRC
    aac_adts_header[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    aac_adts_header[6] = (char)0xFC;

}
int AACCycleRecord::WriteAACHeader(int PacketSize){

    aac_adts_header[3] = (char)(((chanCfg & 3) << 6) + ((7 + PacketSize) >> 11));
    aac_adts_header[4] = (char)(((7 + PacketSize) & 0x7FF) >> 3);
    aac_adts_header[5] = (char)((((7 + PacketSize) & 7) << 5) + 0x1F);

    return fwrite(aac_adts_header, 7, 1, fout_aac);

}
int AACCycleRecord::WriteAACFile(unsigned char* pbAACBuffer,int buffSize){

    if (pbAACBuffer==NULL) return -1;
    return fwrite(pbAACBuffer, 1, buffSize, fout_aac);

}

//return MB
unsigned long long AACCycleRecord::GetDiskSize(const char* path){

    struct statfs diskInfo;
    statfs(path,&diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数
	//unsigned long long totalsize = blocksize * diskInfo.f_blocks; 	//总的字节数，f_blocks为block的数目
	
	//unsigned long long freeDisk = diskInfo.f_bfree * blocksize;	//剩余空间的大小
	unsigned long long availableDisk = diskInfo.f_bavail * blocksize; 	//可用空间大小

    // cout<<"freeDisk  "<<freeDisk<<endl;
    // cout<<"availableDisk  "<<availableDisk<<endl;
	return availableDisk>>20; 

}

// 获取时间戳
uint64_t AACCycleRecord::hidev_gettime_inms()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec*1000 + now.tv_nsec/1000000);
}


void AACCycleRecord::getCurrTime(std::string& str_time)
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    //转为字符串
    std::stringstream ss;
    // 可以分别以不同的形式进行显示
    //	ss << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    //ss << std::put_time(std::localtime(&t), "%Y年%m月%d日%H时%M分%S秒");
    ss << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S");
    str_time = ss.str();
    cout << str_time << endl;
}

AACCycleRecord::AACCycleRecord() :
fout_aac(NULL),
createTime(0),
isFirst(true),
recordTimeS(1800),
chanCfg(1),
profile(2),
freqIdx(5),
leftSpace(100)
{
    memset(aac_adts_header,0,sizeof(aac_adts_header));
}


AACCycleRecord::~AACCycleRecord()
{
}
