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
// below data type for sonata8, canbox vendor is HECHI
#define TYPE_SET_SONATA_FAD			0x03	// front and rear balance
#define TYPE_SET_SONATA_BAL			0x04	// left and right balance
#define TYPE_SET_SONATA_VOL			0x05
#define TYPE_SET_SONATA_BASS		0x06
#define TYPE_SET_SONATA_MID			0x07
#define TYPE_SET_SONATA_TRE			0x08
#define TYPE_SET_SONATA_ONOFF		0x0B
#define TYPE_REQUEST_SONATA_INFO	0x0F
#define TYPE_RESPONE_SONATA_KEY		0x83
#define TYPE_RESPONE_SONATA_TEMP	0x84
#define TYPE_RESPONE_SONATA_INFO	0x85
//===================================================================

extern int updateAndReportSonata8(tSonataCarInfo * sonata8Info, int len);
extern int canbus_fd;

static unsigned int key_cache[3] = {0};
tSonataCarInfo sonata_car_info;

int process_canbus_command_sonata8(unsigned char *buf, int frame_len)
{
	LOGD("myu process_canbus_command_sonata8 !!!");
	switch(buf[1])
	{
		case TYPE_RESPONE_SONATA_KEY:
			if (buf[3])
			{
				key_cache[0] = 2; 					// make the key valid.
				key_cache[1] = (buf[3] & 0xFF)<<8;	// make the min difference of two key to 256
				key_cache[2] = 1; 					// the key's status:down;
				ioctl(canbus_fd, CANBUS_IOCTL_KEY_INFO, &key_cache[0]);
			}
			else
			{
				// if we have received no key pressed, it means key release 
				key_cache[0] = 2; 					// make the key valid.
				key_cache[1] = (buf[3] & 0xFF)<<8;	// make the min difference of two key to 256
				key_cache[2] = 0; 					// the key's status:down;
				ioctl(canbus_fd, CANBUS_IOCTL_KEY_INFO, &key_cache[0]);
			}
			break;
		case TYPE_RESPONE_SONATA_TEMP:
			sonata_car_info.outdoor_temp = buf[3];
			break;
		case TYPE_RESPONE_SONATA_INFO:
			memcpy(&sonata_car_info, &buf[3], 8);
			LOGD("myu sonata_car_info.fad_val:%d  sonata_car_info.bal_val:%d  sonata_car_info.bass_val:%d  sonata_car_info.mid_val:%d sonata_car_info.tre_val:%d sonata_car_info.vol:%d "
						, sonata_car_info.fad_val, sonata_car_info.bal_val, sonata_car_info.bass_val,  sonata_car_info.mid_val, sonata_car_info.tre_val, sonata_car_info.vol);
			updateAndReportSonata8(&sonata_car_info,8);
			break;
		default:
			break;
	}
	return 0;
}

} // namespace android

