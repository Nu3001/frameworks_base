#include "com_android_server_Canbus.h"
#include <sys/ioctl.h>
#include <unistd.h>

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

//===================================================================
// data type for Hyundai and KIA, canbox vendor is XINPU 
// 2015-4-13
//-------------------------------------------------------------------
// data direction canbox-->Android
#define XINPU_HY_KIA_INFO_BACKLIGHT		0x14
#define XINPU_HY_KIA_INFO_SPEED			0x16
#define XINPU_HY_KIA_INFO_KEY			0x20
#define XINPU_HY_KIA_INFO_AC			0x21
#define XINPU_HY_KIA_INFO_REAR_RADAR	0x22
#define XINPU_HY_KIA_INFO_FRONT_RADAR	0x23
#define XINPU_HY_KIA_INFO_VEHICLE		0x24
#define XINPU_HY_KIA_INFO_OUTDOOR_TEMP	0x27
#define XINPU_HY_KIA_INFO_PWR_AMP		0x29
#define XINPU_HY_KIA_INFO_VERSION		0x30
#define XINPU_HY_KIA_INFO_BLUELINK		0x32
#define XINPU_HY_KIA_INFO_TPMS			0x40
//-------------------------------------------------------------------
// data direction canbox<--Android
#define XINPU_HY_KIA_SET_SOURCE			0xC0
#define XINPU_HY_KIA_SET_VOL_SHOW		0xC4
#define XINPU_HY_KIA_SET_CTRL_CMD		0xC6
#define XINPU_HY_KIA_SET_PWR_AMP		0xC7
#define XINPU_HY_KIA_GET_BLUELINK		0xC8
#define XINPU_HY_KIA_SET_TIME			0xC9
#define XINPU_HY_KIA_SET_TPMS			0xF5
//===================================================================

extern int canbus_fd;
extern CAN_AC_INFO ac_info;
extern tCarDoorInfo door_info;
extern tCarRadarInfo radar_info;
extern int updateAndReportCarDoor(void);
extern int updateAndReportRadar(void);
extern int updateAndReportAirConditon(void);

static unsigned char ac_cache[6]={0};
static unsigned char frant_radar_cache[9]= {0};
static unsigned int key_cache[3] = {0};
static unsigned char car_door_cache[2] = {0};
static unsigned char vehicle_info_cache[2] = {0};
static unsigned char rear_radar_cache[4] = {0};
static unsigned char front_radar_cache[4] = {0};

int parse_canbus_ac_info(unsigned char *ac_data_buf, int buf_len)
{
	// parsing the 1st byte
	ac_info.bPowerOn = (ac_data_buf[0]&0x80)>>7;
	ac_info.bAcOn = (ac_data_buf[0]&0x40)>>6;
	ac_info.bLoopMode = (ac_data_buf[0]&0x20)>>5;
	ac_info.bAuto = !!((ac_data_buf[0]&0x1F)>>3);
	ac_info.bDualOn = (ac_data_buf[0]&0x04)>>2;
	ac_info.bFrontOn = (ac_data_buf[0]&0x01);

	// parsing the 2nd byte
	// 送风模式
	switch ((ac_data_buf[1]&0x70)>>5)
	{
		case 0x00:
			ac_info.fanMode = CAN_FAN_MODE_NONE;
			break;
		case 0x01:
			ac_info.fanMode = CAN_FAN_MODE_DOWN;
			break;
		case 0x02:
			ac_info.fanMode = CAN_FAN_MODE_HORI;
			break;
		case 0x03:
			ac_info.fanMode = CAN_FAN_MODE_HORI_DOWN;
			break;
		case 0x04:
			ac_info.fanMode = CAN_FAN_MODE_UP;
			break;
		case 0x05:
			ac_info.fanMode = CAN_FAN_MODE_UP_DOWN;
			break;
		case 0x06:
			ac_info.fanMode = CAN_FAN_MODE_HORI_UP;
			break;
		case 0x07:
			ac_info.fanMode = CAN_FAN_MODE_HORI_UP_DOWN;
			break;
	}

	// fan speed
	ac_info.bShowAcInfo = !!(ac_data_buf[1]&0x10);
	ac_info.fanSpeed.iCurSpeed = ac_data_buf[1]&0x0F;
	ac_info.fanSpeed.iMaxSpeed = 0x08;
	
	// parsing the 3rd byte, left temperature
	if (ac_data_buf[2] == 0)
	{
		ac_info.tempLeft = 0;
	}
	else if (ac_data_buf[2] < 0x1E)
	{
		ac_info.tempLeft = 175+(ac_data_buf[2]-1)*5;
	}
	else if (ac_data_buf[2] == 0x1E)
	{
		ac_info.tempLeft = 0xFFFF;
	}

	// parsing the 4th byte, right temperature
	if (ac_data_buf[3] == 0)
	{
		ac_info.tempRight = 0;
	}
	else if (ac_data_buf[3] < 0x1E)
	{
		ac_info.tempRight = 175+(ac_data_buf[3]-1)*5;
	}
	else if (ac_data_buf[3] == 0x1E)
	{
		ac_info.tempRight = 0xFFFF;
	}
	
	// parsing the 5th byte,
	ac_info.bAQS = (ac_data_buf[4]&0x80)>>7;
	ac_info.nLeftSeatHeated = (ac_data_buf[4]&0x30)>>4;
	ac_info.bShowLeftSeatHeated = !!ac_info.nLeftSeatHeated;
	ac_info.bRearLock = (ac_data_buf[4]&0x08)>>3;
	ac_info.bAcMax = (ac_data_buf[4]&0x04)>>2;
	ac_info.nRightSeatHeated = ac_data_buf[4] & 0x03;
	ac_info.bShowRightSeatHeated = !!ac_info.nRightSeatHeated;
	
	return 0;
}

//===================================================================
// int process_canbus_command_XINPU_HY_KIA(unsigned char *buf, int frame_len)
//-------------------------------------------------------------------
// process canbus command of Hyundai and KIA of XINPU canbox
// buf[1]: command type
// buf: contain one whole frame, 
//	buf[0]--- frame head
//	buf[1]--- command type
//	buf[2]--- data length
//	buf[3]... command data
//-------------------------------------------------------------------
// 2015-4-14
//===================================================================
int process_canbus_command_XINPU_HY_KIA(unsigned char *buf, int frame_len)
{
	int i;
	LOGD("process_canbus_command_XINPU_HY_KIA !!!");
	switch(buf[1])
	{
		case XINPU_HY_KIA_INFO_BACKLIGHT:
			break;
		case XINPU_HY_KIA_INFO_SPEED:
			break;

		// 将按键键值写入canbus设备驱动,通过该驱动将按键上报给安卓
		// 这样操作是为了按键事件的上报有统一的接口
		case XINPU_HY_KIA_INFO_KEY:
            memset(key_cache, 0x00, sizeof(key_cache));
            key_cache[0] = 2; // make the key valid.
            key_cache[1] = (buf[3] & 0xFF)<<8; // make the min difference of two key to 256
            key_cache[2] = buf[4]; // the key's status(down or up);
            ioctl(canbus_fd, CANBUS_IOCTL_KEY_INFO, &key_cache[0]);
			break;
		case XINPU_HY_KIA_INFO_AC:
			parse_canbus_ac_info(&buf[3], 5);
			updateAndReportAirConditon();
			break;
		case XINPU_HY_KIA_INFO_REAR_RADAR:
			if (memcmp(rear_radar_cache, &buf[3], 4))
			{
				memcpy(rear_radar_cache, &buf[3], 4);
				// 将sonata9的4级雷达转换成上层软件标准的0-10
				for (i=0; i<4; i++)
				{
					if (buf[3+i] == 0x01)
					{
						buf[3+i] = 10;
					}
					else if (buf[3+i] == 0x02)
					{
						buf[3+i] = 6;
					}
					else if (buf[3+i] == 0x03)
					{
						buf[3+i] = 2;
					}
					else
					{
						buf[3+i] = 0;
					}
				}
				radar_info.rear_left = buf[3];
				radar_info.rear_center_left = buf[4];
				radar_info.rear_center_right = buf[5];
				radar_info.rear_right = buf[6];
				updateAndReportRadar();
			}
			break;
		case XINPU_HY_KIA_INFO_FRONT_RADAR:
			if (memcmp(front_radar_cache, &buf[3], 4))
			{
				memcpy(front_radar_cache, &buf[3], 4);
				// 将sonata9的4级雷达转换成上层软件标准的0-10
				for (i=0; i<4; i++)
				{
					if (buf[3+i] == 0x01)
					{
						buf[3+i] = 10;
					}
					else if (buf[3+i] == 0x02)
					{
						buf[3+i] = 6;
					}
					else if (buf[3+i] == 0x03)
					{
						buf[3+i] = 2;
					}
					else
					{
						buf[3+i] = 0;
					}
				}
				radar_info.front_left = buf[3];
				radar_info.front_center_left = buf[4];
				radar_info.front_center_right = buf[5];
				radar_info.front_right = buf[6];
				updateAndReportRadar();
			}
			break;
		case XINPU_HY_KIA_INFO_VEHICLE:
			if (memcmp(vehicle_info_cache, &buf[3], 2))
			{
				memcpy(vehicle_info_cache, &buf[3], 2);
				door_info.front_left = buf[4]&0x01;
				door_info.front_right = !!(buf[4]&0x02);
				door_info.rear_left = !!(buf[4]&0x04);
				door_info.rear_right = !!(buf[4]&0x08);
				door_info.trunk = !!(buf[4]&0x10);
				updateAndReportCarDoor();
			}
			break;
		case XINPU_HY_KIA_INFO_OUTDOOR_TEMP:
			if (ac_info.tempOutDoor != (signed char)buf[3])
			{
				ac_info.tempOutDoor = (signed char)buf[3];
				updateAndReportAirConditon();
			}
			break;
		case XINPU_HY_KIA_INFO_PWR_AMP:
			break;
		case XINPU_HY_KIA_INFO_VERSION:
			break;
		case XINPU_HY_KIA_INFO_BLUELINK:
			break;
		case XINPU_HY_KIA_INFO_TPMS:
			break;
		default:
			break;
	}
	return 0;
}

} // namespace android

