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

#define LOG_TAG "CanBusService"
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
#include "com_android_server_Canbus.h"

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

namespace android {


extern int process_canbus_command_volkaswagen(unsigned char *buf, int frame_len);
extern int process_canbus_command_sonata8(unsigned char *buf, int frame_len);
extern tSonataCarInfo sonata_car_info;


JavaVM *gJavaVM;
int canbus_fd = -1;
unsigned char canbus_frame_buf[CANBUS_FRAME_SIZE];
CAN_AC_INFO ac_info;
static void nativeReportAirCondition();
int updateAndReportRadar(void);
int updateAndReportAirConditon(void);
//int updateAndReportCarDoor(unsigned char * buf, int len);
int updateAndReportSonata8(tSonataCarInfo * sonata8Info, int len);

tCarType car_type;
tCanBoxVendor canbox_vendor = eCanBoxVendorBonovo;
tCarDoorInfo door_info;
tCarRadarInfo radar_info;

int calculate_frame_checksum(unsigned char *buf, int frame_len, unsigned char *checksum)
{
	int i;
	
	*checksum = 0;
	
	if (!buf || frame_len <= 0)
	{
        ALOGE("calculate_canbus_frame_checksum error\r\n");
		return -1;
	}
	for (i=1; i<frame_len - 1; i++)
	{
		*checksum += buf[i];
	}
	*checksum ^= 0xFF;
	
	return 0;
}


int process_canbus_command(unsigned char *buf, int frame_len)
{
	int ret_val = 0;
	switch (car_type)
	{
		case eCarTypeVolkaswagen:
			ret_val = process_canbus_command_volkaswagen(buf, frame_len);
			break;
		case eCarTypeSonata8:
			LOGD("myu eCarTypeSonata8!!!");
			ret_val = process_canbus_command_sonata8(buf, frame_len);
			break;
		default:
			break;
	}
	return ret_val;
}


// parse the frame data coming from canbus
int parse_canbus_frame(unsigned char *buf, int frame_len)
{
	unsigned char checksum_calculated;
	if (frame_len < 5)
	{
		return -1;
	}
	if (calculate_frame_checksum(buf, frame_len, &checksum_calculated))
	{
		return -2;
	}
	if (checksum_calculated != buf[frame_len-1])
	{
		return -3;
	}

	process_canbus_command(buf, frame_len);
	return 0;
}


//===================================================================
// void *canbus_thread_func(void *argv) 
//-------------------------------------------------------------------
// the main thread. 
// 1. send start command(some vehicle need to send start command before reading)
// 2. read data from canbus device
// 3. Judge if have receive one whole frame.
// 4. send Ack to canbox
//===================================================================
void *canbus_thread_func(void *argv) 
{
	//LOGD("myu canbus_thread_funcd !!!!");
	struct canbus_buf_t raw_buf;
	int count;
	int flag = 0;
	int data_len = 0;
	int frame_byte_idx = 0;
	unsigned char frame_head;
	unsigned char tmp_ch;
	unsigned char start_cmd_frame[5]=
	{
		0x2E, 0x81, 0x01, 0x01, 0x00
	};
	unsigned char canbus_ack = 0xFF;

	if (canbus_fd == -1)
	{
        ALOGE("could not open %s, errno:%d(%s)", CANBUS_DEV, errno, strerror(errno));
		return NULL;
	}
	
	memset(&raw_buf, 0x00, sizeof(struct canbus_buf_t));

	//===============================================================
	// 八代索纳塔第一个字节为0x0B表示开机
	// 只有发送了开机命令后,才能收到CAN 总线上的数据
	if (car_type == eCarTypeSonata8)
	{
		//LOGD("myu canbus_thread_func-->car_type = S8 ");
		start_cmd_frame[1] = 0x0B;
	}
	calculate_frame_checksum(start_cmd_frame, 5, &start_cmd_frame[4]);
	count = write(canbus_fd, start_cmd_frame, 5);
	//LOGD("myu canbus_thread_func-->write count = %d ",count);
	if (count < 5)
	{
        ALOGE("Can't start. Write %s failed, errno:%d(%s)", CANBUS_DEV, errno, strerror(errno));
	}
	//===============================================================
	frame_head = 0x2E;
	
	
	while (1)
	{
		// read data from canbus char device
		count = read(canbus_fd, raw_buf.buf+raw_buf.w_idx, CANBUS_BUF_SIZE-raw_buf.w_idx);
		raw_buf.w_idx += count;
#ifdef DEBUG
        // add by bonovo zbiao for debug
        LOGD("=== raw_buf.r_idx:%d  raw_buf.w_idx:%d  count:%d  buf:", raw_buf.r_idx, raw_buf.w_idx, count);
        for(int i=0; i<raw_buf.w_idx; i++){
            LOGD(" buf[%d]:0x%02X", i, raw_buf.buf[i]);
        }
#endif
		// process all the data has received
		while (raw_buf.r_idx < raw_buf.w_idx)
		{
			tmp_ch = raw_buf.buf[raw_buf.r_idx];
			raw_buf.r_idx++;
			
			if (flag == 0)
			{ 
				LOGD(" while(1)--->flag == 0");
				// now we have found the frame head
				if (tmp_ch == frame_head)
				{
					frame_byte_idx = 0;
					canbus_frame_buf[frame_byte_idx] = frame_head;
					frame_byte_idx++;
					flag = 1;
				}
				else
				{
					// if we have searched so many data, but cannot find frame_head
					// so we need move the remaining data to the buffer beginning
					if (raw_buf.r_idx > CANBUS_BUF_SIZE/2)
					{
						memcpy(raw_buf.buf, &raw_buf.buf[raw_buf.r_idx], raw_buf.w_idx-raw_buf.r_idx);
						raw_buf.w_idx -= raw_buf.r_idx;
                        raw_buf.r_idx = 0;
					}
				}
			}
			else if (flag == 1)
			{
				LOGD(" while(1)--->flag == 1");
				canbus_frame_buf[frame_byte_idx] = tmp_ch;
				frame_byte_idx++;

				// get frame length from buf[2]
				if (frame_byte_idx == 3)
				{
					data_len = tmp_ch;

					// frame length is larger than frame buffer, 
					// so we suppose this frame is a bad frame
					// and we need to check the byte after frame head again
					// to judge if these bytes can match frame head.
					if (data_len > CANBUS_FRAME_SIZE-4)
					{
						flag = 0;
						raw_buf.r_idx -= 2;
					}
					else
					{
						flag = 2;
					}
				}
			}
			else if (flag == 2)
			{
				LOGD(" while(1)--->flag == 2");
				canbus_frame_buf[frame_byte_idx] = tmp_ch;
				frame_byte_idx++;

				if (frame_byte_idx == data_len+4)
				{
					// call canbus frame process function here
					// move the remaining data to the buffer beginning
					memcpy(raw_buf.buf, &raw_buf.buf[raw_buf.r_idx], raw_buf.w_idx-raw_buf.r_idx);
					raw_buf.w_idx -= raw_buf.r_idx;
                    raw_buf.r_idx = 0;
					// reset flag to 0
					flag = 0;

					write(canbus_fd, &canbus_ack, 1);
                    int ret = parse_canbus_frame(canbus_frame_buf, data_len+4);
                    if(ret){
                        ALOGE("parse_canbus_frame error. error:%d", ret);
                    }
				}
			}
		}
	}
}

// sonata8 set UART--->38400
// myu, 2014-11-14
static jint android_server_CanBusService_setSercial_UART38400(
	JNIEnv* env, jobject obj,jint baud)
{
	int ret_val = 1;
	ret_val = ioctl(canbus_fd, CANBUS_IOCTL_MCU_UART_38400, baud);
	LOGD("myu setSercial_UART38400!!!-->%d",baud);
	return ret_val;
}

static jint android_server_CanBusService_native_setMcuUartFunc(
	JNIEnv* env, jobject obj, jint mcu_uart_func_type)
{
	int ret = -1;
	int func_type;

	func_type = mcu_uart_func_type;
	LOGD("myu native_setMcuUartFunc --->func_type = %d ",func_type);
    ret = ioctl(canbus_fd, CANBUS_IOCTL_MCU_UART_DEV, &func_type);
	
	return ret;
}

static jint android_server_CanBusService_native_setCarType(
	JNIEnv* env, jobject obj, jint type)
{
	  LOGD("myu setCarType type :%d ", type);
	if ((unsigned long)type >= eCarTypeNum)
	{
		car_type = eCarTypeVolkaswagen;
	}
	else
	{
		car_type = (tCarType)type;
		LOGD("myu setCarType :car_type= %d ",car_type);
	}
	
	return 0;
}

// sonata8 front and rear balance
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setFadVal(
	JNIEnv* env, jobject obj, jbyte fad_val)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x03, 0x01, 0x01, 0x00
	};
	
	sonata_car_info.fad_val = fad_val;
	frame_buf[3] = fad_val;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	for(int i=0;i<5;i++){
		LOGD("myu native_setFadVal= 0X%x ",frame_buf[i]);
	}
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}

	return ret_val;
}

// sonata8 left and right balance
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setBalVal(
	JNIEnv* env, jobject obj, jbyte bal_val)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x04, 0x01, 0x01, 0x00
	};
	
	sonata_car_info.bal_val = bal_val;
	frame_buf[3] = bal_val;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	for(int i=0;i<5;i++){
		LOGD("myu native_setBalVal= 0X%x ",frame_buf[i]);
	}
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}

	return ret_val;
}

// sonata8 volume setting
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setVol(
	JNIEnv* env, jobject obj, jbyte volume)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	int i;
	int current_vol;
	int target_vol;
	int step;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x05, 0x01, 0x01, 0x00
	};
	frame_buf[3] = volume;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	ret_val = write(canbus_fd, frame_buf, 5);
//	LOGD("myu----->into native_setVol---->sonata_car_info.vol= %d | volume= %d",sonata_car_info.vol,volume);
//	// mute on, set the volume to 0x00, 
//	if (volume & 0x80)		
//	{
//		LOGD("myu native_setVol volume high 8 bit ");
//		if (sonata_car_info.vol & 0x80)
//		{
//			LOGD("myu native_setVol sonata_car_info.vol 8 bit ");
//			return 0;
//		}
//		
//		if (sonata_car_info.vol >= 1)
//		{
//			LOGD("myu native_setVol sonata_car_info.vol >=1 ");
//			for (i=sonata_car_info.vol-1; i>=0; i--)
//			{
//				frame_buf[3] = i;
//				calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
//				for(int j=0; j<sizeof(frame_buf); j++){
//					LOGD("11111111111myu native_setVol fram_buf[%d] = 0X%02x\n ",j,frame_buf[j]);
//				}
//				count = write(canbus_fd, frame_buf, 5);
//				if (count == 5)
//				{
//					ret_val = 0;
//				}
//				else
//				{
//					ret_val = 1;
//				}
//			}
//		}
//		sonata_car_info.vol |= 0x80;
//		LOGD("myu native_setVol volume high 8 bit-->sonata_car_info.vol=%d ",(int)sonata_car_info.vol);
//	}
//	else
//	{
//		LOGD("myu native_setVol volume low 8 bit ");
//		// last time, the state is mute on
//		if (sonata_car_info.vol & 0x80)
//		{
//			LOGD("myu native_setVol sonata_car_info.vol low 8 bit ");
//			for (i=1; i<volume; i++)
//			{
//				frame_buf[3] = i;
//				calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
//				for(int j=0; j<sizeof(frame_buf); j++){
//					LOGD("22222222myu native_setVol fram_buf[%d] = 0X%02x\n ",j,frame_buf[j]);
//				}
//				count = write(canbus_fd, frame_buf, 5);
//				if (count == 5)
//				{
//					ret_val = 0;
//				}
//				else
//				{
//					ret_val = 1;
//				}
//			}
//		}
//		else if (sonata_car_info.vol < volume)
//		{
//			LOGD("myu native_setVol volume low 8 bit sonata_car_info.vol < volume");
//			LOGD("33333myu sonata_car_info.vol+1 =%d |volume =%d ",sonata_car_info.vol+1,volume);
//			for (i=sonata_car_info.vol+1; i<=volume; i++)
//			{
//				frame_buf[3] = sonata_car_info.vol = i;
//				calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
//				for(int j=0; j<sizeof(frame_buf); j++){
//					LOGD("333333333myu native_setVol fram_buf[%d] = 0X%02x\n ",j,frame_buf[j]);
//				}
//				count = write(canbus_fd, frame_buf, 5);
//				if (count == 5)
//				{
//					ret_val = 0;
//				}
//				else
//				{
//					ret_val = 1;
//				}
//			}
//		}
//		else if (sonata_car_info.vol > volume)		// decrease volume
//		{
//			LOGD("myu native_setVol volume low 8 bit sonata_car_info.vol > volume");
//			LOGD("44444myu sonata_car_info.vol-1 =%d |volume =%d ",sonata_car_info.vol-1,volume);
//			for (i=sonata_car_info.vol-1; i>=volume; i--)
//			{
//				frame_buf[3] = sonata_car_info.vol = i;
//				calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
//				for(int j=0; j<sizeof(frame_buf); j++){
//					LOGD("444444444myu native_setVol fram_buf[%d] = 0X%02x\n ",j,frame_buf[j]);
//				}
//				count = write(canbus_fd, frame_buf, 5);
//				if (count == 5)
//				{
//					ret_val = 0;
//				}
//				else
//				{
//					ret_val = 1;
//				}
//			}
//		}
//		//sonata_car_info.vol = volume;
//		LOGD("myu native_setVol volume low 8 bit-->sonata_car_info.vol=%d ",(int)sonata_car_info.vol);
//	}

	return ret_val;
}

// sonata8 set EQ bass
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setBass(
	JNIEnv* env, jobject obj, jbyte bass_val)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x06, 0x01, 0x01, 0x00
	};
	
	sonata_car_info.bass_val = bass_val;
	frame_buf[3] = bass_val;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	for(int i=0;i<5;i++){
		LOGD("myu native_setBass= 0X%x ",frame_buf[i]);
	}
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}

	return ret_val;
}

// sonata8 set EQ mid
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setMid(
	JNIEnv* env, jobject obj, jbyte mid_val)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x07, 0x01, 0x01, 0x00
	};
	
	sonata_car_info.mid_val = mid_val;
	frame_buf[3] = mid_val;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	for(int i=0;i<5;i++){
		LOGD("myu native_setMid= 0X%x ",frame_buf[i]);
	}
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}

	return ret_val;
}

// sonata8 set EQ treble
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setTre(
	JNIEnv* env, jobject obj, jbyte tre_val)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x08, 0x01, 0x01, 0x00
	};
	
	sonata_car_info.tre_val = tre_val;
	frame_buf[3] = tre_val;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	for(int i=0;i<5;i++){
			LOGD("myu native_setTre= 0X%x ",frame_buf[i]);
		}
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}

	return ret_val;
}

// sonata8 set ReadInfo
// myu, 2014-11-12
static jint android_server_CanBusService_native_readS8Info(
	JNIEnv* env, jobject obj, jbyte read_info)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x0F, 0x01, 0x01, 0x00
	};
	
	frame_buf[3] = read_info;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}
	LOGD("myu native_native_readS8Info :0X%x ret_val=%d count=%d ", read_info, ret_val, count);
	return ret_val;
}

// sonata8 power state.
// dzwei, 2014-11-7
static jint android_server_CanBusService_native_setPower(
	JNIEnv* env, jobject obj, jbyte on_off)
{
	int count;			// the byte number that have been written 
	int ret_val = 1;
	unsigned char frame_buf[5]=
	{
		0x2E, 0x0B, 0x01, 0x01, 0x00
	};
	on_off = !!on_off;	// set to 0 or 1
	frame_buf[3] = on_off;
	calculate_frame_checksum(frame_buf, 5, &frame_buf[4]);
	count = write(canbus_fd, frame_buf, 5);
	if (count == 5)
	{
		ret_val = 0;
	}
	LOGD("myu native_setPower on_off--->0X%x count=%d ", on_off, count);
	return ret_val;
}

//*********************************************************
//*   write data function
//*********************************************************
static jint android_server_CanBusService_native_sendCommand(JNIEnv* env, jobject obj, jbyteArray buf)
{
    int ret = -1;
    if(canbus_fd < 0){
        return -1;
    }

    int len = env->GetArrayLength(buf);
    unsigned char *data = (unsigned char*) malloc((len + 2) * sizeof(jbyte));
    data[0] = 0x2E;
    env->GetByteArrayRegion(buf, 0, len, (jbyte*)&data[1]);
#ifdef DEBUG
    ALOGD("native send command(len=%d):", len);
    for(int i=0; i<len; i++){
        ALOGD("data[%d] : 0x%02X", i, data[i]);
    }
#endif
    calculate_frame_checksum(data, len+2, &data[len+1]);
    ret = write(canbus_fd, data, len+2);
    free(data);
    return ret;
}

struct CanBusService {
    jobject   mCanBusServiceObj;
    jmethodID mGetMemberRadar;
    jmethodID mReportRadarMethod;
    jmethodID mGetMemberAirCondition;
    jmethodID mReportAirContition;
    jmethodID mGetMemberCarDoor;
    jmethodID mReportCarDoor;
    jmethodID mGetMemberSonata8;
    jmethodID mReportSonata8;
};
static CanBusService gCanBusService;

//*********************************************************
//* about Air Condition of can bus
//*********************************************************
struct AirCondition {
    jobject  mAirConditionObj;
    jmethodID mAcDisplaySwitch;
    jmethodID mAirConditioningSwitch;
    jmethodID mACSwitch;
    jmethodID mACMAXSwitch;
    jmethodID mCycle;
    jmethodID mAUTOStrongWindSwitch;
    jmethodID mAUTOSoftWindSiwtch;
    jmethodID mAUTOSwitch;
    jmethodID mDUALSwitch;
    jmethodID mMAXFORNTSwitch;
    jmethodID mREARSwitch;
    jmethodID mUpWindSwitch;
    jmethodID mHorizontalWindSwitch;
    jmethodID mDownWindSwitch;
    jmethodID mWindDirection;
    jmethodID mAirConditioningDisplaySiwtch;
    jmethodID mWindLevel;
    jmethodID mLeftTemp;
    jmethodID mRightTemp;
    jmethodID mAQSInternalCycleSwitch;
    jmethodID mLeftSeatHeatingLevel;
    jmethodID mRightSeatHeatingLevel;
    jmethodID mREARLockSwitch;
    jmethodID mAirConditioning;
    jmethodID mReportMethod;
};
static AirCondition gAirCondition;

//int updateAndReportAirConditon(unsigned char * buf, int len)
int updateAndReportAirConditon(void)
{
    JNIEnv* env;
    gJavaVM->AttachCurrentThread(&env, NULL);
    if (gAirCondition.mAirConditionObj == NULL)
        return -1;
    if (env == NULL) {
        ALOGE("nativeReportAirCondition error. env is NULL!");
        return -1;
    }
    
    LOGD("bShowAcInfo : %d", ac_info.bShowAcInfo);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
            gAirCondition.mAcDisplaySwitch,
            ac_info.bShowAcInfo == 0 ? false : true);

    LOGD("power : %d", ac_info.bPowerOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
            gAirCondition.mAirConditioningSwitch,
            ac_info.bPowerOn == 0 ? false : true);
    LOGD("ac : %d", ac_info.bAcOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
    		gAirCondition.mACSwitch,
    		ac_info.bAcOn == 0 ? false : true);

    LOGD("ac_max : %d", ac_info.bAcMax);
        env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mACMAXSwitch,
                ac_info.bAcMax == 0 ? false : true);
    
    LOGD("cycle : %d", ac_info.bLoopMode);
    env->CallIntMethod(gAirCondition.mAirConditionObj,
    		gAirCondition.mCycle,
    		ac_info.bLoopMode);
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, gAirCondition.mAUTOStrongWindSwitch );
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, gAirCondition.mAUTOSoftWindSiwtch );
    LOGD("auto : %d", ac_info.bAuto);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mAUTOSwitch,
        ac_info.bAuto == 0 ? false : true);

    LOGD("dual : %d", ac_info.bDualOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mDUALSwitch,
        ac_info.bDualOn == 0 ? false : true);
    LOGD("max : %d", ac_info.bFrontOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mMAXFORNTSwitch,
        ac_info.bFrontOn == 0 ? false : true);
    LOGD("rear : %d", ac_info.bRearOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mREARSwitch,
        ac_info.bRearOn == 0 ? false : true);
    LOGD("wind direction : %d", ac_info.fanMode);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mWindDirection,
    		ac_info.fanMode);
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, mAirConditioningDisplaySiwtch;
    LOGD("wind level : %d", ac_info.fanSpeed.iCurSpeed);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mWindLevel,
        ac_info.fanSpeed.iCurSpeed);
    jfloat leftTemp = (float)ac_info.tempLeft / 10;
    LOGD("left temp : %.1f", leftTemp);
    env->CallFloatMethod(gAirCondition.mAirConditionObj, gAirCondition.mLeftTemp, leftTemp);
    jfloat rightTemp = (float)ac_info.tempRight / 10;
    LOGD("right temp : %.1f", rightTemp);
    env->CallFloatMethod(gAirCondition.mAirConditionObj, gAirCondition.mRightTemp, rightTemp);
    LOGD("aqs : %d", ac_info.bAQS);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mAQSInternalCycleSwitch,
        ac_info.bAQS == 0 ? false : true);
    jint leftSeatHeatingLevel;
    if (ac_info.bShowLeftSeatHeated == 0) {
    	leftSeatHeatingLevel = 0;
    } else {
    	leftSeatHeatingLevel = ac_info.nLeftSeatHeated;
    }
    LOGD("left seat heating : %d", leftSeatHeatingLevel);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mLeftSeatHeatingLevel, leftSeatHeatingLevel);
    jint rightSeatHeatingLevel;
    if (ac_info.bShowRightSeatHeated == 0) {
        rightSeatHeatingLevel = 0;
    } else {
        rightSeatHeatingLevel = ac_info.nRightSeatHeated;
    }
    LOGD("right seat heating : %d", rightSeatHeatingLevel);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mRightSeatHeatingLevel, rightSeatHeatingLevel);
    LOGD("rear lock : %d", ac_info.bRearLock);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mREARLockSwitch,
        ac_info.bRearLock == 0 ? false : true);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportAirContition);
    gJavaVM->DetachCurrentThread();
    return 0;
}

static int registerAirConditionFieldIDs(JNIEnv * env)
{
    if(env == NULL) return -1;
    jclass clazz = env->FindClass("com/android/internal/car/can/AirConditioning");
    if(clazz == NULL){
        ALOGE("Can't registerRadarFieldIDs. Can't find AirConditioning class.");
        return -1;
    }

    gAirCondition.mAirConditioningSwitch =
    		env->GetMethodID(clazz, "setAirConditioningSwitch", "(Z)V");
	if (gAirCondition.mAirConditioningSwitch == NULL) {
        ALOGE("Can't find gAirCondition.AirConditioningSwitch");
	}

    gAirCondition.mAcDisplaySwitch = env->GetMethodID(clazz, "setAirConditioningDisplaySiwtch", "(Z)V");
    if (gAirCondition.mAcDisplaySwitch == NULL) {
        ALOGE("Can't find gAirCondition.mAcDisplaySwitch");
    }

	 gAirCondition.mACSwitch = env->GetMethodID(clazz, "setACSwitch", "(Z)V");
	if (gAirCondition.mACSwitch == NULL) {
        ALOGE("Can't find gAirCondition.ACSwitch");
	}

    gAirCondition.mACMAXSwitch = env->GetMethodID(clazz, "setACMAXSwitch", "(Z)V");
	if (gAirCondition.mACMAXSwitch == NULL) {
        ALOGE("Can't find gAirCondition.mACMAXSwitch");
	}
    
	 gAirCondition.mCycle = env->GetMethodID(clazz, "setCycle", "(I)V");
	if (gAirCondition.mCycle == NULL) {
        ALOGE("Can't find gAirCondition.Cycle");
	}
	 gAirCondition.mAUTOStrongWindSwitch =
			 env->GetMethodID(clazz, "setAUTOStrongWindSwitch", "(Z)V");
	if (gAirCondition.mAUTOStrongWindSwitch == NULL) {
        ALOGE("Can't find gAirCondition.AUTOStrongWindSwitch");
	}
	 gAirCondition.mAUTOSoftWindSiwtch =
			 env->GetMethodID(clazz, "setAUTOSoftWindSiwtch", "(Z)V");
	if (gAirCondition.mAUTOSoftWindSiwtch == NULL) {
        ALOGE("Can't find gAirCondition.AUTOSoftWindSiwtch");
	}

    gAirCondition.mAUTOSwitch =
			 env->GetMethodID(clazz, "setAUTOSwitch", "(Z)V");
	if (gAirCondition.mAUTOSwitch == NULL) {
        ALOGE("Can't find gAirCondition.mAUTOSwitch");
	}
    
	 gAirCondition.mDUALSwitch = env->GetMethodID(clazz, "setDUALSwitch", "(Z)V");
	if (gAirCondition.mDUALSwitch == NULL) {
        ALOGE("Can't find gAirCondition.DUALSwitch");
	}
	 gAirCondition.mMAXFORNTSwitch = env->GetMethodID(clazz, "setMAXFORNTSwitch", "(Z)V");
	if (gAirCondition.mMAXFORNTSwitch == NULL) {
        ALOGE("Can't find gAirCondition.MAXFORNTSwitch");
	}
	gAirCondition.mREARSwitch = env->GetMethodID(clazz, "setREARSwitch", "(Z)V");
	if (gAirCondition.mREARSwitch == NULL) {
        ALOGE("Can't find gAirCondition.REARSwitch");
	}
/*	gAirCondition.mUpWindSwitch = env->GetFieldID(clazz, "UpWindSwitch", "Z");
	if (gAirCondition.mUpWindSwitch == NULL) {
        ALOGE("Can't find gAirCondition.UpWindSwitch");
	}
	 gAirCondition.mHorizontalWindSwitch = env->GetFieldID(clazz, "HorizontalWindSwitch", "Z");
	if (gAirCondition.mHorizontalWindSwitch == NULL) {
        ALOGE("Can't find gAirCondition.HorizontalWindSwitch");
	}
	 gAirCondition.mDownWindSwitch = env->GetFieldID(clazz, "DownWindSwitch", "Z");
	if (gAirCondition.mDownWindSwitch == NULL) {
        ALOGE("Can't find gAirCondition.DownWindSwitch");
	}
*/    gAirCondition.mWindDirection =
		env->GetMethodID(clazz, "setWindDirection", "(I)V");
    if (gAirCondition.mWindDirection == NULL) {
        ALOGE("Can't find gAirCondition.WindDirection");
    }
    gAirCondition.mAirConditioningDisplaySiwtch =
    		env->GetMethodID(clazz, "setAirConditioningDisplaySiwtch", "(Z)V");
    if (gAirCondition.mAirConditioningDisplaySiwtch == NULL) {
        ALOGE("Can't find gAirCondition.AirConditioningDisplaySiwtch");
    }
    gAirCondition.mWindLevel = env->GetMethodID(clazz, "setWindLevel", "(I)V");
    if (gAirCondition.mWindLevel == NULL) {
        ALOGE("Can't find gAirCondition.WindLevel");
    }
    gAirCondition.mLeftTemp = env->GetMethodID(clazz,	"setLeftTemp", "(F)V");
    if (gAirCondition.mLeftTemp == NULL) {
        ALOGE("Can't find gAirCondition.LeftTemp");
    }
    gAirCondition.mRightTemp = env->GetMethodID(clazz, "setRightTemp", "(F)V");
    if (gAirCondition.mRightTemp == NULL) {
        ALOGE("Can't find gAirCondition.RightTemp");
    }
    gAirCondition.mAQSInternalCycleSwitch =
    		env->GetMethodID(clazz, "setAQSInternalCycleSwitch", "(Z)V");
    if (gAirCondition.mAQSInternalCycleSwitch == NULL) {
        ALOGE("Can't find gAirCondition.AQSInternalCycleSwitch");
    }
    gAirCondition.mLeftSeatHeatingLevel =
    		env->GetMethodID(clazz, "setLeftSeatHeatingLevel", "(I)V");
    if (gAirCondition.mLeftSeatHeatingLevel == NULL) {
        ALOGE("Can't find gAirCondition.LeftSeatHeatingLevel");
    }
    gAirCondition.mRightSeatHeatingLevel =
        env->GetMethodID(clazz, "setRightSeatHeatingLevel", "(I)V");
    if (gAirCondition.mRightSeatHeatingLevel == NULL) {
        ALOGE("Can't find gAirCondition.RightSeatHeatingLevel");
    }
    gAirCondition.mREARLockSwitch =
    		env->GetMethodID(clazz, "setREARLockSwitch", "(Z)V");
    if (gAirCondition.mREARLockSwitch == NULL) {
        ALOGE("Can't find gAirCondition.REARLockSwitch");
    }
    return 0;
}
//*********************************************************
//* about Radar of can bus
//*********************************************************
struct Radar {
    jobject   mRadarObj;
    jmethodID mSetDistanceHeadstockLeft;
    jmethodID mSetDistanceHeadstockRight;
    jmethodID mSetDistanceTailstockLeft;
    jmethodID mSetDistanceTailstockRight;
    jmethodID mSetDistanceHeadstockCentreLeft;
    jmethodID mSetDistanceHeadstockCentreRight;
    jmethodID mSetDistanceTailstockCentreLeft;
    jmethodID mSetDistanceTailstockCentreRight;
};
static Radar gRadar;

struct CarDoor {
	jobject mCarDoorObj;
	jmethodID mSetFrontLeft;
	jmethodID mSetFrontRight;
	jmethodID mSetRearLeft;
	jmethodID mSetRearRight;
	jmethodID mSetRearCenter;
};

static CarDoor gCarDoor;

struct CarVolume
{
	jobject mCarVolObj;
	jmethodID mSetFrontRearBal;
	jmethodID mSetLeftRightBal;
	jmethodID mSetVol;
	jmethodID mSetFrontLeftVol;	
	jmethodID mSetFrontRightVol;	
	jmethodID mSetRearLeftVol;	
	jmethodID mSetRearRightVol;	
	jmethodID mSetBas;			// bass
	jmethodID mSetMid;			// middle
	jmethodID mSetTre;			// treble
};
static CarVolume gCarVolume;

/*
// original code 
int updateAndReportCarDoor(unsigned char *buf, int len) {
	JNIEnv * env;
	gJavaVM->AttachCurrentThread(&env, NULL);
	if (env == NULL) {
		return -1;
	}
	if ((buf == NULL) || (len < 1)) {
		return -1;
	}
	if (gCarDoor.mCarDoorObj == NULL)
		return -1;
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetFrontLeft, buf[0]&0x01);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetFrontRight, (buf[0]&0x02)>>1);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetRearLeft, (buf[0]&0x04)>>2);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetRearRight, (buf[0]&0x1F)>>3);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportCarDoor);
	return 0;
}
*/
// code modified by dzwei, 2015-4-16
int updateAndReportCarDoor(void) {
	JNIEnv * env;
	gJavaVM->AttachCurrentThread(&env, NULL);
	if (env == NULL) {
		return -1;
	}

	if (gCarDoor.mCarDoorObj == NULL)
		return -1;
	
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetFrontLeft, door_info.front_left);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetFrontRight, door_info.front_right);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetRearLeft, door_info.rear_left);
	env->CallIntMethod(gCarDoor.mCarDoorObj, gCarDoor.mSetRearRight, door_info.rear_right);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportCarDoor);
    gJavaVM->DetachCurrentThread();
	
	return 0;
}

/*
// original code 
int updateAndReportRadar(unsigned char *buf, int len) {
    JNIEnv* env;
    gJavaVM->AttachCurrentThread(&env, NULL);
    if(env == NULL){
        ALOGE("updateAndReportRadar error. env is NULL!");
        return -1;
    }

    if((buf == NULL) || len < 8){
        ALOGE("Happen error in updateAndReportRadar. The parameters is invalid.");
        return -1;
    }
    if(gRadar.mRadarObj == NULL){
        ALOGE("Happen error in updateAndReportRadar. The gRadar.mRadarObj is NULL.");
        return -1;
    }
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockLeft, buf[0] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockRight, buf[1] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockLeft, buf[2] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockRight, buf[3] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockCentreLeft, buf[4] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockCentreRight, buf[5] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockCentreLeft, buf[6] & 0xFF);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockCentreRight, buf[7] & 0xFF);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportRadarMethod);

    gJavaVM->DetachCurrentThread();
    return 0;
}
*/
// code modified by dzwei, 2015-4-16
int updateAndReportRadar(void) {
    JNIEnv* env;
    gJavaVM->AttachCurrentThread(&env, NULL);
    if(env == NULL){
        ALOGE("updateAndReportRadar error. env is NULL!");
        return -1;
    }

    if(gRadar.mRadarObj == NULL){
        ALOGE("Happen error in updateAndReportRadar. The gRadar.mRadarObj is NULL.");
        return -1;
    }
	
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockLeft, radar_info.front_left);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockRight, radar_info.front_right);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockLeft, radar_info.rear_left);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockRight, radar_info.rear_right);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockCentreLeft, radar_info.front_center_left);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceHeadstockCentreRight, radar_info.front_center_right);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockCentreLeft, radar_info.rear_center_left);
    env->CallIntMethod(gRadar.mRadarObj, gRadar.mSetDistanceTailstockCentreRight, radar_info.rear_center_right);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportRadarMethod);

    gJavaVM->DetachCurrentThread();
    return 0;
}

int updateAndReportSonata8(tSonataCarInfo *sonata8Info, int len) {
	JNIEnv * env;
	gJavaVM->AttachCurrentThread(&env, NULL);
	  LOGD("myu  updateAndReportSonata8 is into");
	if (env == NULL) {
		return -1;
	}
	if ((sonata8Info == NULL) || (len < 1)) {
		return -1;
	}
	if (gCarVolume.mCarVolObj == NULL)
		return -1;
	LOGD("myu  updateAndReportSonata8-->FrontRearBal=%d LeftRightBal=%d SetBas=%d SetMid=%d SetTre=%d SetVolume=%d"
			,(int)(sonata8Info->fad_val), (int)(sonata8Info->bal_val), (int)(sonata8Info->bass_val), 
			(int)(sonata8Info->mid_val), (int)(sonata8Info->tre_val), (int)(sonata8Info->vol));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetFrontRearBal, (int)(sonata8Info->fad_val));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetLeftRightBal, (int)(sonata8Info->bal_val));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetBas, (int)(sonata8Info->bass_val));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetMid, (int)(sonata8Info->mid_val));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetTre, (int)(sonata8Info->tre_val));
	env->CallIntMethod(gCarVolume.mCarVolObj, gCarVolume.mSetVol, (int)(sonata8Info->vol));
	env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportSonata8);
	return 0;
}

static int registerRadarFieldIDs(JNIEnv *env){
    if(env == NULL) return -1;
    jclass clazz = env->FindClass("com/android/internal/car/can/Radar");
    if(clazz == NULL){
        ALOGE("Can't registerRadarFieldIDs. Can't find Radar class.");
        return -1;
    }

    gRadar.mSetDistanceHeadstockLeft = env->GetMethodID(clazz, "setDistanceHeadstockLeft", "(I)V");
    if(gRadar.mSetDistanceHeadstockLeft == NULL) return -1;

    gRadar.mSetDistanceHeadstockRight = env->GetMethodID(clazz, "setDistanceHeadstockRight", "(I)V");
    if(gRadar.mSetDistanceHeadstockRight == NULL) return -1;

    gRadar.mSetDistanceTailstockLeft = env->GetMethodID(clazz, "setDistanceTailstockLeft", "(I)V");
    if(gRadar.mSetDistanceTailstockLeft == NULL) return -1;

    gRadar.mSetDistanceTailstockRight = env->GetMethodID(clazz, "setDistanceTailstockRight", "(I)V");
    if(gRadar.mSetDistanceTailstockRight == NULL) return -1;

    gRadar.mSetDistanceHeadstockCentreLeft = env->GetMethodID(clazz, "setDistanceHeadstockCentreLeft", "(I)V");
    if(gRadar.mSetDistanceHeadstockCentreLeft == NULL) return -1;

    gRadar.mSetDistanceHeadstockCentreRight = env->GetMethodID(clazz, "setDistanceHeadstockCentreRight", "(I)V");
    if(gRadar.mSetDistanceHeadstockCentreRight == NULL) return -1;

    gRadar.mSetDistanceTailstockCentreLeft = env->GetMethodID(clazz, "setDistanceTailstockCentreLeft", "(I)V");
    if(gRadar.mSetDistanceTailstockCentreLeft == NULL) return -1;

    gRadar.mSetDistanceTailstockCentreRight = env->GetMethodID(clazz, "setDistanceTailstockCentreRight", "(I)V");
    if(gRadar.mSetDistanceTailstockCentreRight == NULL) return -1;

    return 0;
}

static int registerCarDoorFieldIDs(JNIEnv *env){
    if(env == NULL) return -1;
    jclass clazz = env->FindClass("com/android/internal/car/can/CarDoor");
    if(clazz == NULL){
        ALOGE("Can't registerCarDoorFieldIDs. Can't find Radar class.");
        return -1;
    }

    gCarDoor.mSetFrontLeft = env->GetMethodID(clazz, "setFrontLeft", "(Z)V");
    if(gCarDoor.mSetFrontLeft == NULL) {
    	ALOGE("gCarDoor.mSetFrontLeft");
    	return -1;
    }

    gCarDoor.mSetFrontRight = env->GetMethodID(clazz, "setFrontRight", "(Z)V");
    if(gCarDoor.mSetFrontRight == NULL) {
    	ALOGE("gCarDoor.mSetFrontRight");
    	return -1;
    }
    gCarDoor.mSetRearLeft = env->GetMethodID(clazz, "setRearLeft", "(Z)V");
    if(gCarDoor.mSetRearLeft == NULL){
    	ALOGE("gCarDoor.mSetRearLeft");
    	return -1;
    }

    gCarDoor.mSetRearRight = env->GetMethodID(clazz, "setRearRight", "(Z)V");
    if(gCarDoor.mSetRearRight == NULL) {
    	ALOGE("gCarDoor.mSetRearRight");
    	return -1;
    }

    gCarDoor.mSetRearCenter = env->GetMethodID(clazz, "setRearCenter", "(Z)V");
    if(gCarDoor.mSetRearCenter == NULL) {
    	ALOGE("gCarDoor.mSetRearCenter");
    	return -1;
    }

    return 0;
}
//************************************************************

static int registerSonata8FieldIDs(JNIEnv *env){
		 if(env == NULL) return -1;
     jclass clazz = env->FindClass("com/android/internal/car/can/Sonata8");
     if(clazz == NULL){
        ALOGE("Can't registerSonata8FieldIDs. Can't find Sonata8 class.");
        return -1;
     }
     gCarVolume.mSetFrontRearBal = env->GetMethodID(clazz, "setBalanceFrontAndRear", "(I)V");
   	 if(gCarVolume.mSetFrontRearBal == NULL) {
    		ALOGE("gCarVolume.mSetFrontRearBal");
    		return -1;
     }
     
     gCarVolume.mSetLeftRightBal = env->GetMethodID(clazz, "setBalanceLeftAndRight", "(I)V");
   	 if(gCarVolume.mSetLeftRightBal == NULL) {
    		ALOGE("gCarVolume.mSetLeftRightBal");
    		return -1;
     }
     
     gCarVolume.mSetBas = env->GetMethodID(clazz, "setVolumeEQBass", "(I)V");
   	 if(gCarVolume.mSetBas == NULL) {
    		ALOGE("gCarVolume.mSetBas");
    		return -1;
     }
     
     gCarVolume.mSetMid = env->GetMethodID(clazz, "setVolumeEQMid", "(I)V");
   	 if(gCarVolume.mSetMid == NULL) {
    		ALOGE("gCarVolume.mSetMid");
    		return -1;
     }
     
     gCarVolume.mSetTre = env->GetMethodID(clazz, "setVolumeEQTreble", "(I)V");
   	 if(gCarVolume.mSetTre == NULL) {
    		ALOGE("gCarVolume.mSetTre");
    		return -1;
     }
     
   	gCarVolume.mSetVol = env->GetMethodID(clazz, "setVolume", "(I)V");
   	   	 if(gCarVolume.mSetVol == NULL) {
   	    		ALOGE("gCarVolume.mSetVol");
   	    		return -1;
   	     }
     return 0;
	}
//************************************************************

static jboolean android_server_CanBusService_native_start(JNIEnv* env, jobject obj)
{
	pthread_t canbus_thread_id;
	int err = 0;
    gCanBusService.mCanBusServiceObj = env->NewGlobalRef(obj);
    env->GetJavaVM(&gJavaVM);
    canbus_fd = open(CANBUS_DEV, O_RDWR | O_NOCTTY);
    LOGD("myu native_start-->canbus_fd:%d",canbus_fd);
    //ioctl(canbus_fd, CANBUS_IOCTL_MCU_UART_38400,384);
    if(canbus_fd < 0){
        ALOGE("open %s failed. error:%d(%s)", CANBUS_DEV, errno, strerror(errno));
        return false;
    }

    //****************************
    //* get Radar object
    //****************************
    jobject tempRadar = (jobject)env->CallObjectMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mGetMemberRadar);
    gRadar.mRadarObj = env->NewGlobalRef(tempRadar);

    jobject tempAc = (jobject)env->CallObjectMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mGetMemberAirCondition);
    gAirCondition.mAirConditionObj = env->NewGlobalRef(tempAc);

    jobject tempCarDoor = (jobject)env->CallObjectMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mGetMemberCarDoor);
    gCarDoor.mCarDoorObj = env->NewGlobalRef(tempCarDoor);
    
    jobject tempSnata8 = (jobject)env->CallObjectMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mGetMemberSonata8);
    gCarVolume.mCarVolObj = env->NewGlobalRef(tempSnata8);
    //****************************
    
	err = pthread_create(&canbus_thread_id, NULL, canbus_thread_func, NULL);
	LOGD("myu native_start-->err:%d",err);
	if (err) {
		ALOGE("cant creat canbus_thread_func \r\n");
		return false;
	}
	LOGD("myu native_start!!!");
	// start can thread
	return true;
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
	{"native_start", "()Z", (void*)android_server_CanBusService_native_start},
    {"native_sendCommand", "([B)I", (void*)android_server_CanBusService_native_sendCommand},
	{"native_setMcuUartFunc", "(I)I", (void*)android_server_CanBusService_native_setMcuUartFunc},
	{"native_setCarType", "(I)I", (void*)android_server_CanBusService_native_setCarType},
	{"native_setFadVal", "(B)I", (void*)android_server_CanBusService_native_setFadVal},
	{"native_setBalVal", "(B)I", (void*)android_server_CanBusService_native_setBalVal},
	{"native_setVol", "(B)I", (void*)android_server_CanBusService_native_setVol},
	{"native_setBass", "(B)I", (void*)android_server_CanBusService_native_setBass},
	{"native_setMid", "(B)I", (void*)android_server_CanBusService_native_setMid},
	{"native_setTre", "(B)I", (void*)android_server_CanBusService_native_setTre},
	{"native_setPower", "(B)I", (void*)android_server_CanBusService_native_setPower},
	{"native_readS8Info", "(B)I", (void*)android_server_CanBusService_native_readS8Info},
	{"native_setUART38400", "(I)I", (void*)android_server_CanBusService_setSercial_UART38400},
};

//int register_android_server_BatteryService(JNIEnv* env)
int register_android_server_CanBusService(JNIEnv* env)
{
	gCanBusService.mCanBusServiceObj = NULL;

    jclass clazz = env->FindClass("com/android/server/CanBusService");
    if (clazz == NULL) {
        ALOGE("Can't find com/android/server/CanBusService");
        return -1;
    }

    gCanBusService.mGetMemberRadar = env->GetMethodID(clazz, "getMemberRadar", 
        "()Lcom/android/internal/car/can/Radar;");
    gCanBusService.mReportRadarMethod = env->GetMethodID(clazz, "reportRadarInfo", "()V");
    gCanBusService.mGetMemberAirCondition = env->GetMethodID(clazz, "getMemberAirCondition",
        "()Lcom/android/internal/car/can/AirConditioning;");
	gCanBusService.mReportAirContition = env->GetMethodID(clazz, "reportAirConditioning", "()V" );
	gCanBusService.mGetMemberCarDoor = env->GetMethodID(clazz, "getMemberCarDoor",
			"()Lcom/android/internal/car/can/CarDoor;");
	gCanBusService.mReportCarDoor = env->GetMethodID(clazz, "reportCarDoor", "()V");
	gCanBusService.mGetMemberSonata8 = env->GetMethodID(clazz, "getMemberSonata8", 
				"()Lcom/android/internal/car/can/Sonata8;");
	gCanBusService.mReportSonata8 = env->GetMethodID(clazz, "reportSonata8", "()V" );
    if(registerRadarFieldIDs(env) != 0)
        return -1;
    
    if(registerAirConditionFieldIDs(env) != 0)
        return -1;

    if(registerCarDoorFieldIDs(env) != 0)
    	return -1;
    	
    if(registerSonata8FieldIDs(env) != 0)
    	return -1;

    return jniRegisterNativeMethods(env, "com/android/server/CanBusService", sMethods, NELEM(sMethods));
}

} /* namespace android */


