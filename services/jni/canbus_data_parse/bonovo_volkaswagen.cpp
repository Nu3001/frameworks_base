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
// below data type for volkaswagen
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

extern int canbus_fd;
extern CAN_AC_INFO ac_info;
extern tCarDoorInfo door_info;
extern tCarRadarInfo radar_info;
static unsigned char ac_cache[6] = {0};
static unsigned char radar_cache[9]= {0};
static unsigned char car_door_cache[2] = {0};
static unsigned int key_cache[3] = {0};


extern int updateAndReportRadar(void);
extern int updateAndReportAirConditon(void);
extern int updateAndReportCarDoor(void);

static int parse_ac_info(unsigned char *ac_data_buf, int buf_len)
{
    //ALOGE("==== ac_data_buf [0]:0x%02X  [1]:0x%02X  [2]:0x%02X  [3]:0x%02X  [4]:0x%02X\n",
    //    ac_data_buf[0], ac_data_buf[1], ac_data_buf[2], ac_data_buf[3], ac_data_buf[4]);
	// parse 1st byte
	ac_info.bPowerOn = (ac_data_buf[0]&0x80)>>7;
	ac_info.bAcOn = (ac_data_buf[0]&0x40)>>6;
	ac_info.bLoopMode = (ac_data_buf[0]&0x20)>>5;
	ac_info.bAuto = !!((ac_data_buf[0]&0x1F)>>3);
	ac_info.bDualOn = (ac_data_buf[0]&0x04)>>2;
	ac_info.bFrontOn = (ac_data_buf[0]&0x02)>>1;
	ac_info.bRearOn = ac_data_buf[0]&0x01;

	// parse 2nd byte
	switch ((ac_data_buf[1]&0xE0)>>5)
	{
		case 0:
			ac_info.fanMode = CAN_FAN_MODE_NONE;
			break;
		case 1:
			ac_info.fanMode = CAN_FAN_MODE_DOWN;
			break;
		case 2:
			ac_info.fanMode = CAN_FAN_MODE_HORI;
			break;
		case 3:
			ac_info.fanMode = CAN_FAN_MODE_HORI_DOWN;
			break;
		case 4:
			ac_info.fanMode = CAN_FAN_MODE_UP;
			break;
		case 5:
			ac_info.fanMode = CAN_FAN_MODE_UP_DOWN;
			break;
		case 6:
			ac_info.fanMode = CAN_FAN_MODE_HORI_UP;
			break;
		case 7:
			ac_info.fanMode = CAN_FAN_MODE_HORI_UP_DOWN;
			break;
		default:
			break;
	}
    ac_info.bShowAcInfo = (ac_data_buf[1]&0x10) >> 4;
	ac_info.fanSpeed.iCurSpeed = ac_data_buf[1]&0x07;

	// parse 3rd byte, left temperature
	if (ac_data_buf[2] == 0)
	{
		ac_info.tempLeft = 0;
	}
	else if (ac_data_buf[2] < 0x11)
	{
		ac_info.tempLeft = 180+(ac_data_buf[2]-1)*5;
	}
	else if (ac_data_buf[2] == 0x1F)
	{
		ac_info.tempLeft = 0xFFFF;
	}

	// parse 4th byte, right temperature
	if (ac_data_buf[3] == 0)
	{
		ac_info.tempRight = 0;
	}
	else if (ac_data_buf[3] < 0x11)
	{
		ac_info.tempRight = 180+(ac_data_buf[3]-1)*5;
	}
	else if (ac_data_buf[3] == 0x1F)
	{
		ac_info.tempRight = 0xFFFF;
	}

	// parse 5th byte,
	ac_info.bAQS = (ac_data_buf[4]&0x80)>>7;
	ac_info.nLeftSeatHeated = (ac_data_buf[4]&0x70)>>4;
	ac_info.bShowLeftSeatHeated = !!ac_info.nLeftSeatHeated;
	ac_info.bRearLock = (ac_data_buf[4]&0x08)>>3;
	ac_info.bAcMax = (ac_data_buf[4]&0x04)>>2;
    ac_info.nRightSeatHeated = ac_data_buf[4] & 0x03;
    ac_info.bShowRightSeatHeated = !!ac_info.nRightSeatHeated;

	return 0;
}


int process_canbus_command_volkaswagen(unsigned char *buf, int frame_len)
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
				parse_ac_info(&ac_cache[1], 5);
				//nativeReportAirCondition();
                //updateAndReportAirConditon(&ac_cache[1], 5);
                updateAndReportAirConditon();
			}
			break;
        case TYPE_RESPONE_RADAR:
			/*
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
            */
            if (memcmp(radar_cache, &buf[3], 8))
            {
				memcpy(radar_cache, &buf[3], 8);
				radar_info.front_left = buf[3];
				radar_info.front_right = buf[4];
				radar_info.rear_left = buf[5];
				radar_info.rear_right = buf[6];
				radar_info.front_center_left = buf[7];
				radar_info.front_center_right = buf[8];
				radar_info.rear_center_left = buf[9];
				radar_info.rear_center_right = buf[10];
				updateAndReportRadar();
            }
            break;
        case TYPE_RESPONE_WHEELKEYCODE:
            memset(key_cache, 0x00, sizeof(key_cache));
            key_cache[0] = 2; // make the key valid.
            key_cache[1] = (buf[3] & 0xFF)<<8; // make the min difference of two key to 256
            key_cache[2] = buf[4]; // the key's status(down or up);
            ioctl(canbus_fd, CANBUS_IOCTL_KEY_INFO, &key_cache[0]);
            break;
        case TYPE_RESPONE_CARDOORINFO:
			/*
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
			*/
			if (car_door_cache[0] != buf[3])
			{
				car_door_cache[0] = buf[3];
				door_info.front_left = buf[3]&0x01;
				door_info.front_right = !!(buf[3]&0x02);
				door_info.rear_left = !!(buf[3]&0x04);
				door_info.rear_right = !!(buf[3]&0x08);
				updateAndReportCarDoor();
			}
            
            break;
		default:
			break;
	}
	return 0;
}

} // namespace android

