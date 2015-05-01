#ifndef _ANDROID_SERVER_CANBUS_H
#define _ANDROID_SERVER_CANBUS_H

namespace android {

#define CANBUS_DEV     				"/dev/bonovo_canbus"    
#define CANBUS_BUF_SIZE 			4096
#define CANBUS_FRAME_SIZE			64
#define DEV_MAJOR			        236
#define CANBUS_IOCTL_KEY_INFO		_IO(DEV_MAJOR, 0)
#define CANBUS_IOCTL_MCU_UART_DEV	_IO(DEV_MAJOR, 1)
#define CANBUS_IOCTL_MCU_UART_38400	_IO(DEV_MAJOR, 2)

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

typedef enum
{
	eCarTypeVolkaswagen = 0,
	eCarTypeSonata8,
	eCarTypeSonata9,
	eCarTypeNum
}tCarType;

// enum of canbus decode box vendor
// 2015-4-13
typedef enum
{
	eCanBoxVendorBonovo = 0,	
	eCanBoxVendorHechi,
	eCanBoxVendorXinpu,
	eCanboxVendorXinbasi
}tCanBoxVendor;

// door info, 0 for close, 1 for open
typedef struct
{
	char front_left;
	char front_right;
	char rear_left;
	char rear_right;
	char trunk; 		// 后备箱
	char engine_cabin;	// 发动机仓盖
}tCarDoorInfo;

// the information about gear status
typedef struct
{
	char reverse_gear;		// 1: reverse state; 0: non-reverse state(neutral, drive)
	char park_gear; 		// 1: park state; 0: non-park stat
}tCarGearInfo;


// the information about lamp
typedef struct
{
	char auto_light;
	char width_lamp;		// 小灯(示宽灯) 1: on; 0: off
	char dipped_light;		// 近光灯
	char high_beam; 		// 远光灯
	char emergency_lamp;	// 双闪警示灯
	char left_turn_lamp;
	char right_turn_lamp;
	char front_fog_lamp;
	char rear_fog_lamp;
}tCarLampInfo;

// the information about radar
typedef struct
{
	// front 4 radars
	char front_left;
	char front_center_left;
	char front_center_right;
	char front_right;
	// rear 4 radars
	char rear_left;
	char rear_center_left;
	char rear_center_right;
	char rear_right;
	// left 2 radars
	char left_front;
	char left_rear;
	// right 2 radars
	char right_front;
	char right_rear;
}tCarRadarInfo;

typedef struct
{
	char outdoor_temp;
	char fad_val;		// front and rear balance
	char bal_val;		// left and right balance
	char vol;			// volume
	char bass_val;		// bass value
	char mid_val;		// middle value
	char tre_val;		// treble value
	char power_status;	// 
}tSonataCarInfo;

// the information about volume and power amplifier
typedef struct
{
	char pwr_amp_detect;// 1:pwr_amp has been detected,
	char fad_val;		// front and rear balance
	char bal_val;		// left and right balance
	char vol;			// volume
	char bass_val;		// bass value
	char mid_val;		// middle value
	char tre_val;		// treble value
}tCarVolumeInfo;


struct canbus_buf_t
{
	unsigned char buf[CANBUS_BUF_SIZE];
	int w_idx;			// indicate the buffer that is written now
	int r_idx;			// indicate the buffer that is read now
	int valid_data_num;	// indicate how many valid data in the buffer
};

} // namespace android

#endif
