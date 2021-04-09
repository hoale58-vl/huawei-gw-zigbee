#include "hm_otaDefine.h"

extern DEV_OTA_STATION otaStationInfo;
uint8_t zcXmodemStation;
uint8_t devOtaUpdateingFlag;
extern GWSTATFORTIME gwStateForRealTime;
extern DEV_OTA_STATION otaStationInfo;
OTACONTROL otaContrlInfo;
static uint8_t zbota_complete_counts = 0, zbreset_timeout_counts = 0; //complete
unsigned char FirstPowerOnZBOTAFlag;
uint8_t openFileFlag = 0;

// Table of CRC constants - implements x^16+x^12+x^5+1
const uint16_t crc16_tab[] = {
	0x0000,
	0x1021,
	0x2042,
	0x3063,
	0x4084,
	0x50a5,
	0x60c6,
	0x70e7,
	0x8108,
	0x9129,
	0xa14a,
	0xb16b,
	0xc18c,
	0xd1ad,
	0xe1ce,
	0xf1ef,
	0x1231,
	0x0210,
	0x3273,
	0x2252,
	0x52b5,
	0x4294,
	0x72f7,
	0x62d6,
	0x9339,
	0x8318,
	0xb37b,
	0xa35a,
	0xd3bd,
	0xc39c,
	0xf3ff,
	0xe3de,
	0x2462,
	0x3443,
	0x0420,
	0x1401,
	0x64e6,
	0x74c7,
	0x44a4,
	0x5485,
	0xa56a,
	0xb54b,
	0x8528,
	0x9509,
	0xe5ee,
	0xf5cf,
	0xc5ac,
	0xd58d,
	0x3653,
	0x2672,
	0x1611,
	0x0630,
	0x76d7,
	0x66f6,
	0x5695,
	0x46b4,
	0xb75b,
	0xa77a,
	0x9719,
	0x8738,
	0xf7df,
	0xe7fe,
	0xd79d,
	0xc7bc,
	0x48c4,
	0x58e5,
	0x6886,
	0x78a7,
	0x0840,
	0x1861,
	0x2802,
	0x3823,
	0xc9cc,
	0xd9ed,
	0xe98e,
	0xf9af,
	0x8948,
	0x9969,
	0xa90a,
	0xb92b,
	0x5af5,
	0x4ad4,
	0x7ab7,
	0x6a96,
	0x1a71,
	0x0a50,
	0x3a33,
	0x2a12,
	0xdbfd,
	0xcbdc,
	0xfbbf,
	0xeb9e,
	0x9b79,
	0x8b58,
	0xbb3b,
	0xab1a,
	0x6ca6,
	0x7c87,
	0x4ce4,
	0x5cc5,
	0x2c22,
	0x3c03,
	0x0c60,
	0x1c41,
	0xedae,
	0xfd8f,
	0xcdec,
	0xddcd,
	0xad2a,
	0xbd0b,
	0x8d68,
	0x9d49,
	0x7e97,
	0x6eb6,
	0x5ed5,
	0x4ef4,
	0x3e13,
	0x2e32,
	0x1e51,
	0x0e70,
	0xff9f,
	0xefbe,
	0xdfdd,
	0xcffc,
	0xbf1b,
	0xaf3a,
	0x9f59,
	0x8f78,
	0x9188,
	0x81a9,
	0xb1ca,
	0xa1eb,
	0xd10c,
	0xc12d,
	0xf14e,
	0xe16f,
	0x1080,
	0x00a1,
	0x30c2,
	0x20e3,
	0x5004,
	0x4025,
	0x7046,
	0x6067,
	0x83b9,
	0x9398,
	0xa3fb,
	0xb3da,
	0xc33d,
	0xd31c,
	0xe37f,
	0xf35e,
	0x02b1,
	0x1290,
	0x22f3,
	0x32d2,
	0x4235,
	0x5214,
	0x6277,
	0x7256,
	0xb5ea,
	0xa5cb,
	0x95a8,
	0x8589,
	0xf56e,
	0xe54f,
	0xd52c,
	0xc50d,
	0x34e2,
	0x24c3,
	0x14a0,
	0x0481,
	0x7466,
	0x6447,
	0x5424,
	0x4405,
	0xa7db,
	0xb7fa,
	0x8799,
	0x97b8,
	0xe75f,
	0xf77e,
	0xc71d,
	0xd73c,
	0x26d3,
	0x36f2,
	0x0691,
	0x16b0,
	0x6657,
	0x7676,
	0x4615,
	0x5634,
	0xd94c,
	0xc96d,
	0xf90e,
	0xe92f,
	0x99c8,
	0x89e9,
	0xb98a,
	0xa9ab,
	0x5844,
	0x4865,
	0x7806,
	0x6827,
	0x18c0,
	0x08e1,
	0x3882,
	0x28a3,
	0xcb7d,
	0xdb5c,
	0xeb3f,
	0xfb1e,
	0x8bf9,
	0x9bd8,
	0xabbb,
	0xbb9a,
	0x4a75,
	0x5a54,
	0x6a37,
	0x7a16,
	0x0af1,
	0x1ad0,
	0x2ab3,
	0x3a92,
	0xfd2e,
	0xed0f,
	0xdd6c,
	0xcd4d,
	0xbdaa,
	0xad8b,
	0x9de8,
	0x8dc9,
	0x7c26,
	0x6c07,
	0x5c64,
	0x4c45,
	0x3ca2,
	0x2c83,
	0x1ce0,
	0x0cc1,
	0xef1f,
	0xff3e,
	0xcf5d,
	0xdf7c,
	0xaf9b,
	0xbfba,
	0x8fd9,
	0x9ff8,
	0x6e17,
	0x7e36,
	0x4e55,
	0x5e74,
	0x2e93,
	0x3eb2,
	0x0ed1,
	0x1ef0,
};

void zigbeeModeReset(void)
{
	//	qca_gpio_set_pin_status(ZIGBEE_RESET,0);
	//	us_delay(100000);
	//	qca_gpio_set_pin_status(ZIGBEE_RESET,1);
	zigbeeReset_setlevel(0);
	usleep(100000);
	zigbeeReset_setlevel(1);
}

void zigbeeModeExternalWatchdog(void)
{
	/*static uint8_t reset_flag=0;
  if(reset_flag<10){
  	reset_flag++;
	qca_gpio_set_pin_status(ZIGBEE_RESET,0);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	us_delay(100000);us_delay(100000);
	//qcom_thread_msleep(100);qcom_thread_msleep(100);
	qca_gpio_set_pin_status(ZIGBEE_RESET,1);
	printf("zigbee Mode External Watchdog,%d\r\n",reset_flag);
  	}*/
}

uint16_t do_crc16_check(unsigned char *data, uint16_t len)
{
	uint16_t i;
	uint16_t chk = 0;
	for (i = 0; i < len; i++)
	{
		chk = crc16_tab[((chk >> 8) ^ data[i]) & 0xFF] ^ (chk << 8);
	}
	return chk;
}
// ��������֡
void uartSendXmodemRecord(unsigned char *buff, uint8_t pack_counter)
{
	int i;
	uint16_t crc_result;
	uint8_t tempData[140];
	tempData[0] = XMODEM_HEAD;
	tempData[1] = pack_counter;
	tempData[2] = 255 - pack_counter;

	memcpy(&tempData[3], buff, XMODEM_DATA_SIZE);
	crc_result = do_crc16_check(buff, XMODEM_DATA_SIZE);
	tempData[3 + XMODEM_DATA_SIZE] = (crc_result >> 8) & 0xff;
	tempData[4 + XMODEM_DATA_SIZE] = crc_result & 0xff;
	//qca_uart_send(tempData,XMODEM_DATA_SIZE + 5);
	//userUartSendWithDebugFunc(tempData,XMODEM_DATA_SIZE + 5);
	XmWriteBuf(tempData, XMODEM_DATA_SIZE + 5);
	/*printf("send data:\r\n");
	for(i = 0; i < XMODEM_DATA_SIZE + 5; i ++)
	{
		printf("%02x",tempData[i]);
	}*/
}

void uartSendXmodemCancel(void)
{
	uint8_t data = XMODEM_CAN;
	//qca_uart_send(&data,1);
	XmWriteBuf(&data, 1); //userUartSendWithDebugFunc(&data,1);
}

void uartSendXmodemOTAEnd(void)
{
	uint8_t data = XMODEM_EOT;
	//qca_uart_send(&data,1);
	XmWriteBuf(&data, 1); //userUartSendWithDebugFunc(&data,1);
}

void uartSendXmodemStartOTA(void)
{
	uint8_t data = 'u';
	XmWriteBuf(&data, 1); //userUartSendWithDebugFunc(&data,1);
						  //qca_uart_send(&data,1);
}

void uartSendXmodemExApp(void)
{
	uint8_t data = 'r';
	XmWriteBuf(&data, 1); //userUartSendWithDebugFunc(&data,1);
						  //qca_uart_send(&data,1);
}

void userXmodemTransport(void)
{
	FILE *fp;
	char complete, retry_num;
	uint8_t frame_data[XMODEM_DATA_SIZE];
	uint8_t pack_counter, i;
	uint16_t real_pack_counter; //��ʵ��packet counter����ΪXMODEM��packet counter��255�����
	uint32_t imageAddr = 0;		//START_ADDR_FLASH_SUB_FIRMWARE_ADDR;
	int32_t remainImageSize = 0;
	uint8_t waitNum = 0;
	uint8_t waitEnd = 0;
	uint8_t endFlag = 0;
	uint8_t parm[4];
	uint8_t data = 0xff;
	//uint8_t openFileFlag = 0;
	struct stat statbuf;
	pack_counter = 0;
	real_pack_counter = 0;
	complete = 0;

	//zcXmodemStation = XMODEM_IDLE;
	printf("enter user send xmodem data for ota:%d\r\n", otaStationInfo.devImageSize);
	remainImageSize = otaStationInfo.devImageSize;

	while (complete == 0)
	{
		switch (zcXmodemStation)
		{
		case XMODEM_IDLE:
			//printf("set XMODEM_IDLE\r\n");
			break;
		case XMODEM_ACK: // ��ȷӦ��
		case XUSER_START:
			//printf("xmode start\r\n");
			if (!openFileFlag)
			{
				openFileFlag = 1;
				//fp = fopen("/root/user_d2.bin", "rb+");//old
				fp = fopen("/root/ota/exe/zigbee.gbl", "rb+");
				if (fp == NULL)
				{
					printf("no zigbee ota file\r\n");
					return;
				}
				fseek(fp, 0, SEEK_SET);
				//int fLen = ftell(fp);  // �ļ�����
				//	stat("/root/user_d2.bin",&statbuf);//old
				stat("/root/ota/exe/zigbee.gbl", &statbuf);
				int fLen = statbuf.st_size;
				otaStationInfo.devImageSize = fLen;
				remainImageSize = fLen;
				printf("test ------remainImageSize:%d\r\n", remainImageSize);
			}
			retry_num = 0;
			pack_counter++;
			real_pack_counter++;
			//printf("xmode couter:%d\r\n",real_pack_counter);
			memset(frame_data, 0, XMODEM_DATA_SIZE); // ����������
			if (remainImageSize > 0)
			{
				if (remainImageSize < XMODEM_DATA_SIZE)
				{
					//qca_load_flash_params(imageAddr,frame_data,remainImageSize);
					fread(frame_data, 1, remainImageSize, fp);
					for (i = 0; i < (XMODEM_DATA_SIZE - remainImageSize); i++)
					{
						frame_data[remainImageSize + i] = CTRLZ;
					}
					remainImageSize = 0;
				}
				else
				{
					//qca_load_flash_params(imageAddr,frame_data,XMODEM_DATA_SIZE);
					fread(frame_data, 1, XMODEM_DATA_SIZE, fp);
					remainImageSize = otaStationInfo.devImageSize - real_pack_counter * XMODEM_DATA_SIZE;
					printf("xmode remainImageSize:%d,otaStationInfo.devImageSize:%d\r\n", remainImageSize, otaStationInfo.devImageSize);
				}
				imageAddr += XMODEM_DATA_SIZE;
				uartSendXmodemRecord(frame_data, pack_counter); // ��������֡
				zcXmodemStation = XUSER_WAIT_COMMUNITE_ACK;		// ��Ӧ��
			}
			else
			{
				uartSendXmodemOTAEnd();
				zcXmodemStation = XUSER_WAIT_END_ACK;
				endFlag = 1;
			}
			waitNum = 0; //����ȴ�ʱ���ʱ��
			break;
		case XMODEM_NAK: // �����ط�
			printf("xmode resend\r\n");
			if (retry_num++ > 10) //Retry too many times
			{
				complete = 1;
				uartSendXmodemCancel();
				uartSendXmodemCancel();
				uartSendXmodemCancel();
				if (real_pack_counter > 1) //�������������;ȡ����������zigbeeģ��
				{
					zigbeeModeReset();
				}
				else
				{
					devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
				}
			}
			else
			{
				if (endFlag == 0)
				{
					uartSendXmodemRecord(frame_data, pack_counter); // �ط�
					zcXmodemStation = XUSER_WAIT_COMMUNITE_ACK;
				}
				else
				{
					uartSendXmodemOTAEnd();
					zcXmodemStation = XUSER_WAIT_END_ACK;
				}
			}
			break;
		case XMODEM_CAN:
			printf("xmode not allow send again\r\n");
			if (real_pack_counter > 1) //�������������;ȡ����������zigbeeģ��
			{
				zigbeeModeReset();
			}
			else
			{
				devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
			}
			complete = 1;
			break;
		case XUSER_WAIT_COMMUNITE_ACK:
		case XUSER_WAIT_END_ACK:
			//printf("xmode wait acck\r\n");
			waitNum++;
			if (waitNum >= 50)
			{
				if (real_pack_counter > 1) //�������������;ȡ����������zigbeeģ��
				{
					if (++zbreset_timeout_counts < 3)
					{
						zigbeeModeReset();
					}
					else
					{ //zigbee ota fail,enter wifi ota mode
						otaContrlInfo.ota_type = 2;
						printf("set ---------otaContrlInfo.ota_type = 2  0001\r\n");
						otaContrlInfo.ota_num = 0;
						otaContrlInfo.ota_status = 1; // zigbee fail
						printf("ota test otaContrlInfo.ota_status -----------02\r\n");
						gwStateForRealTime.wifiSoftUpdate.appAllowSoftWareUpdate = 1;
					}
				}
				else
				{
					devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
				}
				complete = 1;
				printf("wait time out\r\n");
			}
			//qcom_thread_msleep(50);
			usleep(50000);
			break;
		case XUSER_END:
			printf("xmode wait zc rsp complet\r\n");
			waitEnd++;
			if (waitEnd >= 30)
			{
				zigbeeModeReset();
			}
			complete = 1;
			usleep(50000); //qcom_thread_msleep(50);
			break;
		case XUSER_RESTART_APP:
			printf("xmode end--------------otaContrlInfo.ota_type:%d\r\n", otaContrlInfo.ota_type);
			if (openFileFlag == 1)
			{
				fclose(fp);
				openFileFlag = 0;
				printf("clear openFileFlag --------------\r\n");
			}
			/*
				parm[0] = OTA_MODE_ZC;
				parm[1] = HM_IOTS_SMALL_GW_TYPE & 0xff;
				parm[2] = (HM_IOTS_SMALL_GW_TYPE >> 8) & 0xff;
				parm[3] = 1;
				*/
			if (otaContrlInfo.ota_type == 1) // ֻ����ZC xmode�����ɹ��������WIFI �̼�����������ZC�޷�����
			{								 //xmode����һ����ʼ�����ʧ�ܣ�������������ɹ������������WIFI ������
				otaContrlInfo.ota_type = 2;	 //wifi �̼�������ʼ
				printf("set ---------otaContrlInfo.ota_type = 2  0002\r\n");
				otaContrlInfo.ota_num = 0;
				otaContrlInfo.ota_status = 0;
				gwStateForRealTime.wifiSoftUpdate.appAllowSoftWareUpdate = 1;
				printf("zigebee ota success,and wifi ota\r\n");
				if (is_file_exist("/root/user_d1.bin") == 0)
				{
					gwStateForRealTime.wifiSoftUpdate.haveNewestSoftWare = 1;
				}
				else
				{
					gwStateForRealTime.wifiSoftUpdate.haveNewestSoftWare = 0;
				}
				if (is_file_exist("/root/ota/exe/hs3gw-app1") == 0)
				{
					gwStateForRealTime.wifiSoftUpdate.haveNewestSoftWare = 1;
				}
				else
				{
					gwStateForRealTime.wifiSoftUpdate.haveNewestSoftWare = 0;
				}
			}
			if (gwStateForRealTime.wifiSoftUpdate.haveNewestSoftWare == 0)
			{
				//parm[0] = OTA_MODE_WIFI;
				//parm[1] = HM_IOTS_SMALL_GW_TYPE & 0xff;
				//parm[2] = (HM_IOTS_SMALL_GW_TYPE >> 8) & 0xff;
				parm[3] = 1;
				printf("call hmSetIotsEvent\r\n");
				// hmSetIotsEvent(HFZ_EVENT_SOURCE_OTA, 0, (void *)&parm, 4);
			}
			usleep(800000); //qcom_thread_msleep(800);
			if (gwStateForRealTime.zigbeeZcBasicInfo.zcSoftVersion == 0)
			{
				printf("---------------test--add--------------------------\r\n");
				printf("sys reset for dev ota ok\r\n");
				uartSendXmodemExApp();
				usleep(500000); //qcom_thread_msleep(500);
				//userCheckSaveFlashAndReset(1);//֤��zigbee��ʼ����ʱ��δ�ɹ��������ڴ���Ҫ�����豸 del

				//add
				gwStateForRealTime.zigbeeZcBasicInfo.zcSoftVersion = otaStationInfo.devImageVersion; //�汾�Ÿ��£��ڴ˲�����������ram�еİ汾����Ҫ�ֶ�����
				uartSendXmodemExApp();
				devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
				complete = 1;
				//add
			}
			else
			{
				gwStateForRealTime.zigbeeZcBasicInfo.zcSoftVersion = otaStationInfo.devImageVersion; //�汾�Ÿ��£��ڴ˲�����������ram�еİ汾����Ҫ�ֶ�����
				uartSendXmodemExApp();
				devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
				complete = 1;
			}
			break;
		default:
			printf("xmode default\r\n");
			complete = 1;
			uartSendXmodemCancel();
			uartSendXmodemCancel();
			uartSendXmodemCancel();
			if (real_pack_counter > 1) //�������������;ȡ����������zigbeeģ��
			{
				zigbeeModeReset();
			}
			else
			{
				devOtaUpdateingFlag = 0; //ֻ�е���ȫ������Ż���û�����ع����ͽ���Ϊ����
			}
			break;
		}
	}
	//hardy add
	if (FirstPowerOnZBOTAFlag == 1 && zcXmodemStation != XUSER_RESTART_APP)
	{
		printf("enter FirstPowerOnZBOTAFlag copare complete\r\n");
		if (complete == 1)
		{
			zbota_complete_counts++;
			if (zbota_complete_counts < 1)
			{
				printf("again start zigbee zc ota\r\n");
				devOtaUpdateingFlag = 1;
				//gwStateForRealTime.gwDevIsOta = 2;//�������Ʊ�־
				//qcom_watchdog(0,0);
				//userXmodemTransport();
				//qcom_watchdog(1,15);
				//gwStateForRealTime.gwDevIsOta = 0;
				//checkFlashDevFirmwareTimes = 36000;
			}
			else
			{
				FirstPowerOnZBOTAFlag = 0;

				printf("clear zigbee ota bin\r\n");
				memset((char *)&otaStationInfo, 0xff, sizeof(otaStationInfo));
				otaStationInfo.head = 0xaa;
				otaStationInfo.end = 0x55;
				//qca_save_flash_params(START_ADDR_FLASH_SUB_FIRMWARE_INFO_ADDR,(char*)&otaStationInfo,sizeof(otaStationInfo));
				//for(i = 0; i < MAX_SUB_FIRMWARE_PAGE; i ++)
				{
					//qca_save_flash_params(START_ADDR_FLASH_SUB_FIRMWARE_ADDR + i * 4096,(char*)&data,1);
				}
			}
		}
	}
	else
	{
		if (complete == 1 && zcXmodemStation != XUSER_RESTART_APP)
		{

			otaContrlInfo.ota_type = 2; //wifi �̼�������ʼ
			printf("set ---------otaContrlInfo.ota_type = 2  0003\r\n");
			otaContrlInfo.ota_num = 0;
			otaContrlInfo.ota_status = 1; // zigbee fail
			printf("ota test otaContrlInfo.ota_status -----------01\r\n");
			gwStateForRealTime.wifiSoftUpdate.appAllowSoftWareUpdate = 1;
		}
	}
	return;
}
