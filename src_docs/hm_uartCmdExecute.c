#include "hm_uartCmdExecute.h"
#include "hm_otaDefine.h"

#define UART_DEBUG_ENABLE

#define HMDBUG_UART 1
#if HMDBUG_UART
#define prtlog(...) printf(__VA_ARGS__)
#else
#define prtlog(...)
#endif

uint8_t startOtaFlag;
uint32_t IlluminanceValue = 0;

PERMITJOINACTION permitJoinAction;
uint8_t userOtaZigbeeGWCheck;
UART_RX_DATA_BUF uartRXData[MAX_UART_RX_BUF_NUM];
UART_TX_CMD_BUF uartTXCmd[MAX_UART_TX_CMD_BUF_NUM];
UART_CTRL_CMD_BUF uartCTRLCmd[10];

uint8_t zigbeeNetWorkStation = ZB_NET_WORK_NORMAL;
uint8_t zigbeeNetPermitJoinTime;

UARTCMDMONITOR uartCmdMonito[MAX_UART_CMD_MONITOR_NUM]; //ָ���⻺�棬���ڲ�ѯ
UARTHEARTINFO uartHeartManage;							//UART ��������

extern uint8_t devOtaUpdateingFlag;

DEV_OTA_STATION otaStationInfo;
uint8_t setWDTimes = 0;
uint16_t WDShortAddr = 0;
int g_hTimerWDDelay = 0;
pthread_t uartSentWork_id;
pthread_t executePlayerWork_id;

uint8_t zbSysStartFlag = 2;

extern GWSTATFORTIME gwStateForRealTime;
extern WIFICMDWATIRSP wifiCmdWaitRspBuf[SPECIALCMDNUM];
extern uint8_t zcXmodemStation;

extern uint8_t otaCompleteFlag;
extern unsigned char FirstPowerOnZBOTAFlag;
extern DEV_MYSELF_INFO hmIotsDevInfo;

void BigSmallEndConversion(char *mac, char *t_mac)
{
	uint8_t i = 0, j = 0;
	uint8_t len;
	char tempMac[10] = {0};
	memcpy(tempMac, mac, 8);
	len = 7;
	for (i = len; i >= 0; i--)
	{
		//printf("i:%d,value:%02x\r\n",i,tempMac[i]);
		t_mac[j++] = tempMac[i];
		if (i == 0)
		{
			break;
		}
	}
}

void usermsleep(unsigned int ms)
{
	struct timespec tms;

	tms.tv_sec = ms / 1000;
	tms.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&tms, NULL);
}

static void userSendtest1(void)
{
	unsigned char cmdBuf[10] = {0xfa, 0x02, 0x11, 0xb4, 0xb4, 0xf5};
	unsigned char i, cmdBufLen;
	XmWriteBuf(cmdBuf, 6);
	printf("sendLen=%d\r\n", strlen(cmdBuf));
	printf("sendBuf:");
	cmdBufLen = strlen(cmdBuf);
	for (i = 0; i < cmdBufLen; i++)
	{
		printf("%02x ", cmdBuf[i]);
	}
	printf("\r\n");
}
static void userSendtest2(void)
{
	unsigned char cmdBuf[10] = {0xfa, 0x02, 0x11, 0x00, 0x00, 0xf5};
	unsigned char i, cmdBufLen;
	XmWriteBuf(cmdBuf, 6);
	printf("sendLen=%d\r\n", strlen(cmdBuf));
	printf("sendBuf:");
	cmdBufLen = 6;
	for (i = 0; i < cmdBufLen; i++)
	{
		printf("%02x ", cmdBuf[i]);
	}
	printf("\r\n");
}

static int userFillSpecialWatiRspCmdBuf(WIFICMDWATIRSP cmdsta)
{
	uint8_t i, tempIndex = 0xff;
	int ret = 0;
	for (i = 0; i < SPECIALCMDNUM; i++)
	{
		if (wifiCmdWaitRspBuf[i].valid == 0)
		{
			if (tempIndex == 0xff)
			{
				tempIndex = i;
			}
		}
		else if (wifiCmdWaitRspBuf[i].valid == 1)
		{ //���֮ǰ�л����ָ��
			if ((wifiCmdWaitRspBuf[i].zbCmd == cmdsta.zbCmd) && (wifiCmdWaitRspBuf[i].zbClusterId == cmdsta.zbClusterId) && (wifiCmdWaitRspBuf[i].zbEp == cmdsta.zbEp) && (wifiCmdWaitRspBuf[i].zbShortAddr == cmdsta.zbShortAddr))
			{
				wifiCmdWaitRspBuf[i].rcvTimems = gwStateForRealTime.sysRealTimeFors;
				return ret;
			}
		}
		else
		{
			wifiCmdWaitRspBuf[i].valid = 0;
			if (tempIndex == 0xff)
			{
				tempIndex = i;
			}
		}
	}
	if (tempIndex < SPECIALCMDNUM)
	{
		wifiCmdWaitRspBuf[tempIndex].cmdSn = cmdsta.cmdSn;
		wifiCmdWaitRspBuf[tempIndex].zbClusterId = cmdsta.zbClusterId;
		wifiCmdWaitRspBuf[tempIndex].zbCmd = cmdsta.zbCmd;
		wifiCmdWaitRspBuf[tempIndex].zbEp = cmdsta.zbEp;
		wifiCmdWaitRspBuf[tempIndex].rcvTimems = gwStateForRealTime.sysRealTimeFors;
		wifiCmdWaitRspBuf[tempIndex].zbShortAddr = cmdsta.zbShortAddr;
		wifiCmdWaitRspBuf[tempIndex].setData = cmdsta.setData;
		memset(wifiCmdWaitRspBuf[tempIndex].appUserName, '\0', 32);
		memcpy(wifiCmdWaitRspBuf[tempIndex].appUserName, cmdsta.appUserName, strlen(cmdsta.appUserName));

		memset(wifiCmdWaitRspBuf[tempIndex].appUserId, '\0', 12);
		memcpy(wifiCmdWaitRspBuf[tempIndex].appUserId, cmdsta.appUserId, strlen(cmdsta.appUserId));

		wifiCmdWaitRspBuf[tempIndex].valid = 1;
	}
	else
	{
		ret = -1;
	}
	return ret;
}

static uint8_t userFindFreeTxBufIndex(void)
{
	uint8_t i;
	for (i = 0; i < MAX_UART_TX_CMD_BUF_NUM; i++)
	{
		if (uartTXCmd[i].cmdValid == ZB_INVALID_DATA)
		{
			return i;
		}
	}
	return UART_FIND_BUF_INVALID_INDEX;
}

static void userClearValidUartTxBufIndex(uint8_t index)
{
	uartTXCmd[index].cmdValid = ZB_INVALID_DATA;
	uartTXCmd[index].dstAddr = 0;
	uartTXCmd[index].dstEp = 0;
	uartTXCmd[index].inCmd = 0;
}

void userFillUartCmdForHeart(void)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_SEND_ZC_HEART_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForReadDevInfo(uint16_t shortAddr)
{
}

void userSetUartHeartTime(void)
{
	time_t t = time(NULL);
	uint32_t temp_data, cal_data;
	temp_data = ((t & 0x000fffff) * 10);
	//printf("test 11111111111111111111111 the time temp_data:%d\r\n",temp_data);
	//cal_data = (uint32_t)((temp_data&0x000fffff)*10);
	//printf("test 11111111111111111111111 the time cal_data:%d\r\n",cal_data);
	uartHeartManage.lastCommuTime100ms = (uint32_t)(temp_data); // time_ms() / 100;//(uint32_t)(qcom_time_us() / 100000);
	uartHeartManage.uartHeartSendCount = 0;
	//printf("test 11111111111111111111111 the time:%ld\r\n",uartHeartManage.lastCommuTime100ms);
}

void userManageUartHeart(void)
{
	//uint32_t tempTimeMs = time_ms() / 100;//(uint32_t)(qcom_time_us() / 100000);
	uint32_t tempTimeMs;
	time_t t = time(NULL); //get current second
	tempTimeMs = (t & 0x000fffff) * 10;
	if (tempTimeMs > uartHeartManage.lastCommuTime100ms)
	{
		if (uartHeartManage.uartHeartSendCount == 0)
		{
			if ((tempTimeMs - uartHeartManage.lastCommuTime100ms) >= 600) // 1 min
			{
				userFillUartCmdForHeart();
				printf("fill send uart heart buf\r\n");
				uartHeartManage.lastCommuTime100ms = tempTimeMs;
				uartHeartManage.uartHeartSendCount++;
			}
		}
		else
		{
			if ((tempTimeMs - uartHeartManage.lastCommuTime100ms) >= 50) // 5 s
			{
				if (uartHeartManage.uartHeartSendCount >= 2)
				{
					uartHeartManage.lastCommuTime100ms = tempTimeMs;
					uartHeartManage.uartHeartSendCount = 0;
					uartHeartManage.uartWorkDataType = UART_WORK_COMMUNICATION;
					printf("the zc not respose the heart,reset it now\r\n");
					//zigbeeModeReset();
				}
				else
				{
					userFillUartCmdForHeart();
					printf("refill send uart heart buf\r\n");
					uartHeartManage.lastCommuTime100ms = tempTimeMs;
					uartHeartManage.uartHeartSendCount++;
				}
			}
		}
	}
	else
	{
		uartHeartManage.lastCommuTime100ms = tempTimeMs;
	}
}

static unsigned char calcFCS(unsigned char *pMsg, unsigned char len)
{
	unsigned char result = 0;
	while (len--)
	{
		result ^= *pMsg++;
	}
	return result;
}
int userUartSendWithDebugFunc(unsigned char *data, int len)
{
	int ret;

	char outTxHex[150];
	uint8_t i;

	ret = XmWriteBuf(data, len);
	return ret;
}

/*
void uartSendXmodemStartOTA(void)
{
	uint8_t data = 'u';
	//qca_uart_send(&data,1);
	XmWriteBuf(&data,1);
}*/

static uint8_t userFromInFineValidUartTxBufIndex(uint8_t in)
{
	uint8_t i;
	for (i = in; i < MAX_UART_TX_CMD_BUF_NUM; i++)
	{
		if (uartTXCmd[i].cmdValid == 1)
		{
			return i;
		}
	}
	return UART_FIND_BUF_INVALID_INDEX;
}

uint8_t checkIEEEAddrHasActive(EmberEUI64 checkEui64)
{
	const uint8_t unset1Eui64[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	const uint8_t unset2Eui64[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if ((0 == memcmp(checkEui64, unset1Eui64, 8)) ||
		(0 == memcmp(checkEui64, unset2Eui64, 8)))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

//HostEnterTestMode
void transportUartHexDataForReadEnterTestMode(void)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = 0xf1;
	transportBuf[3] = 0;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

void test_momde(void) //fa 02 15 ea ea f5
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = 0x15;
	transportBuf[3] = 0xea;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success
	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

void transportUartHexDataForReadIlluminance(void)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = 0xdf;
	transportBuf[3] = 0;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

void transportUartHexDataForReadIlluminanceValue(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = 0;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForReadNodeInfo(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(7); // (char *)malloc(NET_SEND_BUF_BASIC * NET_SEND_PK_NUM);//free(netSendInfo.netsendbuf);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 3;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = calcFCS(&transportBuf[3], 2); //payload ����
	transportBuf[6] = 0xf5;							//��β
	transportLen = 7;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForReadZCInfo(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = ~buff.inCmd;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen); //success

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForLeaveNode(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(16);
	uint8_t transportLen;
	int ret;
	HM_DEV_RAM_LIST *pRam = NULL;

	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}
	pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(buff.dstAddr);
	if (pRam == NULL)
	{
		printf("the leave dev short addr was not in dev list\r\n");
		return;
	}
	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 12;
	transportBuf[2] = buff.inCmd;
	memcpy(&transportBuf[3], pRam->mac, 8);
	transportBuf[11] = 0;
	transportBuf[12] = buff.dstAddr & 0xff;
	transportBuf[13] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[14] = calcFCS(&transportBuf[3], 11); //payload ����
	transportBuf[15] = 0xf5;						  //��β
	transportLen = 16;
	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}
	userDeleteOneDevFromRamAndFlash(pRam); //���ram ��flash
	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForUpdateZc(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.inPayload.inData[0];
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen);

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForWriteAttribute(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(25);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 4 + buff.inPayload.inDataLen;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	memcpy(&transportBuf[6], &buff.inPayload.inData[0], buff.inPayload.inDataLen);
	transportBuf[6 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], 3 + buff.inPayload.inDataLen); //payload ����
	transportBuf[7 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 8 + buff.inPayload.inDataLen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForReadAttribute(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(25);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 4 + buff.inPayload.inDataLen;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	memcpy(&transportBuf[6], &buff.inPayload.inData[0], buff.inPayload.inDataLen);
	transportBuf[6 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], 3 + buff.inPayload.inDataLen); //payload ����
	transportBuf[7 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 8 + buff.inPayload.inDataLen;
	userUartSendWithDebugFunc(transportBuf, transportLen);

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForSetZcToFactory(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	int ret;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = ~buff.inCmd;
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

// zigbee //
void userFillUartCmdForPermitJoin(uint8_t joinTime, uint8_t *userId, uint8_t *userName)
{
	uint8_t tpLen = 0;
	uint8_t tempTime = joinTime;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_PERMIT_JOIN_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = tempTime;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}

	if (tempTime > 0)
	{
		if (userId == NULL)
		{
			memset(permitJoinAction.userId, '\0', 32);
		}
		else
		{
			tpLen = strlen(userId);
			if (tpLen >= 31)
			{
				tpLen = 31;
			}
			memcpy(permitJoinAction.userId, userId, tpLen);
		}

		if (userName == NULL)
		{
			memset(permitJoinAction.userName, '\0', 32);
		}
		else
		{
			tpLen = strlen(userName);
			if (tpLen >= 31)
			{
				tpLen = 31;
			}
			memcpy(permitJoinAction.userName, userName, tpLen);
		}
	}
	else
	{
		memset(permitJoinAction.userName, '\0', 32);
		memset(permitJoinAction.userId, '\0', 32);
	}
}

void userFillUartCmdForReadZCnfo(void)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_READ_ZC_INFO_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForLeaveDev(uint16_t shortAddr)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = 0;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_LEAVE_DEV_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForZCOta(uint8_t softVersion)
{
	uint8_t tempVersion = softVersion;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_UPDATE_ZC_SOFT_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = tempVersion;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

//zigbee�ָ��������� //
void userFillUartCmdForSetZCToFactory(void)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_SET_ZC_TO_FACTORY_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForReadToIlluminance(void)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = 0;
		uartTXCmd[uartTxIndex].dstEp = 1;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_ILLUMINANCE_DATA_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForOnOff(uint16_t shortAddr, uint8_t ep, uint8_t state)
{
	uint8_t onOffState = state;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_ONOFF_CONTROL_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = onOffState;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}
int ailiyun_get_state(void)
{
	int ret = 0;
	if (hmIotsDevInfo.ServerConnected == 1)
	{
		ret = 1;
	}
	return ret;
}

void userFillUartCmdForDoorUnlock(uint16_t shortAddr, uint8_t ep, uint8_t *code, uint32_t cmdSn, uint8_t *userName, uint8_t *appUserId)
{
	uint8_t codeLen;
	uint8_t uartTxIndex;
	WIFICMDWATIRSP tempData;
	do
	{
		if (code == NULL)
		{
			printf("the code point was null\r\n");
			break;
			;
		}
		uartTxIndex = userFindFreeTxBufIndex();
		if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
		{
			codeLen = code[0];
			if (codeLen >= 20) //����ػ���
			{
				printf("the payload was too large\r\n");
				break;
			}
			uartTXCmd[uartTxIndex].dstAddr = shortAddr;
			uartTXCmd[uartTxIndex].dstEp = ep;
			uartTXCmd[uartTxIndex].inCmd = UART_CMD_DOOR_UNLOCK_CONTROL_REQ;
			uartTXCmd[uartTxIndex].cmdValid = 1;
			uartTXCmd[uartTxIndex].inPayload.inDataLen = codeLen + 1;
			memcpy(&uartTXCmd[uartTxIndex].inPayload.inData[0], &code[0], codeLen + 1);

			tempData.cmdSn = cmdSn;
			tempData.setData = 0;
			tempData.zbClusterId = ZCL_DOOR_LOCK_CLUSTER_ID;
			tempData.zbCmd = UART_CMD_DOOR_UNLOCK_CONTROL_REQ;
			tempData.zbEp = ep;
			tempData.zbShortAddr = shortAddr;

			memset(tempData.appUserName, '\0', 32);
			if (userName == NULL)
			{
				memcpy(tempData.appUserName, "", sizeof(""));
			}
			else
			{
				memcpy(tempData.appUserName, userName, strlen(userName));
			}

			memset(tempData.appUserId, '\0', 12);
			if (appUserId == NULL)
			{
				memcpy(tempData.appUserId, "", sizeof(""));
			}
			else
			{
				memcpy(tempData.appUserId, appUserId, strlen(appUserId));
			}
			userFillSpecialWatiRspCmdBuf(tempData);
		}
		else
		{
			printf("not find free tx buf\r\n");
			break;
		}
	} while (0);
}

int userSetAirQualityTime(uint16_t shortAddr, uint8_t ep, uint32_t uctime, int32_t timezone)
{
	int ret = 0;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (ailiyun_get_state() != 1)
	{
		ret = -1;
		return ret;
	}
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 17;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_TIME_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_TIME_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x02; //zcl com write attr
		uartTXCmd[uartTxIndex].inPayload.inData[3] = ZCL_CURRENT_TIME_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[4] = (ZCL_CURRENT_TIME_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[5] = 0xe2; //data type :map8 utc
		uartTXCmd[uartTxIndex].inPayload.inData[6] = uctime & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[7] = ((uint32_t)(uctime >> 8)) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[8] = ((uint32_t)(uctime >> 16)) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[9] = ((uint32_t)(uctime >> 24)) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[10] = ZCL_CURRENT_TIMEZONE_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[11] = (ZCL_CURRENT_TIMEZONE_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[12] = 0x2b; //data type :map8 int32
		uartTXCmd[uartTxIndex].inPayload.inData[13] = timezone & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[14] = ((int32_t)(timezone >> 8)) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[15] = ((int32_t)(timezone >> 16)) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[16] = ((int32_t)(timezone >> 24)) & 0xff;
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}
int userSetAirQualityLG(uint16_t shortAddr, uint8_t ep, uint8_t wrdata)
{
	int ret = 0;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_AIR_CONTROL_REQ; //UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 4;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x00; //zcl LG

		uartTXCmd[uartTxIndex].inPayload.inData[3] = wrdata;
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

int userSetAirQualitySetNotDisturb(uint16_t shortAddr, uint8_t ep, uint16_t wrdata)
{
	int ret = 0;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 8;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x02; //zcl com write attr
		uartTXCmd[uartTxIndex].inPayload.inData[3] = ZCL_AIR_ALARM_ENABLE_VALUE_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[4] = (ZCL_AIR_ALARM_ENABLE_VALUE_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[5] = 0x21; //data type :map8 utc
		uartTXCmd[uartTxIndex].inPayload.inData[6] = wrdata & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[7] = (wrdata >> 8) & 0xff;
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

int userSetAirQualityTUI(uint16_t shortAddr, uint8_t ep, uint32_t wrdata)
{
	int ret = 0;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_AIR_CONTROL_REQ; //UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 4;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x01; //zcl tui
		uartTXCmd[uartTxIndex].inPayload.inData[3] = wrdata;
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

void userFillUartCmdForCurtainMotorClearPhyLimit(uint16_t shortAddr, uint8_t ep)
{
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_CURTAIN_MOTOR_CLEAR_PHY_LIMIT_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = 0;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

int userSetNodeSensitivityLevel(uint16_t shortAddr, uint8_t ep, uint8_t state, uint32_t cmdSn, uint8_t *userName, uint8_t *appUserId)
{
	int ret = 0;
	uint8_t level = state - 1;
	WIFICMDWATIRSP tempData;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 7;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_IAS_ZONE_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_IAS_ZONE_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x02; //zcl com write attr
		uartTXCmd[uartTxIndex].inPayload.inData[3] = ZCL_CURRENT_ZONE_SENSITIVITY_LEVEL_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[4] = (ZCL_CURRENT_ZONE_SENSITIVITY_LEVEL_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[5] = 0x20; //data type :uint8_t
		uartTXCmd[uartTxIndex].inPayload.inData[6] = level;

		tempData.cmdSn = cmdSn;
		tempData.setData = level;
		tempData.zbClusterId = ZCL_IAS_ZONE_CLUSTER_ID;
		tempData.zbCmd = UART_CMD_WRITE_ATTR_REQ;
		tempData.zbEp = ep;
		tempData.zbShortAddr = shortAddr;
		memset(tempData.appUserName, '\0', 32);
		if (userName == NULL)
		{
			memcpy(tempData.appUserName, "", sizeof(""));
		}
		else
		{
			memcpy(tempData.appUserName, userName, strlen(userName));
		}

		memset(tempData.appUserId, '\0', 12);
		if (appUserId == NULL)
		{
			memcpy(tempData.appUserId, "", sizeof(""));
		}
		else
		{
			memcpy(tempData.appUserId, appUserId, strlen(appUserId));
		}

		userFillSpecialWatiRspCmdBuf(tempData);
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

/*****************************************************************
���ô��������ת������
shortAddr �����õ��豸�Ķ̵�ַ
ep �����õ��豸��endpoint
state ������ֵ��Э����ȡֵ0��1
*****************************************************************/
int userSetCurtainMotorDirect(uint16_t shortAddr, uint8_t ep, uint8_t state)
{
	int ret = 0;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 7;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_SHADE_CONFIG_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_SHADE_CONFIG_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x02; //zcl com write attr
		uartTXCmd[uartTxIndex].inPayload.inData[3] = ZCL_SHADE_CONFIG_STATUS_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[4] = (ZCL_SHADE_CONFIG_STATUS_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[5] = 0x18; //data type :map8
		uartTXCmd[uartTxIndex].inPayload.inData[6] = state;
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

int userSetDoorLockRemoteOpenEnable(uint16_t shortAddr, uint8_t ep, uint8_t state, uint32_t cmdSn, uint8_t *userName, uint8_t *appUserId)
{
	int ret = 0;
	WIFICMDWATIRSP tempData;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 7;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = ZCL_BASIC_CLUSTER_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[1] = (ZCL_BASIC_CLUSTER_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[2] = 0x02; //zcl com write attr
		uartTXCmd[uartTxIndex].inPayload.inData[3] = ZCL_DEVICE_ENABLED_ATTRIBUTE_ID & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[4] = (ZCL_DEVICE_ENABLED_ATTRIBUTE_ID >> 8) & 0xff;
		uartTXCmd[uartTxIndex].inPayload.inData[5] = 0x10; //data type :bool
		uartTXCmd[uartTxIndex].inPayload.inData[6] = state;

		tempData.cmdSn = cmdSn;
		tempData.setData = state;
		tempData.zbClusterId = ZCL_BASIC_CLUSTER_ID;
		tempData.zbCmd = UART_CMD_WRITE_ATTR_REQ;
		tempData.zbEp = ep;
		tempData.zbShortAddr = shortAddr;

		memset(tempData.appUserName, '\0', 32);
		if (userName == NULL)
		{
			memcpy(tempData.appUserName, "", sizeof(""));
		}
		else
		{
			memcpy(tempData.appUserName, userName, strlen(userName));
		}

		memset(tempData.appUserId, '\0', 12);
		if (appUserId == NULL)
		{
			memcpy(tempData.appUserId, "", sizeof(""));
		}
		else
		{
			memcpy(tempData.appUserId, appUserId, strlen(appUserId));
		}
		userFillSpecialWatiRspCmdBuf(tempData);
	}
	else
	{
		printf("not find free tx buf\r\n");
		ret = -1;
	}
	return ret;
}

void userFillUartCmdForOnOffByGroupsOrBroadcast(uint16_t addr, uint8_t addrType, uint8_t state)
{
	uint8_t onOffState = state;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = addr;
		uartTXCmd[uartTxIndex].dstEp = addrType;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_ONOFF_CONTROL_BY_GROUP_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = onOffState;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForLevel(uint16_t shortAddr, uint8_t ep, uint8_t subcmd, uint8_t level, uint8_t upOrDown)
{
	uint16_t transitionTime = 1; // 0.2s
	uint8_t inPayload[5];
	uint8_t payloadLen;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	switch (subcmd)
	{
	case MOVE_TO_LEVEL_WTONOFF:
		inPayload[1] = level;
		inPayload[2] = transitionTime & 0xff;
		inPayload[3] = (transitionTime >> 8) & 0xff;
		payloadLen = 4;
		break;
	case STEP_WTONOFF:
		inPayload[1] = upOrDown;
		inPayload[2] = level;
		inPayload[3] = transitionTime & 0xff;
		inPayload[4] = (transitionTime >> 8) & 0xff;
		payloadLen = 5;
		break;
	case STOP_WTONOFF:
		payloadLen = 1;
		break;
		//��ʱ��֧������ָ��
	case MOVE_TO_LEVEL:
		inPayload[1] = level;
		inPayload[2] = transitionTime & 0xff;
		inPayload[3] = (transitionTime >> 8) & 0xff;
		payloadLen = 4;
		break;
	case MOVE:
	case STEP:
	case MOVE_WTONOFF:
	case STOP:
	default:
		return;
	}
	inPayload[0] = subcmd;
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_LEVEL_CONTROL_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForColorControl(uint16_t shortAddr, uint8_t ep, uint16_t r, uint16_t g, uint16_t b)
{
	uint16_t transitionTime = 2; // 0.2s
	uint8_t inPayload[7];
	uint8_t payloadLen;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	uint16_t color_x;
	uint16_t color_y;
	uint8_t tempLevel;
	hmzColorRgbToXY(r, g, b, &color_x, &color_y, &tempLevel);
	printf("CmdForColorControl color_x:%d,color_y:%d,tempLevel:%d\r\n", color_x, color_y, tempLevel);
	inPayload[0] = MOVE_TO_COLOR;
	inPayload[1] = color_x & 0xff;
	inPayload[2] = (color_x >> 8) & 0xff;
	inPayload[3] = color_y & 0xff;
	inPayload[4] = (color_y >> 8) & 0xff;
	inPayload[5] = transitionTime & 0xff;
	inPayload[6] = (transitionTime >> 8) & 0xff;
	payloadLen = 7;
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_COLOR_CONTROL_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForColorTemperatureWY(uint16_t shortAddr, uint8_t ep, uint16_t w, uint16_t y)
{
	uint16_t transitionTime = 2; // 0.2s
	uint8_t inPayload[7];
	uint8_t payloadLen;
	uint16_t ColorTemperature_wy;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	uint16_t color_x;
	uint16_t color_y;
	uint8_t tempLevel;
	ColorTemperature_wy = 1000000 / w;
	inPayload[0] = MOVE_COLOR_TEMPERATURE;
	inPayload[1] = ColorTemperature_wy & 0xff;
	inPayload[2] = (ColorTemperature_wy >> 8) & 0xff;
	inPayload[3] = transitionTime & 0xff;
	inPayload[4] = (transitionTime >> 8) & 0xff;
	payloadLen = 5;
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_COLOR_CONTROL_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForWd(uint16_t shortAddr, uint8_t ep, uint8_t subcmd, uint8_t warmMode, uint16_t duration, uint8_t strobeCycle, uint8_t strobeLevel)
{
	uint8_t inPayload[6];
	uint8_t payloadLen;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (subcmd == START_WARNING)
	{
		inPayload[0] = START_WARNING;
		inPayload[1] = warmMode;
		inPayload[2] = duration & 0xff;
		inPayload[3] = (duration >> 8) & 0xff;
		inPayload[4] = strobeCycle;
		inPayload[5] = strobeLevel;
		payloadLen = 6;
	}
	else
	{
		return;
	}
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WD_CONTROL_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void AlarmWDStateCbk(uint32_t num)
{
	HM_DEV_RAM_LIST *pni_tmp;
	setWDTimes = 0;
	gwStateForRealTime.gwIsAlarning = 0;
	pni_tmp = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(WDShortAddr);
	writeNodeStateAlarnToCloud(pni_tmp, 0, 0xff);
}

void userAppControlWDFunc(uint16_t shortAddr, uint8_t ep, uint8_t action, uint16_t duration)
{
	if (action)
	{
		userFillUartCmdForWd(shortAddr, ep, START_WARNING, 0x14, duration, 40, 0);
		if (g_hTimerWDDelay > 0)
		{
			StopTimer(g_hTimerWDDelay);
		}
		setWDTimes = duration;
		WDShortAddr = shortAddr;
		g_hTimerWDDelay = StartTimer(setWDTimes, 490, AlarmWDStateCbk);
	}
	else
	{
		userFillUartCmdForWd(shortAddr, ep, START_WARNING, 0, 0, 0, 0);
		if (g_hTimerWDDelay > 0)
		{
			StopTimer(g_hTimerWDDelay);
		}
	}
}

void userFillUartCmdForWriteAttr(uint16_t shortAddr, uint8_t ep, uint16_t clusterId, uint16_t attrId, uint8_t dataType, uint8_t dataLen, uint8_t *data)
{
	uint8_t inPayload[20];
	uint8_t payloadLen;
	uint8_t i;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if ((data == NULL) || (dataLen > 8))
	{
		printf("fill write attr func:in put data point was invalid\r\n");
		return;
	}
	inPayload[0] = clusterId & 0xff;
	inPayload[1] = (clusterId >> 8) & 0xff;
	inPayload[2] = 2; //�̶�ֵ( ����)
	inPayload[3] = attrId & 0xff;
	inPayload[4] = (attrId >> 8) & 0xff;
	inPayload[5] = dataType;
	for (i = 0; i < dataLen; i++) //�ֽ�������
	{
		inPayload[6 + i] = data[dataLen - 1 - i];
	}
	payloadLen = 6 + dataLen;
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_WRITE_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;

		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void userFillUartCmdForReadAttr(uint16_t shortAddr, uint8_t ep, uint16_t clusterId, uint16_t attrId)
{
	uint8_t inPayload[5];
	uint8_t payloadLen;
	uint8_t i;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	inPayload[0] = clusterId & 0xff;
	inPayload[1] = (clusterId >> 8) & 0xff;
	inPayload[2] = 0; //�̶�ֵ( ����)
	inPayload[3] = attrId & 0xff;
	inPayload[4] = (attrId >> 8) & 0xff;

	payloadLen = 5;
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_READ_ATTR_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		memcpy(uartTXCmd[uartTxIndex].inPayload.inData, inPayload, payloadLen);
		uartTXCmd[uartTxIndex].inPayload.inDataLen = payloadLen;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

// set bind buff //
void userFillUartCmdForBind(uint16_t srcShortAddr, uint8_t ep, uint8_t *srcMac, uint8_t dstEp, uint8_t *dstMac)
{
	//uint8_t onOffState = state;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = srcShortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_BIND_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 17;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = srcMac[0];
		uartTXCmd[uartTxIndex].inPayload.inData[1] = srcMac[1];
		uartTXCmd[uartTxIndex].inPayload.inData[2] = srcMac[2];
		uartTXCmd[uartTxIndex].inPayload.inData[3] = srcMac[3];
		uartTXCmd[uartTxIndex].inPayload.inData[4] = srcMac[4];
		uartTXCmd[uartTxIndex].inPayload.inData[5] = srcMac[5];
		uartTXCmd[uartTxIndex].inPayload.inData[6] = srcMac[6];
		uartTXCmd[uartTxIndex].inPayload.inData[7] = srcMac[7];

		uartTXCmd[uartTxIndex].inPayload.inData[8] = dstMac[0];
		uartTXCmd[uartTxIndex].inPayload.inData[9] = dstMac[1];
		uartTXCmd[uartTxIndex].inPayload.inData[10] = dstMac[2];
		uartTXCmd[uartTxIndex].inPayload.inData[11] = dstMac[3];
		uartTXCmd[uartTxIndex].inPayload.inData[12] = dstMac[4];
		uartTXCmd[uartTxIndex].inPayload.inData[13] = dstMac[5];
		uartTXCmd[uartTxIndex].inPayload.inData[14] = dstMac[6];
		uartTXCmd[uartTxIndex].inPayload.inData[15] = dstMac[7];
		uartTXCmd[uartTxIndex].inPayload.inData[16] = dstEp;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

// set unbind buff //
void userFillUartCmdForUNBind(uint16_t srcShortAddr, uint8_t ep, uint8_t *srcMac, uint8_t dstEp, uint8_t *dstMac)
{
	//uint8_t onOffState = state;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = srcShortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_UNBIND_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 17;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = srcMac[0];
		uartTXCmd[uartTxIndex].inPayload.inData[1] = srcMac[1];
		uartTXCmd[uartTxIndex].inPayload.inData[2] = srcMac[2];
		uartTXCmd[uartTxIndex].inPayload.inData[3] = srcMac[3];
		uartTXCmd[uartTxIndex].inPayload.inData[4] = srcMac[4];
		uartTXCmd[uartTxIndex].inPayload.inData[5] = srcMac[5];
		uartTXCmd[uartTxIndex].inPayload.inData[6] = srcMac[6];
		uartTXCmd[uartTxIndex].inPayload.inData[7] = srcMac[7];

		uartTXCmd[uartTxIndex].inPayload.inData[8] = dstMac[0];
		uartTXCmd[uartTxIndex].inPayload.inData[9] = dstMac[1];
		uartTXCmd[uartTxIndex].inPayload.inData[10] = dstMac[2];
		uartTXCmd[uartTxIndex].inPayload.inData[11] = dstMac[3];
		uartTXCmd[uartTxIndex].inPayload.inData[12] = dstMac[4];
		uartTXCmd[uartTxIndex].inPayload.inData[13] = dstMac[5];
		uartTXCmd[uartTxIndex].inPayload.inData[14] = dstMac[6];
		uartTXCmd[uartTxIndex].inPayload.inData[15] = dstMac[7];
		uartTXCmd[uartTxIndex].inPayload.inData[16] = dstEp;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

// set privatebind buff //
void userFillUartCmdForPrivateBind(uint16_t shortAddr, uint8_t ep, uint8_t state)
{
	uint8_t onOffState = state;
	uint8_t uartTxIndex = userFindFreeTxBufIndex();
	if (uartTxIndex != UART_FIND_BUF_INVALID_INDEX)
	{
		uartTXCmd[uartTxIndex].dstAddr = shortAddr;
		uartTXCmd[uartTxIndex].dstEp = ep;
		uartTXCmd[uartTxIndex].inCmd = UART_CMD_UNBIND_REQ;
		uartTXCmd[uartTxIndex].cmdValid = 1;
		uartTXCmd[uartTxIndex].inPayload.inDataLen = 1;
		uartTXCmd[uartTxIndex].inPayload.inData[0] = onOffState;
	}
	else
	{
		printf("not find free tx buf\r\n");
	}
}

void fillUartCmdMonitor(uint8_t cmd, uint16_t shortAddr, uint8_t ep, uint8_t *cpcmdData, uint8_t cpcmdLen)
{
	uint8_t i;
	time_t t;
	uint8_t tempIndex = 0xff;
	if (cpcmdLen > MAX_BACKUP_DATA_LEN)
	{
		printf("input the back up func data was too long\r\n");
		return;
	}
	for (i = 0; i < MAX_UART_CMD_MONITOR_NUM; i++)
	{
		if (tempIndex > MAX_UART_CMD_MONITOR_NUM)
		{
			if (uartCmdMonito[i].validFlag == 0)
			{
				tempIndex = i;
			}
		}
		if ((uartCmdMonito[i].cmd == cmd) && (uartCmdMonito[i].ep == ep) && (uartCmdMonito[i].shortAddr == shortAddr))
		{
			tempIndex = i;
			break;
		}
	}
	if (tempIndex < MAX_UART_CMD_MONITOR_NUM)
	{
		t = time(NULL);
		uartCmdMonito[tempIndex].cmd = cmd;
		uartCmdMonito[tempIndex].ep = ep;
		uartCmdMonito[tempIndex].shortAddr = shortAddr;
		uartCmdMonito[tempIndex].sendTime100Ms = t * 10; //10t��100ms //t >> 8;//(qcom_time_us()) >> 8;//����1024��Լ����100���ڴ�ʱ�侫ȷ��Ҫ�󲻸ߣ�Ϊ�˽�ʡCPU����
		uartCmdMonito[tempIndex].validFlag = 1;
		uartCmdMonito[tempIndex].cmdbk.bkLen = cpcmdLen;
		memcpy(uartCmdMonito[tempIndex].cmdbk.bkBuff, cpcmdData, cpcmdLen);
	}
}

static void transportUartHexDataForPermitJoin(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6); //(uint8_t *)mem_alloc(6);
	uint8_t transportLen;
	int ret;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.inPayload.inData[0];
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForHeart(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(5);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = 5;							//����
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	userUartSendWithDebugFunc(transportBuf, transportLen);

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForOnOff(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(12);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 7;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_ON_OFF_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_ON_OFF_CLUSTER_ID >> 8) & 0xff;
	transportBuf[8] = buff.inPayload.inData[0];
	transportBuf[9] = calcFCS(&transportBuf[3], 6); //payload ����
	transportBuf[10] = 0xf5;						//��β
	transportLen = 11;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
		//hardy add
		userFillUartCmdForControlState(buff.inCmd, buff.dstAddr, buff.inPayload.inData[0]);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForLevel(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(10 + buff.inPayload.inDataLen);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = buff.inPayload.inDataLen + 6;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_LEVEL_CONTROL_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_LEVEL_CONTROL_CLUSTER_ID >> 8) & 0xff;
	memcpy(&transportBuf[8], buff.inPayload.inData, buff.inPayload.inDataLen);
	transportBuf[8 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], buff.inPayload.inDataLen + 5); //payload ����
	transportBuf[9 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 10 + buff.inPayload.inDataLen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForWDControl(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(10 + buff.inPayload.inDataLen);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = buff.inPayload.inDataLen + 6;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_IAS_WD_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_IAS_WD_CLUSTER_ID >> 8) & 0xff;
	memcpy(&transportBuf[8], buff.inPayload.inData, buff.inPayload.inDataLen);
	transportBuf[8 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], buff.inPayload.inDataLen + 5); //payload ����
	transportBuf[9 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 10 + buff.inPayload.inDataLen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForColorControl(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(10 + buff.inPayload.inDataLen);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = buff.inPayload.inDataLen + 6;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_COLOR_CONTROL_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_COLOR_CONTROL_CLUSTER_ID >> 8) & 0xff;
	memcpy(&transportBuf[8], buff.inPayload.inData, buff.inPayload.inDataLen);
	transportBuf[8 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], buff.inPayload.inDataLen + 5); //payload ����
	transportBuf[9 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 10 + buff.inPayload.inDataLen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForOnOffControlByGroupsOrBroadcast(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(12);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 7;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstEp;
	transportBuf[4] = buff.dstAddr & 0xff;
	transportBuf[5] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[6] = ZCL_ON_OFF_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_ON_OFF_CLUSTER_ID >> 8) & 0xff;
	transportBuf[8] = buff.inPayload.inData[0];
	transportBuf[9] = calcFCS(&transportBuf[3], 6); //payload ����
	transportBuf[10] = 0xf5;						//��β
	transportLen = 11;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForCutRgb(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(12);
	uint8_t transportLen;
	uint8_t cplen = 0;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_RGBLIGHT_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_RGBLIGHT_CLUSTER_ID >> 8) & 0xff;
	transportBuf[8] = buff.inPayload.inData[0];
	if (transportBuf[8] == 0) //brightness control
	{
		transportBuf[9] = buff.inPayload.inData[1];
		cplen = 1;
		transportBuf[1] = 8; //lenth
	}
	else
	{
		transportBuf[9] = buff.inPayload.inData[1];
		transportBuf[10] = buff.inPayload.inData[2];
		cplen = 2;
		transportBuf[1] = 9; //lenth
	}
	transportBuf[9 + cplen] = calcFCS(&transportBuf[3], 6 + cplen); //payload ����
	transportBuf[10 + cplen] = 0xf5;								//��β
	transportLen = 11 + cplen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForDoorUnlock(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(32);
	uint8_t codeLen;
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ

	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_DOOR_LOCK_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_DOOR_LOCK_CLUSTER_ID >> 8) & 0xff;
	transportBuf[8] = 0x01; //command id ,unlock door
	transportBuf[9] = buff.inPayload.inData[0];
	codeLen = transportBuf[9];
	memcpy(&transportBuf[10], &buff.inPayload.inData[1], codeLen);
	transportBuf[10 + codeLen] = calcFCS(&transportBuf[3], 7 + codeLen); //payload ����
	transportBuf[11 + codeLen] = 0xf5;									 //��β
	transportBuf[1] = 8 + codeLen;
	transportLen = 12 + codeLen;

	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForCurtainMotorClearPhyLimit(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(12);
	uint8_t codeLen;
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 7;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	transportBuf[6] = ZCL_SHADE_CONFIG_CLUSTER_ID & 0xff;
	transportBuf[7] = (ZCL_SHADE_CONFIG_CLUSTER_ID >> 8) & 0xff;
	transportBuf[8] = 0x00;							//command id
	transportBuf[9] = calcFCS(&transportBuf[3], 6); //payload ����
	transportBuf[10] = 0xf5;						//��β

	transportLen = 11;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForWriteAQValue(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(25);
	uint8_t transportLen;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			mem_free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 4 + buff.inPayload.inDataLen;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp;
	memcpy(&transportBuf[6], &buff.inPayload.inData[0], buff.inPayload.inDataLen);
	transportBuf[6 + buff.inPayload.inDataLen] = calcFCS(&transportBuf[3], 3 + buff.inPayload.inDataLen); //payload ����
	transportBuf[7 + buff.inPayload.inDataLen] = 0xf5;													  //��β
	transportLen = 8 + buff.inPayload.inDataLen;
	if (userUartSendWithDebugFunc(transportBuf, transportLen) == 0) //success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

static void transportUartHexDataForSetZcRSSIReport(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(6);
	uint8_t transportLen;
	int ret;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;
	transportBuf[3] = buff.dstEp;					//buff.cmdValid;//.cmdValid
	transportBuf[4] = calcFCS(&transportBuf[3], 1); //payload ����
	transportBuf[5] = 0xf5;							//��β
	transportLen = 6;
	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

//�󶨵�ɫ���� //
static void transportUartHexDataForBindSwitch(UART_TX_CMD_BUF buff) //option ,uint8_t option,uint8_t deviceidType
{
	uint8_t *transportBuf = (uint8_t *)malloc(48);
	uint8_t transportLen;
	int ret;
	uint8_t option, deviceidType;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}
	option = buff.inPayload.inData[11];
	deviceidType = buff.inPayload.inData[12];

	transportBuf[0] = 0xfa; //��ͷ
							//  transportBuf[1] = 2;
	transportBuf[2] = buff.inCmd;

	transportBuf[3] = buff.dstAddr & 0xff;
	transportBuf[4] = (buff.dstAddr >> 8) & 0xff;
	transportBuf[5] = buff.dstEp; //buff.dstAddr
	transportBuf[6] = option;	  //option 0 ��  1���

	transportBuf[7] = buff.inPayload.inData[0];	 //dstep Ŀ��
	transportBuf[8] = buff.inPayload.inData[1];	 //dstAddr
	transportBuf[9] = buff.inPayload.inData[2];	 //dstAddr 00 0D 6F 00 14 F6 EF CA
	transportBuf[10] = buff.inPayload.inData[3]; //mac
	transportBuf[11] = buff.inPayload.inData[4];
	transportBuf[12] = buff.inPayload.inData[5];
	transportBuf[13] = buff.inPayload.inData[6];
	transportBuf[14] = buff.inPayload.inData[7];
	transportBuf[15] = buff.inPayload.inData[8];
	transportBuf[16] = buff.inPayload.inData[9];
	transportBuf[17] = buff.inPayload.inData[10]; //mac
	if (deviceidType == NODE_INFO_TYPE_CURTAINSWITCH_SWITCH)
	{
		transportBuf[1] = 21;

		transportBuf[18] = 0x02; //Cluster ����
		transportBuf[19] = 0x0006 & 0xff;
		transportBuf[20] = (0x0006 >> 8) & 0xff;
		transportBuf[21] = 0x0008 & 0xff;
		transportBuf[22] = (0x0008 >> 8) & 0xff;
		transportBuf[23] = calcFCS(&transportBuf[3], 20); //payload ����
		transportBuf[24] = 0xf5;						  //��β
		transportLen = 25;
	}
	else if (deviceidType == NODE_INFO_TYPE_TEMPDIMMING_SWITCH || deviceidType == NODE_INFO_TYPE_DIMMING_SWITCH)
	{
		transportBuf[1] = 23;

		transportBuf[18] = 0x03;
		transportBuf[19] = 0x0006 & 0xff;
		transportBuf[20] = (0x0006 >> 8) & 0xff;
		transportBuf[21] = 0x0008 & 0xff;
		transportBuf[22] = (0x0008 >> 8) & 0xff;
		transportBuf[23] = 0x0300 & 0xff;
		transportBuf[24] = (0x0300 >> 8) & 0xff;
		transportBuf[25] = calcFCS(&transportBuf[3], 22); //payload ����
		transportBuf[26] = 0xf5;						  //��β
		transportLen = 27;
	}

	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

//�����߿��� //
static void transportUartHexDataForBindFunc(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(48);
	uint8_t transportLen;
	int ret;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 22;
	transportBuf[2] = buff.inCmd;

	transportBuf[3] = buff.inPayload.inData[0]; //source mac
	transportBuf[4] = buff.inPayload.inData[1];
	transportBuf[5] = buff.inPayload.inData[2];
	transportBuf[6] = buff.inPayload.inData[3];
	transportBuf[7] = buff.inPayload.inData[4];
	transportBuf[8] = buff.inPayload.inData[5];
	transportBuf[9] = buff.inPayload.inData[6];
	transportBuf[10] = buff.inPayload.inData[7]; //mac

	transportBuf[11] = buff.dstEp;			 //buff.dstAddr
	transportBuf[12] = 0Xffea & 0xff;		 //cluster 0XFFEA
	transportBuf[13] = (0Xffea >> 8) & 0xff; //cluster 0XFFEA

	transportBuf[14] = 0x03;

	transportBuf[15] = buff.inPayload.inData[8]; //dst mac
	transportBuf[16] = buff.inPayload.inData[9];
	transportBuf[17] = buff.inPayload.inData[10];
	transportBuf[18] = buff.inPayload.inData[11];
	transportBuf[19] = buff.inPayload.inData[12];
	transportBuf[20] = buff.inPayload.inData[13];
	transportBuf[21] = buff.inPayload.inData[14];
	transportBuf[22] = buff.inPayload.inData[15]; //mac
	transportBuf[23] = buff.inPayload.inData[16]; //ep

	transportBuf[24] = calcFCS(&transportBuf[3], 21); //payload ����
	transportBuf[25] = 0xf5;						  //��β
	transportLen = 26;

	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

//������߿��� //
static void transportUartHexDataForUNBindFunc(UART_TX_CMD_BUF buff)
{
	uint8_t *transportBuf = (uint8_t *)malloc(48);
	uint8_t transportLen;
	int ret;
	if (transportBuf == NULL)
	{
		printf("sys malloc mem was faild\r\n");
		return;
	}
	if (buff.inPayload.inData == NULL)
	{
		printf("send uart in data was null\r\n");
		if (transportBuf != NULL)
		{
			free(transportBuf);
			transportBuf = NULL;
		}
		return;
	}

	transportBuf[0] = 0xfa; //��ͷ
	transportBuf[1] = 22;
	transportBuf[2] = buff.inCmd;

	transportBuf[3] = buff.inPayload.inData[0]; //source mac
	transportBuf[4] = buff.inPayload.inData[1];
	transportBuf[5] = buff.inPayload.inData[2];
	transportBuf[6] = buff.inPayload.inData[3];
	transportBuf[7] = buff.inPayload.inData[4];
	transportBuf[8] = buff.inPayload.inData[5];
	transportBuf[9] = buff.inPayload.inData[6];
	transportBuf[10] = buff.inPayload.inData[7]; //mac

	transportBuf[11] = buff.dstEp;			 //buff.dstAddr
	transportBuf[12] = 0Xffea & 0xff;		 //cluster 0XFFEA
	transportBuf[13] = (0Xffea >> 8) & 0xff; //cluster 0XFFEA

	transportBuf[14] = 0x03;

	transportBuf[15] = buff.inPayload.inData[8]; //dst mac
	transportBuf[16] = buff.inPayload.inData[9];
	transportBuf[17] = buff.inPayload.inData[10];
	transportBuf[18] = buff.inPayload.inData[11];
	transportBuf[19] = buff.inPayload.inData[12];
	transportBuf[20] = buff.inPayload.inData[13];
	transportBuf[21] = buff.inPayload.inData[14];
	transportBuf[22] = buff.inPayload.inData[15]; //mac
	transportBuf[23] = buff.inPayload.inData[16]; //ep

	transportBuf[24] = calcFCS(&transportBuf[3], 21); //payload ����
	transportBuf[25] = 0xf5;						  //��β
	transportLen = 26;

	ret = userUartSendWithDebugFunc(transportBuf, transportLen);
	printf("ret:%d\r\n", ret);
	if (ret == 0) //(qca_uart_send(transportBuf,transportLen) == 0)//success
	{
		fillUartCmdMonitor(buff.inCmd, buff.dstAddr, buff.dstEp, transportBuf, transportLen);
	}

	if (transportBuf != NULL)
	{
		free(transportBuf);
		transportBuf = NULL;
	}
}

uint8_t findUartCmdMonitor(uint8_t cmd, uint16_t shortAddr, uint8_t ep)
{
	uint8_t i;
	for (i = 0; i < MAX_UART_CMD_MONITOR_NUM; i++)
	{
		if (uartCmdMonito[i].validFlag == 1)
		{
			if (uartCmdMonito[i].cmd == cmd)
			{
				if (shortAddr == 0xffff) //����Ҫƥ��shortaddr ��ep
				{
					break;
				}
				else if ((uartCmdMonito[i].ep == ep) && (uartCmdMonito[i].shortAddr == shortAddr))
				{
					break;
				}
			}
		}
	}
	if (i < MAX_UART_CMD_MONITOR_NUM)
	{
		return i;
	}
	else
	{
		return 0xff;
	}
}

void deleteUartCmdMonitor(uint8_t index)
{
	if (index < MAX_UART_CMD_MONITOR_NUM)
	{
		memset((uint8_t *)&uartCmdMonito[index], 0, sizeof(UARTCMDMONITOR));
	}
}

void clearUartCmdMonitor(void)
{
	uint8_t i;
	for (i = 0; i < MAX_UART_CMD_MONITOR_NUM; i++)
	{
		deleteUartCmdMonitor(i);
	}
}

void userManageUartCmdMonitor(void)
{
	uint32_t tempTime100ms; // = time_ms() / 100;//qcom_time_us() / 100000 ;
	time_t t = time(NULL);
	uint8_t i;
	tempTime100ms = t * 10; //1s ->> 10 100ms
	for (i = 0; i < MAX_UART_CMD_MONITOR_NUM; i++)
	{
		if (uartCmdMonito[i].validFlag == 1)
		{
			//printf("test compare---------------------tempTime100ms:%d,sendTime100Ms:%d\n",tempTime100ms,uartCmdMonito[i].sendTime100Ms);
			if (tempTime100ms > uartCmdMonito[i].sendTime100Ms)
			{
				if (tempTime100ms - uartCmdMonito[i].sendTime100Ms > 25) // 2.5s
				{
					if (uartCmdMonito[i].sendCount < 2)
					{
						printf("monitor resend the cmd\r\n");
						XmWriteBuf(uartCmdMonito[i].cmdbk.bkBuff, uartCmdMonito[i].cmdbk.bkLen);
						uartCmdMonito[i].sendCount++;
						uartCmdMonito[i].sendTime100Ms = tempTime100ms;
					}
					else
					{
						printf("monitor delete the cmd\r\n");
						deleteUartCmdMonitor(i);
					}
				}
			}
			else
			{
				uartCmdMonito[i].sendTime100Ms = tempTime100ms;
			}
		}
	}
}

/***************************************************************
�������� userFillUartCmdForControlState
����˵���� ������zigbee�豸�����÷���ָ��ĵ�ǰʱ��
***************************************************************/
void userFillUartCmdForControlState(uint8_t cmd, uint16_t shortAddr, uint8_t state)
{
	uint8_t i = 0;
	uint8_t tempIndex = 0xff;
	time_t t;
	for (i = 0; i < 10; i++) //uartCTRLCmd
	{
		//for(i = 0; i < MAX_UART_CMD_MONITOR_NUM; i ++)
		{
			if (tempIndex > 10)
			{
				if (uartCTRLCmd[i].validFlag == 0)
				{
					tempIndex = i;
				}
			}
			if ((uartCTRLCmd[i].cmd == cmd) && (uartCTRLCmd[i].shortAddr == shortAddr)) //&& (uartCTRLCmd[i].ep == ep)
			{
				tempIndex = i;
				break;
			}
		}
	}
	if (tempIndex < 10)
	{
		t = time(NULL);
		uartCTRLCmd[tempIndex].cmd = cmd;
		//uartCTRLCmd[tempIndex].ep = ep;
		uartCTRLCmd[tempIndex].shortAddr = shortAddr;
		uartCTRLCmd[tempIndex].sendTimeS = t; //10t��100ms //t >> 8;//(qcom_time_us()) >> 8;//����1024��Լ����100���ڴ�ʱ�侫ȷ��Ҫ�󲻸ߣ�Ϊ�˽�ʡCPU����
		uartCTRLCmd[tempIndex].validFlag = 1;
	}
}

/***************************************************************
�������� userFillUartCmdForControlState
����˵���� �������еĻ��洦���������
***************************************************************/
uint8_t findUartCmdForControlState(uint8_t cmd, uint16_t shortAddr, uint8_t ep)
{
	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		if (uartCTRLCmd[i].validFlag == 1)
		{
			if (uartCTRLCmd[i].cmd == cmd)
			{
				if (shortAddr == 0xffff) //����Ҫƥ��shortaddr ��ep
				{
					break;
				}
				else if ((uartCTRLCmd[i].shortAddr == shortAddr))
				{ //(uartCTRLCmd[i].ep == ep) &&
					break;
				}
			}
		}
	}
	if (i < 10)
	{
		return i;
	}
	else
	{
		return 0xff;
	}
}

void deleteUartCmdForControl(uint8_t index)
{
	if (index < 10)
	{
		memset((uint8_t *)&uartCTRLCmd[index], 0, sizeof(UART_CTRL_CMD_BUF));
	}
}

void clearUartCmdForControl(void)
{
	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		deleteUartCmdForControl(i);
	}
}

void uartManageCmdControlSuccess(void)
{
	HM_DEV_RAM_LIST *pni;
	uint8_t i;
	time_t nowTime;
	for (i = 0; i < 10; i++)
	{
		if (uartCTRLCmd[i].validFlag == 1)
		{
			nowTime = time(NULL);
			if ((nowTime - uartCTRLCmd[i].sendTimeS) > 10)
			{

				//printf("test-------------------%s,control fail,shortAddr:%02x\r\n",__func__,uartCTRLCmd[i].shortAddr);
				printf("Monitor control onoff subdev fail,shortAddr:%02x\n", uartCTRLCmd[i].shortAddr);
				pni = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(uartCTRLCmd[i].shortAddr);
				if (pni == NULL)
				{
					printf("Monitor the short addr not find in dev table,read from zc now\r\n");
				}
				else
				{
					switch (pni->devType)
					{
					case NODE_INFO_TYPE_LIGHT_RGB:
					case NODE_INFO_TYPE_LIGHT_RGBW:
					case NODE_INFO_TYPE_LIGHT_WY:
					case NODE_INFO_TYPE_AI1SW:
					case NODE_INFO_TYPE_AI2SW:
					case NODE_INFO_TYPE_AI3SW:
					case NODE_INFO_TYPE_RELAY_CONTROL:
					case NODE_INFO_TYPE_AI_SOCKET_USB:
					case NODE_INFO_TYPE_AI_SOCKET_MEASURE:
					case NODE_INFO_TYPE_AI_WALLSOCKET_MEASURE:
					case NODE_INFO_TYPE_CUSTOM_RGB_AREA:
					case NODE_INFO_TYPE_AI_CURTAIN_MOTOR:
					case NODE_INFO_TYPE_WARNING_DEV: //hardy add
						if (get_subdev_logon_complete() == 1)
							setNodeOnlineUpdateDetectedEvent(pni, 0);
						break;
					default:
						break;
					}
				}
				deleteUartCmdForControl(i);
			}
		}
	}
}

int readIlluminanceValue(void)
{
	return IlluminanceValue;
}

void uartTxDataCmdDeal(void)
{
	uint8_t i = 0;
	uint8_t index;
	do
	{
		index = userFromInFineValidUartTxBufIndex(i);
		if (index < MAX_UART_TX_CMD_BUF_NUM)
		{
			switch (uartTXCmd[index].inCmd)
			{
			case UART_CMD_PERMIT_JOIN_REQ:
				printf("tx for permit join\r\n");
				transportUartHexDataForPermitJoin(uartTXCmd[index]);
				break;
			case UART_CMD_SEND_ZC_HEART_REQ:
				printf("tx for heart\r\n");
				transportUartHexDataForHeart(uartTXCmd[index]);
				break;
			case UART_CMD_DEV_INFO_REQ:
				printf("tx for read dev info\r\n");
				transportUartHexDataForReadNodeInfo(uartTXCmd[index]);
				break;
			case UART_CMD_READ_ZC_INFO_REQ:
				printf("tx for read zc info\r\n");
				transportUartHexDataForReadZCInfo(uartTXCmd[index]);
				//sleep(1);
				//usleep(20000);usermsleep(50);
				//usleep(100000);
				//usermsleep(100);
				break;
			case UART_CMD_LEAVE_DEV_REQ:
				printf("tx for leave node\r\n");
				transportUartHexDataForLeaveNode(uartTXCmd[index]);
				break;
			case UART_CMD_UPDATE_ZC_SOFT_REQ:
				printf("tx for update zc program\r\n");
				transportUartHexDataForUpdateZc(uartTXCmd[index]); //���ڷ��͸��¹̼�ָ��
				sleep(1);										   //qcom_thread_msleep(1000);//�ȴ�1s���ٷ��Ϳ�ʼ����ָ��
				uartSendXmodemStartOTA();
				break;
			case UART_CMD_WRITE_ATTR_REQ:
				printf("tx for write attr\r\n");
				transportUartHexDataForWriteAttribute(uartTXCmd[index]);
				break;
			case UART_CMD_AIR_CONTROL_REQ:
				printf("tx for write aq value\r\n");
				transportUartHexDataForWriteAQValue(uartTXCmd[index]);
				break;
			case UART_CMD_READ_ATTR_REQ:
				printf("tx for read attr\r\n");
				transportUartHexDataForReadAttribute(uartTXCmd[index]);
				break;
			case UART_CMD_GET_ACTIVE_EP_REQ:
				break;
			case UART_CMD_GET_EP_SMPDST_REQ:
				break;
			case UART_CMD_CFG_ATTR_REPORT_REQ:
				break;
			case UART_CMD_GET_NODE_DESCRIPTOR_REQ:
				break;
			case UART_CMD_SET_ZC_TO_FACTORY_REQ:
				transportUartHexDataForSetZcToFactory(uartTXCmd[index]);
				break;
			case UART_CMD_ONOFF_CONTROL_REQ:
				//printf("tx for on off\r\n");
				transportUartHexDataForOnOff(uartTXCmd[index]);
				break;
			case UART_CMD_LEVEL_CONTROL_REQ:
				printf("tx for level control\r\n");
				transportUartHexDataForLevel(uartTXCmd[index]);
				break;
			case UART_CMD_WD_CONTROL_REQ:
				printf("tx for wd control\r\n");
				transportUartHexDataForWDControl(uartTXCmd[index]);
				break;
			case UART_CMD_IDENTIFY_REQ:
				break;
			case UART_CMD_COLOR_CONTROL_REQ:
				printf("tx for color control\r\n");
				transportUartHexDataForColorControl(uartTXCmd[index]);
				break;
			case UART_CMD_ONOFF_CONTROL_BY_GROUP_REQ:
				printf("tx for on off control by groups\r\n");
				transportUartHexDataForOnOffControlByGroupsOrBroadcast(uartTXCmd[index]);
				break;
			case UART_CMD_RGBLIGHT_CONTROL_REQ:
				printf("tx for cut rgb 4 control\r\n");
				transportUartHexDataForCutRgb(uartTXCmd[index]);
				break;
			case UART_CMD_DOOR_UNLOCK_CONTROL_REQ:
				printf("tx for door lock unlock operation\r\n");
				transportUartHexDataForDoorUnlock(uartTXCmd[index]);
				break;
			case UART_CMD_CURTAIN_MOTOR_CLEAR_PHY_LIMIT_REQ:
				printf("tx for curtain motor clear physics limit\r\n");
				transportUartHexDataForCurtainMotorClearPhyLimit(uartTXCmd[index]);
				break;
			case UART_CMD_SET_RSSI_REPORT_REQ: //hardy add
				printf("tx for for zc set rssi report\r\n");
				transportUartHexDataForSetZcRSSIReport(uartTXCmd[index]);
				break;
			case UART_CMD_ILLUMINANCE_DATA_REQ: //hardy add
				printf("tx for for zc read Illuminance\r\n");
				transportUartHexDataForReadIlluminanceValue(uartTXCmd[index]);
				break;
			case UART_CMD_BIND_REQ:
				printf("tx for for zc bind deviceid\r\n");
				transportUartHexDataForBindFunc(uartTXCmd[index]);
				break;
			case UART_CMD_UNBIND_REQ:
				printf("tx for for zc unbind deviceid\r\n");
				transportUartHexDataForUNBindFunc(uartTXCmd[index]);
				break;
			case UART_CMD_PRIVATEBIND_REQ:
				printf("tx for for zc privatebind deviceid\r\n");
				transportUartHexDataForBindSwitch(uartTXCmd[index]);
				break;
			default:
				break;
			}
			userClearValidUartTxBufIndex(index);
		}
		i = index;
		//usleep(20000);
		usleep(20000);
	} while (i < MAX_UART_TX_CMD_BUF_NUM);
}

/***************************************************************
�������� uartRcvDataDeal
��������� ����zigbeeЭ��Ĵ������ݰ�
��������� ��
����˵���� uart�������ݵĴ���������
***************************************************************/
void uartRcvDataDeal(uint8_t *data, uint16_t dataLen)
{
	uint8_t index, tempOnline;
	uint16_t shortAddr, clusterId, attrId, alarmStatus, attrId2;
	uint16_t alarmCluster;
	uint8_t alarmCode;
	uint8_t cmdId;
	uint8_t aceCom, armMode;
	uint8_t ep;
	uint8_t addrType;
	uint8_t subType, cpbrightness;
	uint8_t tempMac[8];
	uint16_t cpHumi, cpcolorTemp, getData;
	int16_t cpTmep;
	uint16_t colorValue, colorValueX, colorValueY;
	uint64_t cpPowerKwh;
	uint32_t cpPowerKw, batteryAlarmState;
	uint16_t cpVoltage;
	UART_UPDATE_NEW_DEV_DATA_INFO newDevData;
	HM_DEV_RAM_LIST *pRam = NULL;
	uint8_t convertMac[10] = {0};
	//HM_DEV_FLASH_INFO tempFlashData;
	uint16_t devIndex;
	uint8_t i;
	uint8_t tempLQI, tempRSSI;
	time_t now_time;
	if (data == NULL)
	{
		printf("the uart data was null\r\n");
		return;
	}
	if (data[0] == 0xfa)
	{
		if (data[1] != (dataLen - 4)) //Э�����ݳ��Ȳ�������ͷ��������У�顢��β
		{
			printf("the uart data was invalid\r\n");
			return;
		}
		if (calcFCS(&data[3], dataLen - 5) == data[dataLen - 2]) //У����ȷ
		{
			resetZigbeeOtaFlag(); //hardy add
			userSetUartHeartTime();
			//printf("uart data check was success,cmd:%02x\r\n",data[2]);
			uint8_t cmd = data[2];
			switch (cmd)
			{
			case UART_CMD_PERMIT_JOIN_RSP:
				printf("test for uart rcv permit join\r\n");
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					if (data[4] != 0)
					{
						zigbeeNetWorkStation = ZB_NET_WORK_PERMIT_JOINING; //Ӱ��Ƶ�ָʾ״̬�ı�־λ
						zigbeeNetPermitJoinTime = data[4];
						gwStateForRealTime.gwZbIsJoining = 1;
						//zbJoinFlag = 1;//Ϊ�˼���Ƿ����豸�����ж��Ƿ���Ҫupdate echo list
					}
					else
					{
						zigbeeNetWorkStation = ZB_NET_WORK_NORMAL;
						gwStateForRealTime.gwZbIsJoining = 0;
					}
				}
				else
				{
					printf("uart rsp:set zc to permit join was faild\r\n");
				}
				index = findUartCmdMonitor(cmd - 1, MONITOR_FIND_FOR_ZC_SHORTADDR, 1);
				if (index < MAX_UART_CMD_MONITOR_NUM)
				{
					deleteUartCmdMonitor(index);
				}
				break;
			case UART_CMD_LEAVE_DEV_RSP: //�ڴ�ֻ�ܱ�ʾZC���յ�����������ָ������豸��������Ҫ��idctָ��
				index = findUartCmdMonitor(cmd - 1, MONITOR_FINE_FOR_ANY_SHORTADDR, 0);
				if (index < MAX_UART_CMD_MONITOR_NUM)
				{
					deleteUartCmdMonitor(index);
				}
				break;
			case UART_CMD_ONOFF_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					//printf("onoff control send success\r\n"); //test debug
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						uartCmdMonito[index].rspRcvFlag = 1;
						uartCmdMonito[index].cmd = MONITOR_SUB_CMD_ONOFF;
					}
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					//printf("onoff control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					if (data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_ONOFF, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
						//hardy add
						index = findUartCmdForControlState(cmd - 1, shortAddr, ep);
						if (index < 10)
						{
							deleteUartCmdForControl(index);
						}
					}
					else
					{
						index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
				}
				else
				{
					printf("onoff control faild\r\n");
				}

				break;
			case UART_CMD_RGBLIGHT_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("cutrgb control success\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						uartCmdMonito[index].rspRcvFlag = 1;
						uartCmdMonito[index].cmd = MONITOR_SUB_CMD_CUTRGB;
					}
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("cutrgb control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					if (data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_CUTRGB, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
					else
					{
						index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
				}
				else
				{
					printf("cutrgb control faild\r\n");
				}

				break;
			case UART_CMD_COLOR_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("color control success\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						uartCmdMonito[index].rspRcvFlag = 1;
						uartCmdMonito[index].cmd = MONITOR_SUB_CMD_COLOR;
					}
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("color control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					if (data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_COLOR, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
					else
					{
						index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
				}
				else
				{
					printf("color control faild\r\n");
				}

				break;
			case UART_CMD_LEVEL_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("level control success\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						uartCmdMonito[index].rspRcvFlag = 1;
						uartCmdMonito[index].cmd = MONITOR_SUB_CMD_LEVEL;
					}
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("level control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					if (data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_LEVEL, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
					else
					{
						index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
					}
				}
				else
				{
					printf("level control faild\r\n");
				}

				break;
			case UART_CMD_SET_ZC_TO_FACTORY_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS) //ָ��ִ�гɹ��������������
				{
					index = findUartCmdMonitor(cmd - 1, MONITOR_FIND_FOR_ZC_SHORTADDR, 1);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				break;
			case UART_CMD_WD_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("wd control success\r\n");
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || ((data[3] == UART_DEAL_STA_NO_DEV)))
				{
					printf("wd control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				else
				{
					printf("wd control faild\r\n");
				}

				break;
			case UART_CMD_IDENTIFY_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("indentify control success\r\n");
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("indentify control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				else
				{
					printf("indentify control faild\r\n");
				}

				break;
			case UART_CMD_ONOFF_CONTROL_BY_GROUP_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("on off  control by groups success\r\n");
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("on off  control by groups delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					addrType = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, addrType);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				else
				{
					printf("on off  control by groups faild\r\n");
				}

				break;
			case UART_CMD_DOOR_UNLOCK_CONTROL_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("door unlock  control was  success\r\n");
				}
				else if ((data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS) || (data[3] == UART_DEAL_STA_NO_DEV))
				{
					printf("door unlock control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					addrType = data[6];
					if (data[3] == UART_DEAL_STA_NO_DEV)
					{
						userDealWaitCmdRspEvent(shortAddr, ZCL_DOOR_LOCK_CLUSTER_ID, ep, 0);
					}
					else
					{
						userDealWaitCmdRspEvent(shortAddr, ZCL_DOOR_LOCK_CLUSTER_ID, ep, 1);
					}
					index = findUartCmdMonitor(cmd - 1, shortAddr, addrType);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				else
				{
					printf("door unlock  control was faild\r\n");
				}

				break;
			case UART_CMD_CURTAIN_MOTOR_CLEAR_PHY_LIMIT_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("curtain clear physics limit  control was  success\r\n");
				}
				else if ((data[3] == UART_DEAL_STA_NO_DEV) || (data[3] == UART_DEAL_STA_EXE_CONTROL_CMD_SUCCESS))
				{
					printf("curtain clear physics limit control delete monitor\r\n");
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					addrType = data[6];

					index = findUartCmdMonitor(cmd - 1, shortAddr, addrType);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				else
				{
					printf("door unlock  control was faild\r\n");
				}

				break;
			case UART_CMD_READ_ZC_INFO_RSP:
				if (data[1] == 14)
				{
					gwStateForRealTime.zigbeeZcBasicInfo.channel = data[3];
					gwStateForRealTime.zigbeeZcBasicInfo.zcSoftVersion = data[4];
					gwStateForRealTime.zigbeeZcBasicInfo.panId = data[6] + (uint16_t)(data[7] << 8);
					memcpy(gwStateForRealTime.zigbeeZcBasicInfo.zbMac, &data[9], 8);
					printf("zc channel:%d,soft version:%02x,pan id:%d\r\n", gwStateForRealTime.zigbeeZcBasicInfo.channel, gwStateForRealTime.zigbeeZcBasicInfo.zcSoftVersion, gwStateForRealTime.zigbeeZcBasicInfo.panId);
					Record_GatewayVersion();
				}
				else
				{
					printf("zc info rsp len was invalid\r\n");
				}
				break;
			case UART_CMD_SEND_ZC_HEART_RSP:
				break;
			case UART_CMD_UPDATE_ZC_SOFT_RSP:
				printf("update rsp:the zc soft version was not newest them itself:%d\r\n", data[3]);
				break;
			case UART_CMD_WRITE_ATTR_RSP:
			case UART_CMD_AIR_CONTROL_RSP:
				if (data[9] == 0)
				{
					printf("write attr was  success\r\n");
					shortAddr = data[3] + ((uint16_t)data[4] << 8);
					ep = data[5];
					clusterId = data[6] + ((uint16_t)data[7] << 8);
					userDealWaitCmdRspEvent(shortAddr, clusterId, ep, 1);
				}
				else
				{
					printf("write attr was faild\r\n");
				}
				index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
				if (index < MAX_UART_CMD_MONITOR_NUM)
				{
					deleteUartCmdMonitor(index);
				}
				break;
			case UART_CMD_NEW_DEV_IDCT: //���豸��������
				printf("UART_CMD_NEW_DEV_IDCT----UART_UPDATE_NEW_DEV_DATA_INFO:%d,,%d\r\n", sizeof(UART_UPDATE_NEW_DEV_DATA_INFO), dataLen - 5);
				if ((dataLen - 5) <= sizeof(UART_UPDATE_NEW_DEV_DATA_INFO))
				{
					//zbJoinDevNum ++;//Ϊ�˼���Ƿ����豸�����ж��Ƿ���Ҫupdate echo list
					memset(newDevData.macAddr, 0, sizeof(newDevData.macAddr));
					//memcpy(newDevData.macAddr,&data[3],8);//old
					memcpy(convertMac, &data[3], 8);
					printf("convertMac:%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
						   convertMac[0], convertMac[1], convertMac[2], convertMac[3], convertMac[4],
						   convertMac[5], convertMac[6], convertMac[7]);
					BigSmallEndConversion(convertMac, newDevData.macAddr);

					newDevData.netState = data[11];
					newDevData.batteryRemain = data[12];
					newDevData.deviceId = data[13] + ((uint16_t)data[14] << 8);
					newDevData.shortAddr = data[15] + ((uint16_t)data[16] << 8);
					newDevData.epId = data[17];
					memcpy(newDevData.modeId, &data[18], ZIGBEE_MODEID_LEN);
					printf("the new dev mode id:%s\r\n", newDevData.modeId);
					printf("new node id:%04x,devId:%04x,mac:%02X%02X%02X%02X%02X%02X%02X%02X\r\n", newDevData.shortAddr, newDevData.deviceId,
						   newDevData.macAddr[0], newDevData.macAddr[1], newDevData.macAddr[2], newDevData.macAddr[3], newDevData.macAddr[4],
						   newDevData.macAddr[5], newDevData.macAddr[6], newDevData.macAddr[7]);
					if (checkIEEEAddrHasActive(newDevData.macAddr))
					{
						if (dataLen > 36) //������У��Ͱ�β
						{
							printf("userNewDevJoiningDetectedEvent ----->36\r\n");
							userNewDevJoiningDetectedEvent((char *)&newDevData, &data[34], dataLen - 36);
						}
						else
						{
							userNewDevJoiningDetectedEvent((char *)&newDevData, NULL, 0);
						}
					}
					else
					{
						printf("the new dev mac was not valid\r\n");
					}
				}
				else
				{
					printf("the new dev data len was error:%d\r\n", dataLen - 5);
				}
				break;
			case UART_CMD_IAS_ZONE_ALARM_IDCT: //��������
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				alarmStatus = data[9] + ((uint16_t)data[10] << 8);
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc now\r\n");
				}
				else
				{
					setIasAlarmDetectedEvent(pRam, alarmStatus);
				}
				break;
			case UART_CMD_DEV_ALARM_IDCT:
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				alarmCode = data[9];
				alarmCluster = data[10] + ((uint16_t)data[11] << 8);
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc now\r\n");
					//userFillUartCmdForReadDevInfo(shortAddr);
				}
				else
				{
					setAlarmsAlarmDetectedEvent(pRam, alarmCluster, alarmCode);
				}
				break;
			case UART_CMD_IAS_ACE_COM_IDCT:
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				aceCom = data[8];
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc now\r\n");
					//userFillUartCmdForReadDevInfo(shortAddr);
				}
				else
				{
					pRam->epList[0].times = gwStateForRealTime.sysRealTimeFors;
					setAceComDetectedEvent(pRam, aceCom, &data[9], data[1] - 7);
				}
				break;
			case UART_CMD_READ_ATTR_RSP:
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				clusterId = data[6] + ((uint16_t)data[7] << 8);
				attrId = data[9] + ((uint16_t)data[10] << 8);
				if (data[10] == 0)
				{
					printf("read attr success\r\n");
				}
				else
				{
					printf("read attr faild\r\n");
					break;
				}
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("read attr:the short addr not find in dev table,read from zc now\r\n");
					userFillUartCmdForReadDevInfo(shortAddr);
				}
				else
				{
					if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
					{
						if (attrId == ZCL_ON_OFF_ATTRIBUTE_ID)
						{
							setNodeOnoffDetectedEvent(pRam, ep, data[13]);
						}
						else
						{
							printf("for on off cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
					{
						if (attrId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID)
						{
							setNodeLevelDetectedEvent(pRam, ep, data[13]);
						}
						else
						{
							printf("for level cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_COLOR_CONTROL_CLUSTER_ID)
					{
						colorValue = (uint16_t)(data[13] + ((uint16_t)data[14] << 8));
						if (attrId == ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID)
						{
							setNodeColorDetectedEvent(pRam, ep, 'x', colorValue);
						}
						else if (attrId == ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID)
						{
							setNodeColorDetectedEvent(pRam, ep, 'y', colorValue);
						}
						else
						{
							printf("for color cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_POWER_CONFIG_CLUSTER_ID)
					{
						if (attrId == ZCL_BATTERY_PERCENTAGE_REMAINING_ATTRIBUTE_ID)
						{
							setNodeBatteryDetectedEvent(pRam, data[13]);
						}
						else
						{
							printf("for power cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							cpTmep = (int16_t)(data[13] + ((uint16_t)data[14] << 8));
							setNodeTemperatureDetectedEvent(pRam, ep, cpTmep);
						}
						else
						{
							printf("for temp cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							cpHumi = data[13] + ((uint16_t)data[14] << 8);
							setNodeHumilityDetectedEvent(pRam, ep, cpHumi);
						}
						else
						{
							printf("for humi cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_AIR_PM2_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_AIR_PM2_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[13] + ((uint16_t)data[14] << 8));
							setNodePM2DetectedEvent(pRam, ep, getData);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_AIR_CH2O_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_AIR_CH2O_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[13] + ((uint16_t)data[14] << 8));
							setNodeCH2ODetectedEvent(pRam, ep, getData);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
					{
						if (attrId == ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID) //48bit
						{
							cpPowerKwh = data[13] + ((uint64_t)data[14] << 8) + ((uint64_t)data[15] << 16) + ((uint64_t)data[16] << 24) + ((uint64_t)data[17] << 32) + ((uint64_t)data[18] << 40);
							setNodePowerKWHDetectedEvent(pRam, ep, cpPowerKwh);
						}
						else if (attrId == ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID) //24bit
						{
							cpPowerKw = data[13] + ((uint32_t)data[14] << 8) + ((uint32_t)data[15] << 16);
							setNodePowerKWDetectedEvent(pRam, ep, cpPowerKw);
						}
						else
						{
							printf("for simple meter cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_RMS_VOLTAGE_ATTRIBUTE_ID)
						{
							cpVoltage = data[13] + ((uint32_t)data[14] << 8);
							//��ʱ�رռ���������ѹ��⹦��
							//setNodeVoltageDetectedEvent(pRam,ep,cpVoltage);
						}
						else
						{
							printf("for electrical cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID)
					{
						if (attrId == ZCL_CURRENT_ZONE_SENSITIVITY_LEVEL_ATTRIBUTE_ID)
						{
							setNodeSensitivityLevelEvent(pRam, data[13]);
						}
						else
						{
							printf("for simple meter cluster:not valid attr id for read attr\r\n");
						}
					}
					else if (clusterId == ZCL_BASIC_CLUSTER_ID)
					{
						if (attrId == ZCL_DEVICE_ENABLED_ATTRIBUTE_ID)
						{
							setNodeDoorLockRemoteOpenEnableEvent(pRam, data[13]);
						}
						else
						{
							printf("for simple meter cluster:not valid attr id for read attr\r\n");
						}
					}
				}
				break;

			case UART_CMD_ATTR_REPORT_IDCT: //xiugai
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				clusterId = data[6] + ((uint16_t)data[7] << 8);
				attrId = data[9] + ((uint16_t)data[10] << 8);
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc now\r\n");
				}
				else
				{
					printf("the short addr:%04x  data handle\r\n", shortAddr);
					if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_ONOFF, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
						if (attrId == ZCL_ON_OFF_ATTRIBUTE_ID)
						{
							setNodeOnoffDetectedEvent(pRam, ep, data[12]);
						}
						else
						{
							printf("for on off cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_LEVEL_CONTROL_CLUSTER_ID)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_LEVEL, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
						if (attrId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID)
						{
							setNodeLevelDetectedEvent(pRam, ep, data[12]);
						}
						else
						{
							printf("for level cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_COLOR_CONTROL_CLUSTER_ID)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_COLOR, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}
						colorValue = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
						printf("the color value:%d\r\n", colorValue);
						if (attrId == ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID)
						{
							setNodeColorDetectedEvent(pRam, ep, 'x', colorValue);
						}
						else if (attrId == ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID)
						{
							setNodeColorDetectedEvent(pRam, ep, 'y', colorValue);
						}
						else if (attrId == ZCL_COLOR_CONTROL_CURRENT_XY_ATTRIBUTE_ID)
						{
							colorValueX = (uint16_t)(data[25] + ((uint16_t)data[26] << 8));
							colorValueY = (uint16_t)(data[20] + ((uint16_t)data[21] << 8));
							setNodeColorXYDetectedEvent(pRam, ep, colorValueX, colorValueY);
						}
						else if (attrId == ZCL_COLOR_CONTROL_CURRENT_WY_ATTRIBUTE_ID)
						{
							setNodeColorWYDetectedEvent(pRam, ep, colorValue);
						}
						else
						{ //ZCL_COLOR_CONTROL_CURRENT_XY_ATTRIBUTE_ID
							printf("for color cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_POWER_CONFIG_CLUSTER_ID)
					{
						if (attrId == ZCL_BATTERY_PERCENTAGE_REMAINING_ATTRIBUTE_ID)
						{
							setNodeBatteryDetectedEvent(pRam, data[12]);
							if (data[1] == 0x12) //Ϊ�ɱȵ������ض��Ĺ��ܣ�����attrһ��report����
							{
								attrId2 = data[13] + ((uint16_t)data[14] << 8);
								if (attrId2 == ZCL_BATTERY_ALARM_STATE_ATTRIBUTE_ID)
								{
									batteryAlarmState = data[16] + ((uint32_t)data[17] << 8) + ((uint32_t)data[18] << 16) + ((uint32_t)data[19] << 24);
									//setNodeBatteryStateForFBDoorLockEvent(pRam,batteryAlarmState);
								}
							}
						}
						else
						{
							printf("for power cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							cpTmep = (int16_t)(data[12] + ((uint16_t)data[13] << 8));
							setNodeTemperatureDetectedEvent(pRam, ep, cpTmep);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							cpHumi = data[12] + ((uint16_t)data[13] << 8);
							setNodeHumilityDetectedEvent(pRam, ep, cpHumi);
						}
						else
						{
							printf("for humi cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_AIR_PM2_MEASUREMENT_CLUSTER_ID)
					{ //ZCL_AIR_PM2_MEASUREMENT_CLUSTER_ID
						if (attrId == ZCL_AIR_PM2_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
							setNodePM2DetectedEvent(pRam, ep, getData);
							//setNodeTemperatureDetectedEvent(pRam, ep, cpTmep);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_AIR_CH2O_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_AIR_CH2O_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
							setNodeCH2ODetectedEvent(pRam, ep, getData);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_AIR_ACST_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							//getData = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
							getData = (uint16_t)(data[12]);
							setNodeACSTDetectedEvent(pRam, ep, getData);
						}
						else if (attrId == ZCL_AIR_PM10_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
							setNodePM10DetectedEvent(pRam, ep, getData);
						}
						else if (attrId == ZCL_AIR_AQ_MEASURED_VALUE_ATTRIBUTE_ID)
						{
							getData = (uint16_t)(data[12] + ((uint16_t)data[13] << 8));
							setNodeAQDetectedEvent(pRam, ep, getData);
						}
						else
						{
							printf("for temp cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
					{
						if (attrId == ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID) //48bit
						{
							cpPowerKwh = data[12] + ((uint64_t)data[13] << 8) + ((uint64_t)data[14] << 16) + ((uint64_t)data[15] << 24)\ 
					 				+ ((uint64_t)data[16] << 32) +
										 ((uint64_t)data[17] << 40);
							setNodePowerKWHDetectedEvent(pRam, ep, cpPowerKwh);
						}
						else if (attrId == ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID) //24bit -----
						{
							cpPowerKw = data[12] + ((uint32_t)data[13] << 8) + ((uint32_t)data[14] << 16);
							setNodePowerKWDetectedEvent(pRam, ep, cpPowerKw);
						}
						else
						{
							printf("for simple meter cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)
					{
						if (attrId == ZCL_RMS_VOLTAGE_ATTRIBUTE_ID)
						{
							cpVoltage = data[12] + ((uint32_t)data[13] << 8);
							//��ʱ�رռ���������ѹ��⹦��
							//setNodeVoltageDetectedEvent(pRam,ep,cpVoltage);
						}
						else
						{
							printf("for electrical cluster:not valid attr id for report\r\n");
						}
					}
					else if (clusterId == ZCL_RGBLIGHT_CLUSTER_ID)
					{
						index = findUartCmdMonitor(MONITOR_SUB_CMD_CUTRGB, shortAddr, ep);
						if (index < MAX_UART_CMD_MONITOR_NUM)
						{
							deleteUartCmdMonitor(index);
						}

						if (attrId == ZCL_BRIGHTNESS_ATTRIBUTE_ID)
						{
							cpbrightness = data[12];
							//setNodeBrightnessDetectedEvent(pRam,ep,cpbrightness);
						}
						else if (attrId == ZCL_COLOR_TEMPERATURE_ATTRIBUTE_ID)
						{
							cpcolorTemp = data[12] + ((uint32_t)data[13] << 8);
							//setNodeColorTempDetectedEvent(pRam,ep,cpcolorTemp);
						}
						else
						{
							printf("for electrical cluster:not valid attr id for report\r\n");
						}
					}
				}

				break;
			case UART_CMD_DEV_LEAVE_IDCT:
				printf("leave the node\r\n");
				//memcpy(tempMac,&data[3],8);//old
				memcpy(convertMac, &data[3], 8); //old
				BigSmallEndConversion(convertMac, tempMac);

				if (checkIEEEAddrHasActive(tempMac))
				{
					pRam = (HM_DEV_RAM_LIST *)findInDevListByMac(tempMac);
					if (pRam == NULL)
					{
						printf("the leave node is not in dev list\r\n");
					}
					else
					{
						printf("remove the node form the list now,index:%d\r\n", pRam->index);

						userDeleteDevFromLightNightAndHomeArm(pRam->index); //�� hardy add wait test
						userDeleteDevFromWDLinkList(pRam->index);			//���⾯��
						userDeleteDevFromSceneAndLinkage(pRam->index);		//����������

						setNodeRemoveDetectedEvent(pRam->index); //�ϱ�ɾ���¼�
						userDeleteOneDevFromRamAndFlash(pRam);
						gwStateForRealTime.echoNeedUpdateFlag = 1; //ɾ���豸
						pRam = NULL;							   //ɾ���豸��pRam��ΪNULL
					}
				}
				break;
			case UART_CMD_DOOR_OPERATION_EVENT_IDCT:
				printf("door operation event indication\r\n");
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				cmdId = data[8];

				pRam = findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc no\r\n");
					//userFillUartCmdForReadDevInfo(shortAddr);
				}
				else
				{
					setDoorLockOperationNotificationEvent(pRam, cmdId, &data[9]);
				}
				break;
			case UART_CMD_SCENE_ONOFF_REPORT_IDCT:
				printf("scene onoff event report\r\n"); // F1 20
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				ep = data[5];
				cmdId = data[8];

				pRam = findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					//read zc the node mac
					printf("the short addr not find in dev table,read from zc no\r\n");
					//userFillUartCmdForReadDevInfo(shortAddr);
				}
				else
				{
					setSceneSwitchEvent(pRam, cmdId, NULL);
				}
				break;
			case UART_CMD_DEV_UPDATE_SHORT_ADDR_IDCT:
				printf("the dev node id was change ,it is update now\r\n");
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				//memcpy(tempMac,&data[5],8);//old
				memcpy(convertMac, &data[5], 8);
				BigSmallEndConversion(convertMac, tempMac); //new

				pRam = (HM_DEV_RAM_LIST *)findInDevListByMac(tempMac);
				if (pRam == NULL)
				{
					printf("the update node is not in dev list\r\n");
				}
				else
				{
					pRam->shortAddr = shortAddr;
					userUpdateDevInfoDataToFlash(pRam);
				}
				break;
			case UART_CMD_DEV_OFFLINE_IDCT:
				printf("set dev on off line\r\n");
				//hardy test add
				shortAddr = data[3] + ((uint16_t)data[4] << 8);
				pRam = (HM_DEV_RAM_LIST *)findInDevListByNwkadd(shortAddr);
				if (pRam == NULL)
				{
					printf("the update node is not in dev list\r\n");
				}
				else
				{
					if (data[5]) //�ϱ���״̬��1Ϊ���ߣ���wifi��Э���෴
					{
						tempOnline = 0;
					}
					else
					{
						tempOnline = 1;
					}
					printf("test-------setNodeOnlineUpdateDetectedEvent------02\r\n");
					setNodeOnlineUpdateDetectedEvent(pRam, tempOnline);
					//(for working lixiong)�ϱ�app�豸���ߡ�����
				}
				break;
			case UART_CMD_SET_RSSI_REPORT_RSP:
				printf("set rssi report succees\r\n");
				if (data[3] == UART_DEAL_STA_SUCCESS) //ָ��ִ�гɹ��������������
				{
					index = findUartCmdMonitor(cmd - 1, MONITOR_FIND_FOR_ZC_SHORTADDR, 1);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						deleteUartCmdMonitor(index);
					}
				}
				break;
			case UART_CMD_DEV_RSSI_IDCT:
				break;
			case UART_CMD_ILLUMINANCE_DATA_REP:
				if (data[3] == UART_DEAL_NIGHT_STATUS) //alarmCode
				{
					IlluminanceValue = 0;
				}
				else if (data[3] == UART_DEAL_DAY_STATUS)
				{
					IlluminanceValue = 1;
				}
				break;
			case UART_CMD_BIND_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					//printf("onoff control send success\r\n"); //test debug
					shortAddr = data[4] + ((uint16_t)data[5] << 8);
					ep = data[6];
					index = findUartCmdMonitor(cmd - 1, shortAddr, ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						// uartCmdMonito[index].rspRcvFlag = 1;
						//uartCmdMonito[index].cmd = MONITOR_SUB_CMD_ONOFF;
					}
				}
				break;
			case UART_CMD_UNBIND_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("set UART_CMD_UNBIND_RSP success\r\n"); //test debug
																   //shortAddr = data[4] + ((uint16_t)data[5] << 8);
																   // ep = data[6];
																   // index = findUartCmdMonitor(cmd - 1,shortAddr,ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						// uartCmdMonito[index].rspRcvFlag = 1;
						//uartCmdMonito[index].cmd = MONITOR_SUB_CMD_ONOFF;
					}
				}
				break;
			case UART_CMD_PRIVATEBIND_RSP:
				if (data[3] == UART_DEAL_STA_SUCCESS)
				{
					printf("set UART_CMD_PRIVATEBIND_RSP success\r\n"); //test debug
																		//shortAddr = data[4] + ((uint16_t)data[5] << 8);
																		//  ep = data[6];
																		//  index = findUartCmdMonitor(cmd - 1,shortAddr,ep);
					if (index < MAX_UART_CMD_MONITOR_NUM)
					{
						// uartCmdMonito[index].rspRcvFlag = 1;
						//uartCmdMonito[index].cmd = MONITOR_SUB_CMD_ONOFF;
					}
				}
				break;
			default:
				break;
			}

			if (pRam != NULL)
			{
				if ((cmd != UART_CMD_DEV_OFFLINE_IDCT) && (cmd != UART_CMD_READ_ZC_INFO_RSP))
				{ //
					time(&now_time);
					gwStateForRealTime.sysRealTimeFors = (unsigned int)now_time;
					pRam->lastCommunicationTimes = gwStateForRealTime.sysRealTimeFors;
					if (pRam->onOffLineState != 0) //�����ȡָ���־�������ȡָ���־��Ϊ0���ٴν���check����ʱ����ֱ���ø��豸����
					{
						pRam->onOffLineState = 0;
					}
					if (pRam->onLine == 0) //offline
					{
						printf("on line for dev uart rcv cmd\r\n");
						tempOnline = 1;
						printf("test-------setNodeOnlineUpdateDetectedEvent------03\r\n");
						//setNodeOnlineUpdateDetectedEvent(pRam,tempOnline); //���豸�����ϱ�
						setNodeOnlineUpdateDetectedEvent(pRam, tempOnline);
					}
				}
			}
		}
		else
		{
			printf("uart rcv check was error\r\n");
		}
	}
}

// ���ڷ����߳� //
void *uart_sent_thread(void *unused)
{
	uint8_t ret;
	uint8_t monitorTime = 0;
	uint8_t controlAckTime = 0;
	uint8_t loopTimes = 0;
	while (1)
	{
		if (devOtaUpdateingFlag == 0) //ȷ����OTAʱ�����ʹ�������
		{
			if (zbSysStartFlag == 0) //��һ�ζ�ȡzc info�����һֱû�ж�����Ч�Ĵ������ݣ�zbSysStartFlagһֱ����1
			{
				uartTxDataCmdDeal(); //sent uart data  //35ms loop once
				monitorTime++;
				//printf("mc:%d\n",monitorTime);//test log ---uartTxDataCmdDeal
				if (monitorTime >= 10) //150////if(monitorTime >= 100)//500ms
				{					   //enter 350ms loop onece //
					//prtlog("----------------------now time: %d\r\n",t);
					monitorTime = 0;
					userManageUartHeart();		// sent zigbee heart
					userManageUartCmdMonitor(); // ack recv uart data
				}
				controlAckTime++;
				if (controlAckTime >= 150) //5s ����
				{
					uartManageCmdControlSuccess();
				}
			}
			else
			{
			}
		}
		usermsleep(30);

		//userFillUartCmdForReadZCnfo();usermsleep(150);
	}
}

void uart_work_task_init(void)
{
	int ret, rcvLen;
	int i, tempLen;
	time_t t;
	unsigned char rcvBuf[128], dealBuf[128];
	//	uint8_t zbSysStartFlag = 2;
	uint8_t monitorTime = 0;
	uint8_t controlAckTime = 0;
	uint32_t sysTime100ms;
	uint8_t time_buff[20] = {0};
	pthread_create(&executePlayerWork_id, NULL, &execute_player_voice_thread, NULL);
	pthread_detach(executePlayerWork_id);
	clearUartCmdMonitor();
	clearUartCmdForControl();
	printf("UART start task\r\n");
	ret = XmUARTInit("/dev/ttyS0"); //"/dev/ttyS1"
	if (ret != 0)
	{
		printf("UARTInit faile\r\n");
		return;
	}
	else
	{
		printf("UARTInit ok\r\n");
	}
	usleep(500000);
	zbSysStartFlag = 0;
	printf("test uart data-------------------000\n");
	userFillUartCmdForReadZCnfo(); //usermsleep(5);//usleep(20000);
	pthread_create(&uartSentWork_id, NULL, &uart_sent_thread, NULL);
	pthread_detach(uartSentWork_id);

	while (1)
	{
		//printf("test uart data-------------------001\n");

		rcvLen = XmReadBuf(uartRXData[0].data, 128); //uartRXData[0].data  rcvBuf

		//test //
		//test
		if (rcvLen <= 0)
		{	  //add = hardy add
			; //no message handle
		}
		else
		{
			if (rcvLen == 1)
			{
				printf("uart_work_task_init-------wait \r\n");
				printf("%d\r\n", uartRXData[0].data[0]);
				if (otaStationInfo.devUpdateNeedFlag != NO_DEV_NEED_UPDATE_IMAGE)
				{
					if (devOtaUpdateingFlag == 0)
					{
						if ((uartRXData[0].data[0] == XMODEM_NAK) || (uartRXData[0].data[0] == 'C'))
						{
							userOtaZigbeeGWCheck++;
							if (userOtaZigbeeGWCheck >= 2)
							{
								if (otaStationInfo.firmwareValid == 1)
								{
									otaStationInfo.needDownloadImageFromCloud = 0;
									devOtaUpdateingFlag = 1;
									zcXmodemStation = XUSER_START;
								}
								else
								{
									otaStationInfo.needDownloadImageFromCloud = 1;
								}
							}
						}
					}
					else if (devOtaUpdateingFlag == 1)
					{
						if (uartRXData[0].data[0] == XMODEM_ACK)
						{
							if (zcXmodemStation == XUSER_WAIT_COMMUNITE_ACK)
							{
								zcXmodemStation = XMODEM_ACK;
							}
							else if (zcXmodemStation == XUSER_WAIT_END_ACK)
							{
								zcXmodemStation = XUSER_END;
							}
						}
						else if (uartRXData[0].data[0] == XMODEM_CAN)
						{
							zcXmodemStation = XMODEM_CAN;
						}
						else if (uartRXData[0].data[0] == XMODEM_NAK)
						{
							zcXmodemStation = XMODEM_NAK;
						}
					}
				}
			}
			else
			{
				if (devOtaUpdateingFlag == 0)
				{
					if (uartRXData[0].data[rcvLen - 1] == 'C')
					{
						resetZigbeeOtaFlag(); //hardy add ��Ҫ�����ʱ����wifi����
						printf("test ---------------------zigbee ota\r\n");
						if (uartRXData[0].data[rcvLen - 2] == 0x0a)
						//if(userOtaZigbeeGWCheck >= 2)
						{
							if (otaStationInfo.firmwareValid == 1)
							{
								otaStationInfo.needDownloadImageFromCloud = 0;
								devOtaUpdateingFlag = 1;
								zcXmodemStation = XUSER_START;
								printf("-zigbee ota zcXmodemStation = XUSER_START\r\n");
							}
							else
							{
								otaStationInfo.needDownloadImageFromCloud = 1;
							}
						}
					}
				}

				do
				{
					// �ж�ZC firmware OTA��ɣ���Ҫstart app
					if ((zcXmodemStation == XUSER_END) || (XUSER_WAIT_END_ACK == zcXmodemStation))
					{
						if (strstr(uartRXData[0].data, "Serial upload complete") != NULL)
						{
							zcXmodemStation = XUSER_RESTART_APP;
							break;
						}
					}

					if (zbSysStartFlag != 0)
					{
						if (strstr(uartRXData[0].data, "zb app start") != NULL)
						{
							//qcom_thread_msleep(100);
							usleep(100);
							//userFillUartCmdForSetZCReportRssi();//set zigbee rssi
							printf("zb app check ok\r\n");
							zbSysStartFlag = 0; //zigbee ģ������app����
							break;
						}
					}

					userOtaZigbeeGWCheck = 0; //���ȷ��OTAָ�������

					for (i = 0; i < rcvLen; i++) //len
					{
						if (uartRXData[0].data[i] == UART_DATA_HEAD)
						{
							if ((uartRXData[0].data[i + 1] < MAX_UART_RX_BUF_LENTH - 3) && (uartRXData[0].data[i + 1] >= 2))
							{
								tempLen = uartRXData[0].data[i + 1];
								if (uartRXData[0].data[i + tempLen + 3] == UART_DATA_END)
								{
									//��ע: ��Ҫ��uart OTA���˺���һ֡�������ݣ���ȷ��ZC�ɹ�����
									t = time(NULL);
									sysTime100ms = t * 10;							   //10*100ms t / 100;//sysTime100ms = time_ms() / 100;//qcom_time_us() / 100000;
									uartHeartManage.lastCommuTime100ms = sysTime100ms; //��������������������ʱ��
									uartHeartManage.uartWorkDataType = UART_WORK_COMMUNICATION;
									uartRcvDataDeal(&uartRXData[0].data[i], tempLen + 4);
									i += tempLen + 3;
								}
							}
						}
					}

				} while (0);
			}
		}

		//userFillUartCmdForReadZCnfo();
		/*if(devOtaUpdateingFlag == 0)//ȷ����OTAʱ�����ʹ�������
		{
			if (zbSysStartFlag == 0)//��һ�ζ�ȡzc info�����һֱû�ж�����Ч�Ĵ������ݣ�zbSysStartFlagһֱ����1
			{
				uartTxDataCmdDeal(); //sent uart data  //35ms loop once
				monitorTime ++;
				//printf("mc:%d\n",monitorTime);//test log ---uartTxDataCmdDeal
				if(monitorTime >= 10)//150////if(monitorTime >= 100)//500ms 
				{	//enter 350ms loop onece //
					//prtlog("----------------------now time: %d\r\n",t);
					monitorTime = 0;
					userManageUartHeart();// sent zigbee heart 
					userManageUartCmdMonitor();// ack recv uart data
				}
				controlAckTime ++;
				if(controlAckTime >= 150)//5s ����
				{
					uartManageCmdControlSuccess();
				}
			}else{
				
			}
		}*/
		//usleep(300000);//ok

		//usermsleep(7);//usermsleep(7);//usermsleep(5); 15
		//usleep(5000);
		//usleep(5000);//hardy add
		//usermsleep(10);
		usermsleep(20);

		/*struct timeval    tv;   
    	gettimeofday(&tv, NULL); 
    	printf("gettimeofday tv_sec:%ld\n",tv.tv_sec);
		//memset(time_buff,0,sizeof(time_buff));
		//sprintf(time_buff,"%"PRId64,((unsigned long long)(tv.tv_sec*1000 + tv.tv_usec/1000)));//ms
    	//printf("gettimeofday ms:%s\n",time_buff); //sprintf(time_buff,"%"PRId64,((unsigned long long)(now_time))*1000);
		//printf("microsecond:%ld\n",tv.tv_sec*1000000 + tv.tv_usec);  //΢��
		unsigned long long t_data;
		t_data = tv.tv_sec*1000 + tv.tv_usec/1000;
		sprintf(time_buff,"%"PRId64,((unsigned long long)(t_data)));
		printf("--new gettimeofday ms:%s\n",time_buff); */
		//usleep(10000);
		//usermsleep(20);//usleep(20000);//10ms
	}

	/*usleep(1000);
	userSendtest1();
	usleep(1000);
	rcvLen = XmReadBuf(rcvBuf,128);
	if(rcvLen>0){
		printf("rcvLen=%d\r\n",rcvLen);
		printf("recvBuf:");
		for(i=0;i<rcvLen;i++){
		printf("%02x ",rcvBuf[i]);
			}
		printf("\r\n");
	}

	usleep(1000);
	userSendtest2();
	usleep(1000);
	rcvLen = XmReadBuf(rcvBuf,128);
	if(rcvLen>0){
		printf("rcvLen=%d\r\n",rcvLen);
		printf("recvBuf:");
		for(i=0;i<rcvLen;i++){
		printf("%02x ",rcvBuf[i]);
			}
		printf("\r\n");
	}
	while(1){
		sleep(5);
		}*/
}
