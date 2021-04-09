#include <stdint.h>
#include "hm_uartCmdExecute.h"
#include "hm_otaDefine.h"

void XmWriteBuf(unsigned char *data, uint8_t len);

HM_DEV_RAM_LIST *findInDevListByNwkadd();

void userDeleteOneDevFromRamAndFlash(HM_DEV_RAM_LIST *devList);

void hmzColorRgbToXY(uint16_t r, uint16_t g, uint16_t b, uint16_t *color_x, uint16_t *color_y, uint8_t tempLevel);

void writeNodeStateAlarnToCloud(HM_DEV_RAM_LIST *devList, uint8_t param1, uint8_t param2);

void StopTimer(int timer);

int StartTimer(uint8_t duration, uint8_t param1, void *callback);

uint8_t get_subdev_logon_complete();