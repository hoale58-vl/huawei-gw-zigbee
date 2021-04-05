#define NO_DEV_NEED_UPDATE_IMAGE 1

#include <stdint.h>

typedef struct
{
    uint8_t appAllowSoftWareUpdate;
    uint8_t haveNewestSoftWare;
} WIFI_SOFT_UPDATE;

typedef struct
{
    uint8_t zcSoftVersion;
    uint8_t channel;
    uint8_t panId;
    uint8_t zbMac;
} ZIGBEE_ZC_BASIC_INFO;

typedef struct
{
    uint8_t echoNeedUpdateFlag;
    uint8_t gwZbIsJoining;
    uint8_t gwIsAlarning;
    uint32_t sysRealTimeFors;
    WIFI_SOFT_UPDATE wifiSoftUpdate;
    ZIGBEE_ZC_BASIC_INFO zigbeeZcBasicInfo;
} GWSTATFORTIME;

typedef struct
{
    uint8_t ServerConnected;
} DEV_MYSELF_INFO;

typedef struct
{
    uint8_t mac[8];
    uint8_t devType;
    uint8_t index;
    uint8_t shortAddr;
    uint8_t onOffLineState;
    uint8_t onLine;
    EP_DEV epList[1];
    uint32_t lastCommunicationTimes;
} HM_DEV_RAM_LIST;

typedef struct
{
    uint32_t times;
} EP_DEV;

void execute_player_voice_thread();