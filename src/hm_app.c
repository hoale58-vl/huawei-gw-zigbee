#include "hm_app.h"
#include "hm_uartCmdExecute.h"

GWSTATFORTIME gwStateForRealTime;
DEV_OTA_STATION otaStationInfo;
DEV_OTA_STATION otaStationInfo;
uint8_t devOtaUpdateingFlag;

GWSTATFORTIME gwStateForRealTime;
WIFICMDWATIRSP wifiCmdWaitRspBuf[SPECIALCMDNUM];
uint8_t zcXmodemStation;

uint8_t otaCompleteFlag;
unsigned char FirstPowerOnZBOTAFlag;
DEV_MYSELF_INFO hmIotsDevInfo;

int main()
{
    uart_work_task_init();
    return 0;
}