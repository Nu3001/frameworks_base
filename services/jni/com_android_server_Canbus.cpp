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

#define CANBUS_DEV     			"/dev/bonovo_canbus"    
#define CANBUS_BUF_SIZE 		4096
#define CANBUS_FRAME_SIZE			64
int canbus_fd = -1;
JavaVM *gJavaVM;

#define TYPE_RESPONE_HOST_REQUEST   0x10
#define TYPE_RESPONE_BACKLIGHT      0x14
#define TYPE_RESPONE_CARSPEED       0x16
#define TYPE_RESPONE_AIRCONDITION   0x21
#define TYPE_RESPONE_RADAR          0x22
#define TYPE_RESPONE_WHEELKEYCODE   0x23
#define TYPE_RESPONE_WHEELANGLE     0x24
#define TYPE_RESPONE_ADVANCEINFO    0x25
#define TYPE_RESPONE_CARDOORINFO    0x26
#define TYPE_RESPONE_SOFTVERSION    0x71

#define DEV_MAJOR			        236
#define CANBUS_IOCTL_KEY_INFO		_IO(DEV_MAJOR, 0)


struct canbus_buf_t
{
	unsigned char buf[CANBUS_BUF_SIZE];
	int w_idx;			// indicate the buffer that is written now
	int r_idx;			// indicate the buffer that is read now
	int valid_data_num;	// indicate how many valid data in the buffer
};

// 送风模式
typedef enum _CAN_FAN_MODE
{
	CAN_FAN_MODE_NONE = 0,				// 无风
	CAN_FAN_MODE_HORI,					// 平行送风
	CAN_FAN_MODE_UP,					// 向上送风(风挡送风)
	CAN_FAN_MODE_DOWN,					// 向下送风(腿部送风)
	CAN_FAN_MODE_HORI_DOWN,				// 平行与向下送风
	CAN_FAN_MODE_UP_DOWN,				// 向上与向下送风
	CAN_FAN_MODE_HORI_UP,				// 平行与向上送风
	CAN_FAN_MODE_HORI_UP_DOWN,			// 平行、向上与向下送风
	CAN_FAN_MODE_AUTO = 100				// 自动送风模式
}CAN_FAN_MODE;

// 风速
typedef struct _CAN_FAN_SPEED_LEVEL
{
	int iCurSpeed;      // 当前速度，0xffff 自动风量
	int iMaxSpeed;      // 最大速度
}CAN_FAN_SPEEDLEVEL,*PCAN_FAN_SPEEDLEVEL;


// 空调信息
typedef struct _CAN_AC_INFO
{
    int bShowAcInfo;         // true 显示空调信息，false 不显示空调信息
	int bPowerOn;            // true 空调开，false 空调关
	int bAcOn;               // A/C指示(空调压缩机开关指示)
	int bAuto;               // AUTO指示(Auto大小风有一个开该指示均置位)
	int bDeicerOn;           // 除冰灯指示
	int bDualOn;             // DUAL指示
	int bSwingOn;            // SWING指示(皇冠专用)
	int bKafunOn;            // KAFUN指示(皇冠专用)
	int bFrontOn;            // FRONT指示
	int bRearOn;             // REAR指示
	int bIonOn;              // ION指示(离子)
	int bLoopMode;			 // 1:内循环,0:外循环指示
	int bAQS;                // AQS内循环指示
	int bRearLock;           // 后座空调锁定
	int bAcMax;              // ACMAX指示，空调所有值都为最大
	CAN_FAN_MODE fanMode;         // 送风模式
	CAN_FAN_SPEEDLEVEL fanSpeed;  // 风速
	int bShowLeftTemp;       // 是否显示左温度
	unsigned int     tempLeft;            // 左温度,0X0000 LO,0XFFFF HI
	int bShowRightTemp;      // 是否显示右温度
	unsigned int     tempRight;           // 右温度,0X0000 LO,OXFFFF HI
	int bShowOutdoorTemp;    // 是否显示室外温度
	int tempOutDoor;         // 室外温度(CAN_INVALID_VALUE 表示无效数据或不支持)
	int bShowLeftSeatHeated; // 是否显示左桌椅加热
	unsigned int     nLeftSeatHeated;     // 左桌椅加热温度等级，1-3级
	int bShowRightSeatHeated;// 是否显示右桌椅加热
	unsigned int     nRightSeatHeated;    // 右桌椅加热温度等级，1-3级
	//int bEcoOn;              // ECO指示(不知道是哪个车型使用的)
	//int bZoneOn;             // ZONE指示(不知道是哪个车型使用的)
	//int bAutoFanSpeed;       // 自动风量指示(不知道是哪个车型使用的)
}CAN_AC_INFO,*PCAN_AC_INFO;


unsigned char canbus_frame_buf[CANBUS_FRAME_SIZE];
unsigned char ac_cache[6];
unsigned char radar_cache[9];
unsigned int key_cache[3];
unsigned char car_door_cache[2];
CAN_AC_INFO canbus_ac_info;
static void nativeReportAirCondition();
int updateAndReportRadar(unsigned char *buf, int len);
int updateAndReportAirConditon(unsigned char * buf, int len);
int updateAndReportCarDoor(unsigned char * buf, int len);

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


int parse_canbus_ac_info(unsigned char *ac_data_buf, int buf_len)
{
    //ALOGE("==== ac_data_buf [0]:0x%02X  [1]:0x%02X  [2]:0x%02X  [3]:0x%02X  [4]:0x%02X\n",
    //    ac_data_buf[0], ac_data_buf[1], ac_data_buf[2], ac_data_buf[3], ac_data_buf[4]);
	// parse 1st byte
	canbus_ac_info.bPowerOn = (ac_data_buf[0]&0x80)>>7;
	canbus_ac_info.bAcOn = (ac_data_buf[0]&0x40)>>6;
	canbus_ac_info.bLoopMode = (ac_data_buf[0]&0x20)>>5;
	canbus_ac_info.bAuto = !!((ac_data_buf[0]&0x1F)>>3);
	canbus_ac_info.bDualOn = (ac_data_buf[0]&0x04)>>2;
	canbus_ac_info.bFrontOn = (ac_data_buf[0]&0x02)>>1;
	canbus_ac_info.bRearOn = ac_data_buf[0]&0x01;

	// parse 2nd byte
	switch ((ac_data_buf[1]&0xE0)>>5)
	{
		case 0:
			canbus_ac_info.fanMode = CAN_FAN_MODE_NONE;
			break;
		case 1:
			canbus_ac_info.fanMode = CAN_FAN_MODE_DOWN;
			break;
		case 2:
			canbus_ac_info.fanMode = CAN_FAN_MODE_HORI;
			break;
		case 3:
			canbus_ac_info.fanMode = CAN_FAN_MODE_HORI_DOWN;
			break;
		case 4:
			canbus_ac_info.fanMode = CAN_FAN_MODE_UP;
			break;
		case 5:
			canbus_ac_info.fanMode = CAN_FAN_MODE_UP_DOWN;
			break;
		case 6:
			canbus_ac_info.fanMode = CAN_FAN_MODE_HORI_UP;
			break;
		case 7:
			canbus_ac_info.fanMode = CAN_FAN_MODE_HORI_UP_DOWN;
			break;
		default:
			break;
	}
    canbus_ac_info.bShowAcInfo = (ac_data_buf[1]&0x10) >> 4;
	canbus_ac_info.fanSpeed.iCurSpeed = ac_data_buf[1]&0x07;

	// parse 3rd byte, left temperature
	if (ac_data_buf[2] == 0)
	{
		canbus_ac_info.tempLeft = 0;
	}
	else if (ac_data_buf[2] < 0x11)
	{
		canbus_ac_info.tempLeft = 180+(ac_data_buf[2]-1)*5;
	}
	else if (ac_data_buf[2] == 0x1F)
	{
		canbus_ac_info.tempLeft = 0xFFFF;
	}

	// parse 4th byte, right temperature
	if (ac_data_buf[3] == 0)
	{
		canbus_ac_info.tempRight = 0;
	}
	else if (ac_data_buf[3] < 0x11)
	{
		canbus_ac_info.tempRight = 180+(ac_data_buf[3]-1)*5;
	}
	else if (ac_data_buf[3] == 0x1F)
	{
		canbus_ac_info.tempRight = 0xFFFF;
	}

	// parse 5th byte,
	canbus_ac_info.bAQS = (ac_data_buf[4]&0x80)>>7;
	canbus_ac_info.nLeftSeatHeated = (ac_data_buf[4]&0x70)>>4;
	canbus_ac_info.bShowLeftSeatHeated = !!canbus_ac_info.nLeftSeatHeated;
	canbus_ac_info.bRearLock = (ac_data_buf[4]&0x08)>>3;
	canbus_ac_info.bAcMax = (ac_data_buf[4]&0x04)>>2;
    canbus_ac_info.nRightSeatHeated = ac_data_buf[4] & 0x03;
    canbus_ac_info.bShowRightSeatHeated = !!canbus_ac_info.nRightSeatHeated;

	return 0;
}

int process_canbus_command(unsigned char *buf, int frame_len)
{
	switch(buf[1])
	{
		case TYPE_RESPONE_AIRCONDITION:
			if (!ac_cache[0])			// ac_cache has no data in it
			{
				memcpy(&ac_cache[1], &buf[3], 5);
				ac_cache[0] = 2;
			}
			else if (memcmp(&ac_cache[1], &buf[3], 5))
			{
				memcpy(&ac_cache[1], &buf[3], 5);
				ac_cache[0] = 2;		// ac_cache has data in it, and need parse the data
			}
			else 
			{
				ac_cache[0] = 1;		// ac_cache has data in it, and do not need parse data
			}
			
			if (ac_cache[0] == 2)
			{
				parse_canbus_ac_info(&ac_cache[1], 5);
				//nativeReportAirCondition();
                updateAndReportAirConditon(&ac_cache[1], 5);
			}
			break;
        case TYPE_RESPONE_RADAR:
            if(!radar_cache[0])
            {
                memcpy(&radar_cache[1], &buf[3], 8);
                radar_cache[0] = 2;
            }else if (memcmp(&radar_cache[1], &buf[3], 8)){
                memcpy(&radar_cache[1], &buf[3], 8);
                radar_cache[0] = 2;
            }else{
                radar_cache[0] = 1;
            }
            if(radar_cache[0] == 2){
                updateAndReportRadar(&radar_cache[1], 8);
            }
            break;
        case TYPE_RESPONE_WHEELKEYCODE:
            memset(key_cache, 0x00, sizeof(key_cache));
            key_cache[0] = 2; // make the key valid.
            key_cache[1] = (buf[3] & 0xFF)<<8; // make the min difference of two key is 256
            key_cache[2] = buf[4]; // the key's status(down or up);
            ioctl(canbus_fd, CANBUS_IOCTL_KEY_INFO, &key_cache[0]);
            break;
        case TYPE_RESPONE_CARDOORINFO:
            if(!car_door_cache[0])
            {
                memcpy(&car_door_cache[1], &buf[3], 1);
                car_door_cache[0] = 2;
            }else if (memcmp(&car_door_cache[1], &buf[3], 1)){
                memcpy(&car_door_cache[1], &buf[3], 1);
                car_door_cache[0] = 2;
            }else{
            	car_door_cache[0] = 1;
            }
            if(car_door_cache[0] == 2){
                updateAndReportCarDoor(&car_door_cache[1], 1);
            }
            break;
		default:
			break;
	}
	return 0;
}

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

void *canbus_thread_func(void *argv) 
{
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

	memset(&raw_buf, 0x00, sizeof(struct canbus_buf_t));
	memset(ac_cache, 0x00, 6);
    memset(radar_cache, 0x00, 9);

	calculate_frame_checksum(start_cmd_frame, 5, &start_cmd_frame[4]);
	count = write(canbus_fd, start_cmd_frame, 5);
	if (count < 5)
	{
        ALOGE("Can't start. Write %s failed, errno:%d(%s)", CANBUS_DEV, errno, strerror(errno));
	}
	frame_head = 0x2E;
	
	if (canbus_fd == -1)
	{
        ALOGE("could not open %s, errno:%d(%s)", CANBUS_DEV, errno, strerror(errno));
		return NULL;
	}
	
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

int updateAndReportAirConditon(unsigned char * buf, int len)
{
    JNIEnv* env;
    gJavaVM->AttachCurrentThread(&env, NULL);
    if (gAirCondition.mAirConditionObj == NULL)
        return -1;
    if (env == NULL) {
        ALOGE("nativeReportAirCondition error. env is NULL!");
        return -1;
    }
    
    LOGD("bShowAcInfo : %d", canbus_ac_info.bShowAcInfo);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
            gAirCondition.mAcDisplaySwitch,
            canbus_ac_info.bShowAcInfo == 0 ? false : true);

    LOGD("power : %d", canbus_ac_info.bPowerOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
            gAirCondition.mAirConditioningSwitch,
            canbus_ac_info.bPowerOn == 0 ? false : true);
    LOGD("ac : %d", canbus_ac_info.bAcOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
    		gAirCondition.mACSwitch,
    		canbus_ac_info.bAcOn == 0 ? false : true);

    LOGD("ac_max : %d", canbus_ac_info.bAcMax);
        env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mACMAXSwitch,
                canbus_ac_info.bAcMax == 0 ? false : true);
    
    LOGD("cycle : %d", canbus_ac_info.bLoopMode);
    env->CallIntMethod(gAirCondition.mAirConditionObj,
    		gAirCondition.mCycle,
    		canbus_ac_info.bLoopMode);
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, gAirCondition.mAUTOStrongWindSwitch );
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, gAirCondition.mAUTOSoftWindSiwtch );
    LOGD("auto : %d", canbus_ac_info.bAuto);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mAUTOSwitch,
        canbus_ac_info.bAuto == 0 ? false : true);

    LOGD("dual : %d", canbus_ac_info.bDualOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mDUALSwitch,
        canbus_ac_info.bDualOn == 0 ? false : true);
    LOGD("max : %d", canbus_ac_info.bFrontOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mMAXFORNTSwitch,
        canbus_ac_info.bFrontOn == 0 ? false : true);
    LOGD("rear : %d", canbus_ac_info.bRearOn);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj, gAirCondition.mREARSwitch,
        canbus_ac_info.bRearOn == 0 ? false : true);
    LOGD("wind direction : %d", canbus_ac_info.fanMode);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mWindDirection,
    		canbus_ac_info.fanMode);
    //	env->SetBooleanField(gAirCondition.mAirConditionObj, mAirConditioningDisplaySiwtch;
    LOGD("wind level : %d", canbus_ac_info.fanSpeed.iCurSpeed);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mWindLevel,
        canbus_ac_info.fanSpeed.iCurSpeed);
    jfloat leftTemp = (float)canbus_ac_info.tempLeft / 10;
    LOGD("left temp : %.1f", leftTemp);
    env->CallFloatMethod(gAirCondition.mAirConditionObj, gAirCondition.mLeftTemp, leftTemp);
    jfloat rightTemp = (float)canbus_ac_info.tempRight / 10;
    LOGD("right temp : %.1f", rightTemp);
    env->CallFloatMethod(gAirCondition.mAirConditionObj, gAirCondition.mRightTemp, rightTemp);
    LOGD("aqs : %d", canbus_ac_info.bAQS);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mAQSInternalCycleSwitch,
        canbus_ac_info.bAQS == 0 ? false : true);
    jint leftSeatHeatingLevel;
    if (canbus_ac_info.bShowLeftSeatHeated == 0) {
    	leftSeatHeatingLevel = 0;
    } else {
    	leftSeatHeatingLevel = canbus_ac_info.nLeftSeatHeated;
    }
    LOGD("left seat heating : %d", leftSeatHeatingLevel);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mLeftSeatHeatingLevel, leftSeatHeatingLevel);
    jint rightSeatHeatingLevel;
    if (canbus_ac_info.bShowRightSeatHeated == 0) {
        rightSeatHeatingLevel = 0;
    } else {
        rightSeatHeatingLevel = canbus_ac_info.nRightSeatHeated;
    }
    LOGD("right seat heating : %d", rightSeatHeatingLevel);
    env->CallIntMethod(gAirCondition.mAirConditionObj, gAirCondition.mRightSeatHeatingLevel, rightSeatHeatingLevel);
    LOGD("rear lock : %d", canbus_ac_info.bRearLock);
    env->CallBooleanMethod(gAirCondition.mAirConditionObj,
        gAirCondition.mREARLockSwitch,
        canbus_ac_info.bRearLock == 0 ? false : true);
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

int updateAndReportRadar(unsigned char *buf, int len) {
    JNIEnv* env;
    gJavaVM->AttachCurrentThread(&env, NULL);
    if(env == NULL){
        ALOGE("updateAndReportRadar error. env is NULL!");
        return -1;
    }
/*
    jbyteArray data = env->NewByteArray(len);
    if(data == NULL){
        gJavaVM->DetachCurrentThread();
        return -1;
    }

    env->SetByteArrayRegion(data, 0, len, (jbyte*)buf);
    env->CallVoidMethod(gCanBusService.mCanBusServiceObj, gCanBusService.mReportRadarMethod, data);
    env->DeleteLocalRef(data);
*/

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

static jboolean android_server_CanBusService_native_start(JNIEnv* env, jobject obj)
{
	pthread_t canbus_thread_id;
	int err = 0;
    gCanBusService.mCanBusServiceObj = env->NewGlobalRef(obj);
    env->GetJavaVM(&gJavaVM);
    canbus_fd = open(CANBUS_DEV, O_RDWR | O_NOCTTY);
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
    //****************************
    
	err = pthread_create(&canbus_thread_id, NULL, canbus_thread_func, NULL); 
	if (err) {
		ALOGE("cant creat canbus_thread_func \r\n");
		return false;
	}

	// start can thread
	return true;
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
	{"native_start", "()Z", (void*)android_server_CanBusService_native_start},
    {"native_sendCommand", "([B)I", (void*)android_server_CanBusService_native_sendCommand},
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
    if(registerRadarFieldIDs(env) != 0)
        return -1;
    
    if(registerAirConditionFieldIDs(env) != 0)
        return -1;

    if(registerCarDoorFieldIDs(env) != 0)
    	return -1;

    return jniRegisterNativeMethods(env, "com/android/server/CanBusService", sMethods, NELEM(sMethods));
}

} /* namespace android */


