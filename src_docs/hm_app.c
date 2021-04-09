#include "hm_app.h"

GWSTATFORTIME gwStateForRealTime;
DEV_OTA_STATION otaStationInfo;
DEV_OTA_STATION otaStationInfo;
uint8_t devOtaUpdateingFlag;
WIFICMDWATIRSP wifiCmdWaitRspBuf[SPECIALCMDNUM];
uint8_t zcXmodemStation;
uint8_t otaCompleteFlag;
unsigned char FirstPowerOnZBOTAFlag;
DEV_MYSELF_INFO hmIotsDevInfo;

void XmWriteBuf(unsigned char *data, uint8_t len)
{
    printf("Call XmWriteBuf\r\n");
}

HM_DEV_RAM_LIST *findInDevListByNwkadd()
{
    printf("Call findInDevListByNwkadd\r\n");
    return 0;
}

void userDeleteOneDevFromRamAndFlash(HM_DEV_RAM_LIST *devList)
{
    printf("Call userDeleteOneDevFromRamAndFlash\r\n");
}

void hmzColorRgbToXY(uint16_t r, uint16_t g, uint16_t b, uint16_t *color_x, uint16_t *color_y, uint8_t tempLevel)
{
    printf("Call hmzColorRgbToXY\r\n");
}

void writeNodeStateAlarnToCloud(HM_DEV_RAM_LIST *devList, uint8_t param1, uint8_t param2)
{
    print("Call writeNodeStateAlarnToCloud\r\n");
}

void StopTimer(int timer)
{
    print("Call StopTimer\r\n");
}

int StartTimer(uint8_t duration, uint8_t param1, void *callback)
{
    print("Call StartTimer\r\n");
}

uint8_t get_subdev_logon_complete()
{
    print("Call get_subdev_logon_complete\r\n");
}

int main()
{
    uart_work_task_init();
    return 0;
}