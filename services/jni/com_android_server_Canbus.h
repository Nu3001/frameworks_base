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

// �ͷ�ģʽ
typedef enum _CAN_FAN_MODE
{
	CAN_FAN_MODE_NONE = 0,				// �޷�
	CAN_FAN_MODE_HORI,					// ƽ���ͷ�
	CAN_FAN_MODE_UP,					// �����ͷ�(�絲�ͷ�)
	CAN_FAN_MODE_DOWN,					// �����ͷ�(�Ȳ��ͷ�)
	CAN_FAN_MODE_HORI_DOWN,				// ƽ���������ͷ�
	CAN_FAN_MODE_UP_DOWN,				// �����������ͷ�
	CAN_FAN_MODE_HORI_UP,				// ƽ���������ͷ�
	CAN_FAN_MODE_HORI_UP_DOWN,			// ƽ�С������������ͷ�
	CAN_FAN_MODE_AUTO = 100				// �Զ��ͷ�ģʽ
}CAN_FAN_MODE;

// ����
typedef struct _CAN_FAN_SPEED_LEVEL
{
	int iCurSpeed;      // ��ǰ�ٶȣ�0xffff �Զ�����
	int iMaxSpeed;      // ����ٶ�
}CAN_FAN_SPEEDLEVEL,*PCAN_FAN_SPEEDLEVEL;


// �յ���Ϣ
typedef struct _CAN_AC_INFO
{
    int bShowAcInfo;         // true ��ʾ�յ���Ϣ��false ����ʾ�յ���Ϣ
	int bPowerOn;            // true �յ�����false �յ���
	int bAcOn;               // A/Cָʾ(�յ�ѹ��������ָʾ)
	int bAuto;               // AUTOָʾ(Auto��С����һ������ָʾ����λ)
	int bDeicerOn;           // ������ָʾ
	int bDualOn;             // DUALָʾ
	int bSwingOn;            // SWINGָʾ(�ʹ�ר��)
	int bKafunOn;            // KAFUNָʾ(�ʹ�ר��)
	int bFrontOn;            // FRONTָʾ
	int bRearOn;             // REARָʾ
	int bIonOn;              // IONָʾ(����)
	int bLoopMode;			 // 1:��ѭ��,0:��ѭ��ָʾ
	int bAQS;                // AQS��ѭ��ָʾ
	int bRearLock;           // �����յ�����
	int bAcMax;              // ACMAXָʾ���յ�����ֵ��Ϊ���
	CAN_FAN_MODE fanMode;         // �ͷ�ģʽ
	CAN_FAN_SPEEDLEVEL fanSpeed;  // ����
	int bShowLeftTemp;       // �Ƿ���ʾ���¶�
	unsigned int     tempLeft;            // ���¶�,0X0000 LO,0XFFFF HI
	int bShowRightTemp;      // �Ƿ���ʾ���¶�
	unsigned int     tempRight;           // ���¶�,0X0000 LO,OXFFFF HI
	int bShowOutdoorTemp;    // �Ƿ���ʾ�����¶�
	int tempOutDoor;         // �����¶�(CAN_INVALID_VALUE ��ʾ��Ч���ݻ�֧��)
	int bShowLeftSeatHeated; // �Ƿ���ʾ�����μ���
	unsigned int     nLeftSeatHeated;     // �����μ����¶ȵȼ���1-3��
	int bShowRightSeatHeated;// �Ƿ���ʾ�����μ���
	unsigned int     nRightSeatHeated;    // �����μ����¶ȵȼ���1-3��
	//int bEcoOn;              // ECOָʾ(��֪�����ĸ�����ʹ�õ�)
	//int bZoneOn;             // ZONEָʾ(��֪�����ĸ�����ʹ�õ�)
	//int bAutoFanSpeed;       // �Զ�����ָʾ(��֪�����ĸ�����ʹ�õ�)
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
	char trunk; 		// ����
	char engine_cabin;	// �������ָ�
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
	char width_lamp;		// С��(ʾ���) 1: on; 0: off
	char dipped_light;		// �����
	char high_beam; 		// Զ���
	char emergency_lamp;	// ˫����ʾ��
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
