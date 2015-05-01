/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "RdsService"
#include "utils/Log.h"

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include "com_android_server_Rds.h"

//#define DEBUG
#ifdef DEBUG
#define LOGV(fmt, args...) ALOGV(fmt, ##args)
#define LOGD(fmt, args...) ALOGD(fmt, ##args)
#define LOGI(fmt, args...) ALOGI(fmt, ##args)
#define LOGE(fmt, args...) ALOGE(fmt, ##args)
#define LOGW(fmt, args...) ALOGW(fmt, ##args)
#else
#define LOGV(fmt, args...)
#define LOGD(fmt, args...)
#define LOGI(fmt, args...)
#define LOGE(fmt, args...)
#define LOGW(fmt, args...)
#endif
#define TEST_RDS_DATA_ONLY

namespace android {

typedef unsigned int RDS_BLOCK;
	
// MACRO definition
#define DEV_MAJOR				237
#define RDS_IOCTL_START_DATA	_IO(DEV_MAJOR, 0)		// request rds data
#define RDS_IOCTL_STOP_DATA		_IO(DEV_MAJOR, 1)		// stop transfer rds data to android
#define RDS_DEV     			"/dev/bonovo_rds"  
#define FILE_PATH				"/mnt/internal_sd/"
#define RDS_BUF_SIZE 			4096	// it contains RDS raw data from char driver
#define RDS_BLOCK_BITSIZE  26

// globle variable definition
int rds_fd = -1;
int flag_save_data = false;
int rds_onoff = 0;
static char file_name[128];
RDS_BLOCK g_BlockA = 0;
RDS_BLOCK g_BlockB = 0;
RDS_BLOCK g_BlockC = 0;
RDS_BLOCK g_BlockD = 0;
unsigned int g_nPI = 0, g_lastPI = 0;

static JavaVM *gJavaVM;


struct rds_buf_t
{
	unsigned char buf[RDS_BUF_SIZE];
	int w_idx;			// indicate the buffer that is written now
	int r_idx;			// indicate the buffer that is read now
	int valid_data_num;	// indicate how many valid data in the buffer
};

#if (0)
// 解析RDS数据
int RDSParse(unsigned char *pData, int size)
{
	if(pData == NULL)
		return -1;
	
	if (size != 16)
		return -1;

	memcpy(&g_BlockA,pData,4);
	memcpy(&g_BlockB,pData+4,4);
	memcpy(&g_BlockC,pData+8,4);
	memcpy(&g_BlockD,pData+12,4);
	//dbgprt("BlockA:0x%07x,BlockB:0x%07x,BlockC:0x%07x,BlockD:0x%07x\n",g_BlockA,g_BlockB,g_BlockC,g_BlockD);

	RDS_PI pi;
	pi.countryCode = g_BlockA>>(RDS_BLOCK_BITSIZE-4);
	pi.areaCode = (g_BlockA>>(RDS_BLOCK_BITSIZE-8))&0x0F;
	pi.referNum = (g_BlockA>>10)&0xFF;
	g_nPI = (g_BlockA>>10)&0xFFFF;
	dbgprt("PI:%d, countryCode:%X, areaCode:%X,referNum;%X\n",g_nPI,pi.countryCode,pi.areaCode,pi.referNum);
	SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_PI,&pi,sizeof(pi));

	RDS_TP tp = (RDS_TP)((g_BlockB>>(RDS_BLOCK_BITSIZE-6))&0x01);
    SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_TP,&tp,sizeof(tp));

	RDS_PTY pty;
	pty.type = RDS_PTY_T_EUP;
	pty.ptyEUP = (RDS_PTY_EUP)((g_BlockB>>15)&0x1F);
	SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_PTY,&pty,sizeof(pty));
	
	dbgprt("TP:%d,PTY:%d\n",tp,pty.ptyEUP);

	RDS_GROUP_TYPE groupType= (RDS_GROUP_TYPE)(g_BlockB>>(RDS_BLOCK_BITSIZE-5));

	switch(groupType)
	{
	case GROUP_0A:
	case GROUP_0B:
		{
			// Block B
			RDS_TA ta = (g_BlockB>>14)&0x01;
			RDS_MS ms = (g_BlockB>>13)&0x01;
			RDS_DI di = (g_BlockB>>12)&0x01;
			BYTE c1c0 = (g_BlockB>>10)&0x03;
			SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_TA,&ta,sizeof(ta));
			SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_DI,&di,sizeof(di));

			static RDS_AF af;
			static int count = 0;
			if(groupType == GROUP_0A)
			{
				// Block C
				unsigned char af1 = g_BlockC>>(RDS_BLOCK_BITSIZE-8);
				unsigned char af2 = (g_BlockC>>10)&0xFF;
				if(IsPIChanged()) // 如果电台发生变化,重新拼接AF
				{
					memset(&af,0,sizeof(af));
					count = 0;
				}
				if(af1==224) // 无替换频率存在
				{
					memset(&af,0,sizeof(af));
					count = 0;
					dbgprt("AF: not exist..\n");
				}
				else if(af1>=225 && af1<=249)
				{
					memset(&af,0,sizeof(af));
					af.bAFExist = TRUE;
					af.listCount = af1 - 224;
					af.freq[0]= 87500 + (af2*100);
					count = 1;
				}
				else
				{
					if(af1<=204)
						af.freq[count++]= 87500 + (af1*100);
					if(af2<=204)
						af.freq[count++]= 87500 + (af2*100);
				}
				if(af.listCount == count)
				{
					dbgprt("AF list[%d]:",af.listCount);
					for(UINT i =0;i<af.listCount;i++)
					{
						dbgprt("%d ",af.freq[i]);
					}
					dbgprt("\n");
					SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_AF,&af,sizeof(af));
					count = 0;
				}
			}
			else
			{
				memset(&af,0,sizeof(af));
				dbgprt("AF: not exist..\n");
				SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_AF,&af,sizeof(af));
			}
			// Block D
			static RDS_PS ps;
			static int nRevLevel = 0;
			if(IsPIChanged()) // 如果电台发生变化,重新拼接PS
			{
				memset(&ps,0,sizeof(ps));
				nRevLevel = 0;
			}
			ps.name[c1c0*2] = (g_BlockD>>18)&0xFF;
			ps.name[c1c0*2+1] = (g_BlockD>>10)&0xFF;
			if(c1c0 == nRevLevel)
			{
				nRevLevel++;
			}
            if(nRevLevel >= 4)
			{
				dbgprt("TA:%d,MS:%d,DI:%d,PS:%s\n",ta,ms,di,ps.name);
				SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_PS,&ps,sizeof(ps));
				nRevLevel = 0;
			}
		}
		break;
	case GROUP_1A:
	case GROUP_1B:
		{
			if(groupType == GROUP_1A)
			{
				// Block C
			}

			// Block D
			RDS_PIN pin;
			pin.day = g_BlockD>>(RDS_BLOCK_BITSIZE-5);
			pin.hour = (g_BlockD>>16)&0x1F;
			pin.minute = (g_BlockD>>10)&0x3F;

			dbgprt("PIN:%02d %02d:%02d\n",pin.day,pin.hour,pin.minute);
		}
		break;
	case GROUP_2A:
	case GROUP_2B:
		{
			static RDS_RT rt;
			static UINT flag = 0;
			static int nRevLevel = 0;
			if(((g_BlockB>>14)&0x01)!=flag ||
				IsPIChanged()) // 如果电台发生变化,重新拼接RT
			{
				memset(&rt,0,sizeof(rt));
				nRevLevel = 0;
			}
			flag = (g_BlockB>>14)&0x01;
			BYTE c3c2c1c0 = (g_BlockB>>10)&0x0F;

			if(groupType == GROUP_2A)
			{			
				rt.radioText[c3c2c1c0*4] = (g_BlockC>>18)&0xFF;
				rt.radioText[c3c2c1c0*4+1] = (g_BlockC>>10)&0xFF;
				rt.radioText[c3c2c1c0*4+2] = (g_BlockD>>18)&0xFF;
				rt.radioText[c3c2c1c0*4+3] = (g_BlockD>>10)&0xFF;
			}
			else
			{
				rt.radioText[c3c2c1c0*2] = (g_BlockD>>18)&0xFF;
				rt.radioText[c3c2c1c0*2+1] = (g_BlockD>>10)&0xFF;
			}
			if(c3c2c1c0 == nRevLevel)
			{
				nRevLevel++;
			}

			if(nRevLevel>0)
			{
				dbgprt("RT:%s\n",rt.radioText);
				SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_RT,&rt,sizeof(rt));
			}
		}
		break;
	case GROUP_3A: // Open data applications
	case GROUP_3B: // Open data applications
		break;
	case GROUP_4A:
		{
			int MJD = ((g_BlockB%0x03)<<15)+(g_BlockC>>11);
			RDS_CT ct;
			int y = (int)((MJD - 15078.2)/365.25);
			int m = (int)((MJD-14956.1-int(y*365.25))/30.6001);
			ct.day = MJD-14956-int(y*365.25)-int(m*30.6001);
			int k = (m ==14 || m==15) ? 1 : 0;
			ct.year = y+k+1900;
			ct.month = m-1-12*k;
			ct.hour = (((g_BlockC>>10)&0x01)<<4)+(g_BlockD>>22);
			ct.minute = (g_BlockD>>16)&0x3F;

			int timeOffset = ((g_BlockD>>10)&0x3F)*30;
			int minutes = ct.hour*60+ct.minute+timeOffset;
			if(minutes<0)
			{
				minutes = 24*60+minutes;
			}
			ct.hour = minutes/60;
			ct.minute = minutes%60;
			dbgprt("Local Time:%04d年%02d月%02d日 %02d:%02d\n",ct.year,ct.month,ct.day,ct.hour,ct.minute);
		}
		break;
	case GROUP_4B:// Open data applications
		break;
	case GROUP_5A:
		break;
	case GROUP_5B:
		break;
	case GROUP_6A:
		break;
	case GROUP_6B:
		break;
	case GROUP_7A:
		break;
	case GROUP_7B:// Open data applications 
		break;
	case GROUP_8A:
		break;
	case GROUP_8B:// Open data applications 
		break;
	case GROUP_9A:// Emergency Warning System
		break;
	case GROUP_9B:// Open data applications 
		break;
	case GROUP_10A:
		{
			static RDS_PTYN ptyn;
			UINT flag = 0;
			if((g_BlockB>>11)*0x07 != flag ||
				IsPIChanged())
			{
				memset(&ptyn,0,sizeof(ptyn));
			}
			flag = (g_BlockB>>11)*0x07;
			UINT c0 = (g_BlockB>>10)*0x01;
			g_BlockC <<=6;
			memcpy(&ptyn.name[c0],&g_BlockC,2);
			g_BlockD <<=6;
			memcpy(&ptyn.name[c0*2],&g_BlockD,2);
			dbgprt("PTYN:%s\n",ptyn.name);
			SendRDSMsg(MSGQUEUE_QUEUENAME_RADIO,RDS_INFO_PTYN,&ptyn,sizeof(ptyn));
		}
		break;
	case GROUP_10B:// Open data applications 
	case GROUP_11A:
	case GROUP_11B:
	case GROUP_12A:
	case GROUP_12B:
		break;
	case GROUP_13A:
		break;
	case GROUP_13B:// Open data applications 
		break;
	case GROUP_14A:
		dbgprt("Group ECON A\n");
		break;
	case GROUP_14B:
		dbgprt("Group ECON B\n");
		break;
	case GROUP_15A:
		break;
	case GROUP_15B:
		break;
	default:
		break;
	}
	g_lastPI = g_nPI;
	return 1;
}
#endif

void build_file_name(void)
{
	time_t now;
	struct tm *timenow; 
	int len;
	time(&now);
	timenow = localtime(&now);
	len = sprintf(file_name, "%s%04d%02d%02d-%02d%02d%02d",
		FILE_PATH, timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday,
		timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	ALOGE("========dzwei, %s ===========\r\n", file_name);
}

void *rds_thread_func(void *argv) 
{
	struct rds_buf_t raw_buf;
	int fd_rds_data_file = -1;
	int read_count;
	int write_count;
	int try_times;
	memset(&raw_buf, 0x00, sizeof(struct rds_buf_t));
	
	while(1)
	{
		if (!rds_onoff)
		{
			usleep(1000);
			continue;
		}

		//---------------------------------------------------------------------
		// process saving rds data to file command
		// if received start saving rds raw data to file, create a new file.
		if (fd_rds_data_file < 0 && flag_save_data == true)
		{
			build_file_name();
			fd_rds_data_file = open(file_name, O_RDWR|O_CREAT, S_IWUSR|S_IWGRP|S_IWOTH);
			
			if (fd_rds_data_file < 0)
			{
				ALOGE("Creat %s failed. error:%d(%s)", file_name, errno, strerror(errno));
			}
		}
		
		// if received stop saving rds data to file, close file, and reset file fd.
		if (fd_rds_data_file >= 0 && flag_save_data == false)
		{
			close(fd_rds_data_file);
			fd_rds_data_file = -1;
		}

#ifdef TEST_RDS_DATA_ONLY
		// read rds raw data from rds char dev
		read_count = read(rds_fd, raw_buf.buf, RDS_BUF_SIZE);
#else
		// read rds raw data from rds char dev
		read_count = read(rds_fd, raw_buf.buf+raw_buf.w_idx, RDS_BUF_SIZE-raw_buf.w_idx);
#endif
		// save the data to file
		if (fd_rds_data_file >= 0)
		{
			write_count = 0;
			try_times = 0;
			while (write_count < read_count)
			{
				write_count += write(fd_rds_data_file, raw_buf.buf+raw_buf.w_idx, read_count);
				//flush(fd_rds_data_file);
				try_times++;
				// if we have try so many times, we will give up to save data
				// the limit of try_times can use any value
				// here we select read_count as the upper limit
				if (try_times >= read_count)
				{
					ALOGE("Cannot save RDS data to file.");
					break;
				}
			}
		}
		//---------------------------------------------------------------------

		//---------------------------------------------------------------------
		// Here, we should parse RDS raw data		
	}
	return NULL;
}

struct RdsService {
    jobject   mRdsServiceObj;
    jmethodID mGetMemberRds;
};
static RdsService gRdsService;


static jboolean android_server_RdsService_native_start(JNIEnv* env, jobject obj)
{
	pthread_t rds_thread_id;
	int err = 0;
    gRdsService.mRdsServiceObj = env->NewGlobalRef(obj);
    env->GetJavaVM(&gJavaVM);
	
    rds_fd = open(RDS_DEV, O_RDWR|O_NOCTTY);
    if(rds_fd < 0){
        ALOGE("open %s failed. error:%d(%s)", RDS_DEV, errno, strerror(errno));
        return false;
    }

	err = pthread_create(&rds_thread_id, NULL, rds_thread_func, NULL); 
	if (err) {
		ALOGE("cant creat rds_thread_func \r\n");
		return false;
	}
	LOGD("rds native_start!!!");

	return true;
}

static jint android_server_RdsService_native_rds_onoff(
	JNIEnv* env, jobject obj, jint onoff)
{
	rds_onoff = onoff;
	return true;
}


static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
	{"native_start", "()Z", (void*)android_server_RdsService_native_start},
	{"native_setRdsOnoff", "(I)I", (void*)android_server_RdsService_native_rds_onoff},
};

int register_android_server_RdsService(JNIEnv* env)
{
	gRdsService.mRdsServiceObj = NULL;

    jclass clazz = env->FindClass("com/android/server/RdsService");
    if (clazz == NULL) {
        ALOGE("Can't find com/android/server/RdsService");
        return -1;
    }

    return jniRegisterNativeMethods(env, "com/android/server/RdsService", sMethods, NELEM(sMethods));
}

} /* namespace android */


