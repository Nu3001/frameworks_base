/********************************************************************
filename: 	com_android_server_Rds.h
created:	2012-04-24
author:		zfhu

purpose:	RDS���Ͷ���
*********************************************************************/
#ifndef _RDS_DEF_
#define _RDS_DEF_
//#include <WinDef.h>

//����Ŀ��ʶ Program Identification
typedef struct RDS_PI_
{
	unsigned char countryCode;
	unsigned char areaCode;
    unsigned char referNum;
}RDS_PI;

typedef unsigned char RDS_TP;  //����ͨ��Ŀ��ʶTraffic Program Identification
typedef unsigned char RDS_TA;  //����ͨ�����ʶTraffic Announcement Identification
typedef unsigned char RDS_MS;  //������/�����л�Music/Speech Switch
typedef unsigned char RDS_DI;  //����������ʶDecoder Identification

typedef enum RDS_PTY_T_
{
    RDS_PTY_T_CHN = 0,
	RDS_PTY_T_EUP,
	RDS_PTY_T_USA
}RDS_PTY_T;

typedef enum RDS_PTY_CHN_
{
	PTY_CHN_NONE = 0, // �޽�Ŀ���ͻ�δ����
	PTY_CHN_NEWS,     // ����
	PTY_CHN_AFFARIS,  // ʱ��
	PTY_CHN_INFO,     // ��Ϣ
	PTY_CHN_SPORT,    // ����
	PTY_CHN_EDUCATE,  // ����
	PTY_CHN_LITERATE, // ��ѧ
    PTY_CHN_SCIENCE,  // �Ƽ�
    PTY_CHN_VARIED,   // ����
	PTY_CHN_HOTLINE,  // ����
    PTY_CHN_SPECIAL,  // ר��
	PTY_CHN_POPULARM, // ��������
	PTY_CHN_SERIOUSM, // ��������
	PTY_CHN_LIGHTM,   // ��������
	PTY_CHN_NATIONM,  // ��������
	PTY_CHN_OPERA,    // Ϸ��
	PTY_CHN_OTHERM,   // ��������
	PTY_CHN_ALERM = 31// ��������ϵͳ
}RDS_PTY_CHN;

typedef enum RDS_PTY_USA_
{
	PTY_USA_NONE = 0,
	PTY_USA_NEWS,
	PTY_USA_INFO,
	PTY_USA_SPORTS,
	PTY_USA_TALK,
	PTY_USA_ROCK,
    PTY_USA_CLSROCK,
	PTY_USA_ADLTHITS,
	PTY_USA_SOFTHITS,
	PTY_USA_TOP40,
	PTY_USA_COUNTRY,
	PTY_USA_OLDIES,
	PTY_USA_SOFT,
	PTY_USA_NOSTALGA, 
	PTY_USA_JAZZ,  
	PTY_USA_CLASSICL, 
	PTY_USA_RANDB, 
	PTY_USA_SOFTRANDB,
	PTY_USA_LANGUAGE,
	PTY_USA_RELMUSIC, 
	PTY_USA_RELTALK, 
	PTY_USA_PERSNLTY,  
	PTY_USA_PBULIC,
	PTY_USA_COLLEGE,
	PTY_USA_HABLESP,
	PTY_USA_MUSICESP,
	PTY_USA_HIPHOP,
	PTY_USA_WEATHER = 29,
	PTY_USA_TEST = 30,
	PTY_USA_ALERT = 31
}RDS_PTY_USA;

typedef enum RDS_PTY_EUP_
{
    PTY_EUP_NONE  = 0,
	PTY_EUP_NEWS,
	PTY_EUP_AFFARIS,
	PTY_EUP_INFO,
	PTY_EUP_SPORT,
	PTY_EUP_EDUCATE,
	PTY_EUP_DRAMA,
	PTY_EUP_CULTURE,
	PTY_EUP_SCIENCE,
	PTY_EUP_VARIED,
	PTY_EUP_POPM,
	PTY_EUP_ROCKM,
	PTY_EUP_EASYM,
	PTY_EUP_LIGHTM,
	PTY_EUP_CLASSICS,
	PTY_EUP_OTHERM,
    PTY_EUP_WEATHER,
	PTY_EUP_FINANCE,
	PTY_EUP_CHILDREN,
	PTY_EUP_SOCIAL,
	PTY_EUP_RELIGION,
	PTY_EUP_PHONEIN,
	PTY_EUP_TRAVEL,
	PTY_EUP_LEISURE,
	PTY_EUP_JAZZ,
	PTY_EUP_COUNTRY,
	PTY_EUP_NATIONM,
	PTY_EUP_OLDIES,
	PTY_EUP_FOLKM,
	PTY_EUP_DOCUMENT,
	PTY_EUP_TEST,
	PTY_EUP_ALARM,
}RDS_PTY_EUP;

//����Ŀ����Program Type
typedef struct RDS_PTY_
{
	RDS_PTY_T type;
	union
	{
		RDS_PTY_CHN  ptyCHN;
		RDS_PTY_USA  ptyUSA;
		RDS_PTY_EUP  ptyEUP;
	};
}RDS_PTY;

//����Ŀ�������� Program Type Name
typedef struct RDS_PTYN_
{
	char name[8];
}RDS_PTYN;

// �滻Ƶ�ʱ� list of Alternative Frequencies
typedef struct RDS_AF_
{
	unsigned int bAFExist;    // AF�Ƿ����
	unsigned int listCount;
	unsigned int freq[25];
}RDS_AF;

// ��Ŀ���� Program Service Name
typedef struct RDS_PS_ 
{
	char name[8];
}RDS_PS;

// ��Ŀ��Ŀ�� Program Item Number
typedef  struct RDS_PIN_
{
	unsigned char day;
	unsigned char hour;
    unsigned char minute;
}RDS_PIN;

// �������ı���Ϣ Radio Text
typedef struct RDS_RT_
{
	char radioText[64];
}RDS_RT;

// ʱ������� Clock Time and Date
typedef struct RDS_CT_
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
}RDS_CT;

// ������
typedef enum RDS_GROUP_TYPE_
{
	GROUP_0A = 0,
	GROUP_0B,
	GROUP_1A,
	GROUP_1B,
	GROUP_2A,
	GROUP_2B,
	GROUP_3A,
	GROUP_3B,
	GROUP_4A,
	GROUP_4B,
	GROUP_5A,
	GROUP_5B,
	GROUP_6A,
	GROUP_6B,
	GROUP_7A,
	GROUP_7B,
	GROUP_8A,
	GROUP_8B,
	GROUP_9A,
	GROUP_9B,
	GROUP_10A,
	GROUP_10B,
	GROUP_11A,
	GROUP_11B,
	GROUP_12A,
	GROUP_12B,
	GROUP_13A,
	GROUP_13B,
	GROUP_14A,
	GROUP_14B,
	GROUP_15A,
	GROUP_15B,
	GROUP_UNKNOWN
}RDS_GROUP_TYPE;

// RDS��Ϣ����
typedef enum RDS_INFO_TYPE_
{
    RDS_INFO_QUAL,
	RDS_INFO_PI,
	RDS_INFO_TP,
	RDS_INFO_TA,
	RDS_INFO_DI,
    RDS_INFO_AF,
	RDS_INFO_PTY,
	RDS_INFO_PTYN,
	RDS_INFO_PS,
	RDS_INFO_RT,
	RDS_INFO_CT
}RDS_INFO_TYPE;

#define MAX_RDS_DATA_SIZE 256

// ������UI��ͨ����Ϣ�ṹ��
typedef struct _RDS_MSG_STRU
{
	unsigned short      nPI;                         // ��Ŀ��ʶ��16λPI���ݣ�
	unsigned int     dwMsgType;	               // ��Ϣ����,�μ�RDS_INFO_TYPE
	int       dataSize;                    // ���ݳ���
	unsigned char  data[MAX_RDS_DATA_SIZE];// �������ݣ���С��dataSizeָ����
}RDS_MSG_STRU,*PRDS_MSG_STRU;

#endif
