/********************************************************************
filename: 	com_android_server_Rds.h
created:	2012-04-24
author:		zfhu

purpose:	RDS类型定义
*********************************************************************/
#ifndef _RDS_DEF_
#define _RDS_DEF_
//#include <WinDef.h>

//　节目标识 Program Identification
typedef struct RDS_PI_
{
	unsigned char countryCode;
	unsigned char areaCode;
    unsigned char referNum;
}RDS_PI;

typedef unsigned char RDS_TP;  //　交通节目标识Traffic Program Identification
typedef unsigned char RDS_TA;  //　交通公告标识Traffic Announcement Identification
typedef unsigned char RDS_MS;  //　音乐/语言切换Music/Speech Switch
typedef unsigned char RDS_DI;  //　解码器标识Decoder Identification

typedef enum RDS_PTY_T_
{
    RDS_PTY_T_CHN = 0,
	RDS_PTY_T_EUP,
	RDS_PTY_T_USA
}RDS_PTY_T;

typedef enum RDS_PTY_CHN_
{
	PTY_CHN_NONE = 0, // 无节目类型或未定义
	PTY_CHN_NEWS,     // 新闻
	PTY_CHN_AFFARIS,  // 时事
	PTY_CHN_INFO,     // 信息
	PTY_CHN_SPORT,    // 体育
	PTY_CHN_EDUCATE,  // 教育
	PTY_CHN_LITERATE, // 文学
    PTY_CHN_SCIENCE,  // 科技
    PTY_CHN_VARIED,   // 综艺
	PTY_CHN_HOTLINE,  // 热线
    PTY_CHN_SPECIAL,  // 专题
	PTY_CHN_POPULARM, // 流行音乐
	PTY_CHN_SERIOUSM, // 严肃音乐
	PTY_CHN_LIGHTM,   // 轻松音乐
	PTY_CHN_NATIONM,  // 民族音乐
	PTY_CHN_OPERA,    // 戏曲
	PTY_CHN_OTHERM,   // 其它音乐
	PTY_CHN_ALERM = 31// 紧急报警系统
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

//　节目类型Program Type
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

//　节目类型名称 Program Type Name
typedef struct RDS_PTYN_
{
	char name[8];
}RDS_PTYN;

// 替换频率表 list of Alternative Frequencies
typedef struct RDS_AF_
{
	unsigned int bAFExist;    // AF是否存在
	unsigned int listCount;
	unsigned int freq[25];
}RDS_AF;

// 节目名称 Program Service Name
typedef struct RDS_PS_ 
{
	char name[8];
}RDS_PS;

// 节目栏目号 Program Item Number
typedef  struct RDS_PIN_
{
	unsigned char day;
	unsigned char hour;
    unsigned char minute;
}RDS_PIN;

// 收音机文本信息 Radio Text
typedef struct RDS_RT_
{
	char radioText[64];
}RDS_RT;

// 时间和日期 Clock Time and Date
typedef struct RDS_CT_
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
}RDS_CT;

// 组类型
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

// RDS消息类型
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

// 定义与UI层通信消息结构体
typedef struct _RDS_MSG_STRU
{
	unsigned short      nPI;                         // 节目标识（16位PI数据）
	unsigned int     dwMsgType;	               // 消息类型,参见RDS_INFO_TYPE
	int       dataSize;                    // 数据长度
	unsigned char  data[MAX_RDS_DATA_SIZE];// 具体数据（大小由dataSize指定）
}RDS_MSG_STRU,*PRDS_MSG_STRU;

#endif
