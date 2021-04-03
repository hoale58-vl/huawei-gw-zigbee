#ifndef __HM_UARTCMDEXECUTE_H__
#define __HM_UARTCMDEXECUTE_H__

#define UART_PACKET_BASIC_LEN 1
#define UART_DATA_HEAD 0xfa
#define UART_DATA_END 0xf5

#define MONITOR_FIND_FOR_ZC_SHORTADDR 0
#define MONITOR_FINE_FOR_ANY_SHORTADDR 0xffff

#include <stdint.h>

//#define ZIGBEE_MODEID_LEN	16

typedef struct
{
	uint8_t permitJoinFlag;
	uint8_t userId[32];
	uint8_t userName[32];
} PERMITJOINACTION;

enum
{
	MONITOR_SUB_CMD_ONOFF = 0x01,
	MONITOR_SUB_CMD_LEVEL,
	MONITOR_SUB_CMD_COLOR,
	MONITOR_SUB_CMD_CUTRGB
};

typedef struct
{
	uint8_t head;
	uint8_t firmwareValid;
	uint8_t devUpdateNeedFlag;
	uint8_t flashSaveImageType;
	uint8_t devImageVersion;
	uint8_t needDownloadImageFromCloud;
	uint32_t devImageSize;
	uint8_t end;
} DEV_OTA_STATION;

typedef struct
{
	int16_t temp;
} TEMP_STA;

typedef struct
{
	uint16_t humi;
} HUMI_STA;

typedef struct
{
	uint8_t onOffStatus;
	uint16_t rmsVoltage;
	int32_t InstantaneousDemand;
	uint64_t CurrentSummationDelivered;
} SMARTPLUG_STA;

typedef struct
{
	uint8_t onOffStatus;
	uint8_t level;
	uint8_t hue;
	uint16_t color_x;
	uint16_t color_y;
	uint16_t color_temp;
} COLOR_STA;

typedef struct
{
	uint8_t onOffStatus;
	uint8_t level;
} DIMMER_STA;

typedef struct
{
	uint8_t onOffStatus;
} SWITCH_STA;

typedef struct
{
	uint8_t zoneId;
	uint16_t zoneType;
	uint16_t zoneStatus;
} ZONE_STA;

typedef struct
{
	uint8_t comId;
	uint8_t modeId;
} ACE_STA;

typedef struct
{
	uint8_t onoffStatus;
	uint8_t brightness;
	uint16_t colorTemrature;
} CSTRGB_STA;

typedef struct
{
	uint8_t doorState;
	uint8_t remoteOpenDoorEnable;
} DOOR_STA;

typedef struct
{
	uint8_t onOffStatus;
	uint8_t level;
} CURTAIN_MOTOR_APP;

//����zigbee�豸�ϱ�ʱ
typedef union
{
	TEMP_STA tempValue;
	HUMI_STA humiValue;
	SMARTPLUG_STA splugValue;
	COLOR_STA colorValue;
	DIMMER_STA dimmerValue;
	SWITCH_STA witchValue;
	SWITCH_STA onoffOutValue;
	ZONE_STA zoneValue;
	ACE_STA aceValue;
	CSTRGB_STA cstRgbValue;
	DOOR_STA doorValue;
	CURTAIN_MOTOR_APP curtainMoValue;
	AIR_STA_APP airValue;
} DEV_DATA;

typedef uint8_t EmberEUI64[8];

typedef struct
{
	uint8_t macAddr[8];
	uint8_t netState;
	uint8_t batteryRemain;
	uint16_t deviceId;
	uint16_t shortAddr;
	uint8_t epId;
	uint8_t modeId[ZIGBEE_MODEID_LEN];
	DEV_DATA devData;
} UART_UPDATE_NEW_DEV_DATA_INFO;

typedef struct
{
	uint8_t valid;
	uint8_t zbCmd;
	uint8_t zbEp;
	uint16_t zbClusterId;
	uint16_t zbShortAddr;
	uint32_t cmdSn;
	uint32_t rcvTimems;
	uint32_t setData;
	uint8_t appUserId[12];
	uint8_t appUserName[32];
} WIFICMDWATIRSP;
#define SPECIALCMDNUM 3

typedef struct
{
	uint8_t acucounts;
	uint8_t settimecounts;
	uint8_t setTAlarmflag;
	uint8_t setTAlarmcounts;
	uint8_t setHAlarmflag;
	uint8_t setHAlarmcounts;
} AIRTIMERINFO;

//���崮��cmd����
#define UART_CMD_PERMIT_JOIN_REQ 0x11
#define UART_CMD_PERMIT_JOIN_RSP 0x12
#define UART_CMD_NEW_DEV_IDCT 0x10
#define UART_CMD_DEV_INFO_REQ 0x13
#define UART_CMD_DEV_INFO_RSP 0x14
#define UART_CMD_READ_ZC_INFO_REQ 0x15
#define UART_CMD_READ_ZC_INFO_RSP 0x16
#define UART_CMD_LEAVE_DEV_REQ 0x17
#define UART_CMD_LEAVE_DEV_RSP 0x18
#define UART_CMD_DEV_LEAVE_IDCT 0x1a
#define UART_CMD_UPDATE_ZC_SOFT_REQ 0x1b
#define UART_CMD_UPDATE_ZC_SOFT_RSP 0x1c
#define UART_CMD_SET_ZC_TO_FACTORY_REQ 0x1d
#define UART_CMD_SET_ZC_TO_FACTORY_RSP 0x1e

#define UART_CMD_SET_RSSI_REPORT_REQ 0x21
#define UART_CMD_SET_RSSI_REPORT_RSP 0x22
#define UART_CMD_DEV_RSSI_IDCT 0x24
#define UART_CMD_SEND_ZC_HEART_REQ 0x25
#define UART_CMD_SEND_ZC_HEART_RSP 0x26

#define UART_CMD_WRITE_ATTR_REQ 0x31
#define UART_CMD_WRITE_ATTR_RSP 0x32
#define UART_CMD_READ_ATTR_REQ 0x33
#define UART_CMD_READ_ATTR_RSP 0x34
#define UART_CMD_ATTR_DATA_IDCT 0x36
#define UART_CMD_ATTR_REPORT_IDCT 0x38
#define UART_CMD_GET_ACTIVE_EP_REQ 0x39
#define UART_CMD_GET_ACTIVE_EP_RSP 0x3a
#define UART_CMD_GET_EP_SMPDST_REQ 0x3b
#define UART_CMD_GET_EP_SMPDST_RSP 0x3c
#define UART_CMD_CFG_ATTR_REPORT_REQ 0x3d
#define UART_CMD_CFG_ATTR_REPORT_RSP 0x3e
#define UART_CMD_CFG_ATTR_REPORT_IDCT 0x40
#define UART_CMD_GET_NODE_DESCRIPTOR_REQ 0x41
#define UART_CMD_GET_NODE_DESCRIPTOR_RSP 0x42
#define UART_CMD_GET_NODE_DESCRIPTOR_IDCT 0x44
#define UART_CMD_BIND_REQ 0x45 //ռλ������,�����ϱ�����ʱ�䣬��д�����ȵ�cmdдʱ��
#define UART_CMD_BIND_RSP 0x46
#define UART_CMD_UNBIND_REQ 0x47
#define UART_CMD_UNBIND_RSP 0x48
#define UART_CMD_PRIVATEBIND_REQ 0x50
#define UART_CMD_PRIVATEBIND_RSP 0x51

#define UART_CMD_DEV_UPDATE_SHORT_ADDR_IDCT 0x60
#define UART_CMD_DEV_OFFLINE_IDCT 0x62
#define UART_CMD_IAS_ZONE_ALARM_IDCT 0x64
#define UART_CMD_DEV_ALARM_IDCT 0x66
#define UART_CMD_IAS_ACE_COM_IDCT 0x68
#define UART_CMD_DOOR_OPERATION_EVENT_IDCT 0x6a
#define UART_CMD_SCENE_ONOFF_REPORT_IDCT 0x6c

#define UART_CMD_ONOFF_REPORT_IDCT 0x80
#define UART_CMD_ONOFF_CONTROL_REQ 0x81
#define UART_CMD_ONOFF_CONTROL_RSP 0x82
#define UART_CMD_LEVEL_CONTROL_REQ 0x83
#define UART_CMD_LEVEL_CONTROL_RSP 0x84
#define UART_CMD_WD_CONTROL_REQ 0x85
#define UART_CMD_WD_CONTROL_RSP 0x86
#define UART_CMD_IDENTIFY_REQ 0x87
#define UART_CMD_IDENTIFY_RSP 0x88
#define UART_CMD_COLOR_CONTROL_REQ 0x89
#define UART_CMD_COLOR_CONTROL_RSP 0x8a

#define UART_CMD_GROUP_CONTROL_REQ 0x8b
#define UART_CMD_GROUP_CONTROL_RSP 0x8c

#define UART_CMD_RGBLIGHT_CONTROL_REQ 0x8d
#define UART_CMD_RGBLIGHT_CONTROL_RSP 0x8e

#define UART_CMD_DOOR_UNLOCK_CONTROL_REQ 0x91
#define UART_CMD_DOOR_UNLOCK_CONTROL_RSP 0x92
#define UART_CMD_CURTAIN_MOTOR_CLEAR_PHY_LIMIT_REQ 0x93
#define UART_CMD_CURTAIN_MOTOR_CLEAR_PHY_LIMIT_RSP 0x94
#define UART_CMD_AIR_CONTROL_REQ 0x95
#define UART_CMD_AIR_CONTROL_RSP 0x96

#define UART_CMD_ONOFF_CONTROL_BY_GROUP_REQ 0xc1
#define UART_CMD_ONOFF_CONTROL_BY_GROUP_RSP 0xc2
#define UART_CMD_LEVEL_CONTROL_BY_GROUP_REQ 0xc3
#define UART_CMD_LEVEL_CONTROL_BY_GROUP_RSP 0xc4

#define UART_CMD_ILLUMINANCE_DATA_REQ 0xdf //����
#define UART_CMD_ILLUMINANCE_DATA_REP 0xde

//end uart cmd define

#define UART_DEAL_DAY_STATUS 1
#define UART_DEAL_NIGHT_STATUS 0

//cmd deal status
#define UART_DEAL_STA_SUCCESS 0
#define UART_DEAL_STA_NO_DEV 1
#define UART_DEAL_STA_CMD_ERROR 2
#define UART_DEAL_STA_DEV_OFFLINE 3
#define UART_DEAL_STA_SOFT_VERSION_NOT_MATCH 4
#define UART_DEAL_STA_UNKNOW_ERROR 18
#define UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS 0xa0

#define MAX_UART_TX_CMD_BUF_NUM 10
#define UART_FIND_BUF_INVALID_INDEX 0xff

#define MAX_UART_CMD_MONITOR_NUM 8

#define MAX_UART_RX_BUF_LENTH 256
#define MAX_UART_RX_BUF_NUM 2

typedef enum
{
	UART_WORK_COMMUNICATION,
	UART_WORK_OTA
} UARTWORKTYPE;

typedef enum
{
	ZB_INVALID_DATA = 0,
	ZB_NORMAL_DATA = 1,
	ZB_IMPORTANT_DATA = 2
} ZB_DATA_TYPE;

typedef struct
{
	uint8_t data[MAX_UART_RX_BUF_LENTH];
	ZB_DATA_TYPE validFlag;
} UART_RX_DATA_BUF;

typedef struct
{
	uint8_t inData[20];
	uint8_t inDataLen;
} INPAYLOAD;

typedef struct
{
	uint8_t inCmd;
	uint8_t dstEp; //ep��daar type���ã���incmdΪ�鲥/�㲥���Ƶ�ʱ�򣬸�λ��ʾaddr type
	uint16_t dstAddr;
	INPAYLOAD inPayload;
	ZB_DATA_TYPE cmdValid;
} UART_TX_CMD_BUF;

typedef struct
{
	uint8_t validFlag;
	uint8_t cmd;
	uint8_t ep;
	uint16_t shortAddr;
	uint8_t state;
	uint32_t sendTimeS;
	INPAYLOAD inPayload;
} UART_CTRL_CMD_BUF;

#define MAX_BACKUP_DATA_LEN 64
typedef struct
{
	uint8_t bkLen;
	uint8_t bkBuff[MAX_BACKUP_DATA_LEN];
} CMD_BACKUP;

typedef struct
{
	uint8_t validFlag;
	uint8_t cmd;
	uint8_t ep;
	uint16_t shortAddr;
	uint32_t sendTime100Ms;
	uint8_t sendCount;
	uint8_t rspRcvFlag;
	CMD_BACKUP cmdbk;
} UARTCMDMONITOR;

typedef struct
{
	uint32_t lastCommuTime100ms; //���
	uint8_t uartWorkDataType;
	uint8_t uartHeartSendCount;
} UARTHEARTINFO;

typedef enum
{
	ZB_NET_WORK_NORMAL = 0,
	ZB_NET_WORK_PERMIT_JOINING
} ZIGBEE_NET_STATION;

//define all light groups
#define ALL_LIGHT_GROUPS 12
#define GROUPS_ADDR 1

void transportUartHexDataForReadEnterTestMode(void);
void transportUartHexDataForReadIlluminance(void);

void userFillUartCmdForPermitJoin(uint8_t joinTime, uint8_t *userId, uint8_t *userName);
void fillUartCmdMonitor(uint8_t cmd, uint16_t shortAddr, uint8_t ep, uint8_t *cpcmdData, uint8_t cpcmdLen);

void uart_work_task_init(void);

extern void *ota_thread(void *unused);

#endif
