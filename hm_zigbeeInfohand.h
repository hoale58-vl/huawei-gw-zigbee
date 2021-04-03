#ifndef _ZIGBEE_INFO_H
#define _ZIGBEE_INFO_H

//device id

#define DEVICE_ID_ON_OFF_SWITCH 0x0000
#define DEVICE_ID_LEVEL_CONTROL_SWITCH 0x0001
#define DEVICE_ID_ON_OFF_OUTPUT 0x0002
#define DEVICE_ID_LEVEL_CONTROL_OUTPUT 0x0003
#define DEVICE_ID_SCENE_SELECTOR 0x0004
#define DEVICE_ID_CONFIG_TOOL 0x0005
#define DEVICE_ID_REMOTE_CONTROL 0x0006
#define DEVICE_ID_COMBINED_INTERFACE 0x0007
#define DEVICE_ID_RANGE_EXTENDER 0x0008
#define DEVICE_ID_MAINS_POWER_OUTLET 0x0009
#define DEVICE_ID_DOOR_LOCK 0x000a
#define DEVICE_ID_DOOR_LOCK_CONTROLLER 0x000b
#define DEVICE_ID_SIMPLE_SENSOR 0x000c
#define DEVICE_ID_CONSUMPTION_AWARENESS_DEVICE 0x000d
#define DEVICE_ID_HOME_GATEWAY 0x0050
#define DEVICE_ID_SMART_PLUG 0x0051
#define DEVICE_ID_WHITE_GOODS 0x0052
#define DEVICE_ID_METER_INTERFACE 0x0053

#define DEVICE_ID_ON_OFF_LIGHT 0x0100
#define DEVICE_ID_DIMMABLE_LIGHT 0x0101
#define DEVICE_ID_COLOR_DIMMABLE_LIGHT 0x0102
#define DEVICE_ID_ON_OFF_LIGHT_SWITCH 0x0103
#define DEVICE_ID_DIMMER_SWITCH 0x0104
#define DEVICE_ID_COLOR_DIMMER_SWITCH 0x0105
#define DEVICE_ID_LIGHT_SENSOR 0x0106
#define DEVICE_ID_OCCUPANCY_SENSOR 0x0107

#define DEVICE_ID_SHADE 0x0200
#define DEVICE_ID_SHADE_CONTROLLER 0x0201
#define DEVICE_ID_WINDOW_COVERING_DEVICE 0x0202
#define DEVICE_ID_WINDOW_COVERING_CONTROLLER 0x0203

#define DEVICE_ID_HEATING_COOLING_UNIT 0x0300
#define DEVICE_ID_THERMOSTAT 0x0301
#define DEVICE_ID_TEMPERATURE_SENSOR 0x0302
#define DEVICE_ID_PUMP 0x0303
#define DEVICE_ID_PUMP_CONTROLLER 0x0304
#define DEVICE_ID_PRESSURE_SENSOR 0x0305
#define DEVICE_ID_FLOW_SENSOR 0x0306
#define DEVICE_ID_MINI_SPLIT_AC 0x0307
#define DEVICE_ID_AIRQUALITY_SENSOR 0x03e0


#define DEVICE_ID_IAS_CIE 0x0400
#define DEVICE_ID_IAS_ANCILLARY_CONTROL 0x0401
#define DEVICE_ID_IAS_ZONE 0x0402
#define DEVICE_ID_IAS_WARNING 0x0403

#define DEVICE_ID_USER_LDX_RGBW		0x0210
#define DEVICE_ID_USER_OTHER_RGBW		0x010d

#define DEVICE_ID_USER_OTHER_COLORTEMPERATUR	0x010c//hardy add


//cluser id
#define ZCL_BASIC_CLUSTER_ID                         0x0000
#define ZCL_POWER_CONFIG_CLUSTER_ID                  0x0001
#define ZCL_DEVICE_TEMP_CLUSTER_ID                   0x0002
#define ZCL_IDENTIFY_CLUSTER_ID                      0x0003
#define ZCL_GROUPS_CLUSTER_ID                        0x0004
#define ZCL_SCENES_CLUSTER_ID                        0x0005
#define ZCL_ON_OFF_CLUSTER_ID                        0x0006
#define ZCL_ON_OFF_SWITCH_CONFIG_CLUSTER_ID          0x0007
#define ZCL_LEVEL_CONTROL_CLUSTER_ID                 0x0008
#define ZCL_ALARM_CLUSTER_ID                         0x0009
#define ZCL_TIME_CLUSTER_ID                          0x000A
#define ZCL_RSSI_LOCATION_CLUSTER_ID                 0x000B
#define ZCL_BINARY_INPUT_BASIC_CLUSTER_ID            0x000F
#define ZCL_COMMISSIONING_CLUSTER_ID                 0x0015
#define ZCL_PARTITION_CLUSTER_ID                     0x0016
#define ZCL_OTA_BOOTLOAD_CLUSTER_ID                  0x0019
#define ZCL_POWER_PROFILE_CLUSTER_ID                 0x001A
#define ZCL_APPLIANCE_CONTROL_CLUSTER_ID             0x001B
#define ZCL_POLL_CONTROL_CLUSTER_ID                  0x0020
#define ZCL_GREEN_POWER_CLUSTER_ID                   0x0021
#define ZCL_KEEPALIVE_CLUSTER_ID                     0x0025
#define ZCL_SHADE_CONFIG_CLUSTER_ID                  0x0100
#define ZCL_DOOR_LOCK_CLUSTER_ID                     0x0101
#define ZCL_WINDOW_COVERING_CLUSTER_ID               0x0102
#define ZCL_PUMP_CONFIG_CONTROL_CLUSTER_ID           0x0200
#define ZCL_THERMOSTAT_CLUSTER_ID                    0x0201
#define ZCL_FAN_CONTROL_CLUSTER_ID                   0x0202
#define ZCL_DEHUMID_CONTROL_CLUSTER_ID               0x0203
#define ZCL_THERMOSTAT_UI_CONFIG_CLUSTER_ID          0x0204
#define ZCL_COLOR_CONTROL_CLUSTER_ID                 0x0300
#define ZCL_BALLAST_CONFIGURATION_CLUSTER_ID         0x0301
#define ZCL_ILLUM_MEASUREMENT_CLUSTER_ID             0x0400
#define ZCL_ILLUM_LEVEL_SENSING_CLUSTER_ID           0x0401
#define ZCL_TEMP_MEASUREMENT_CLUSTER_ID              0x0402
#define ZCL_AIR_PM2_MEASUREMENT_CLUSTER_ID           0x042A
#define ZCL_AIR_CH2O_MEASUREMENT_CLUSTER_ID          0x042B
#define ZCL_AIR_INFO_MEASUREMENT_CLUSTER_ID          0xFC81


#define ZCL_PRESSURE_MEASUREMENT_CLUSTER_ID          0x0403
#define ZCL_FLOW_MEASUREMENT_CLUSTER_ID              0x0404
#define ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID 0x0405
#define ZCL_OCCUPANCY_SENSING_CLUSTER_ID             0x0406
#define ZCL_IAS_ZONE_CLUSTER_ID                      0x0500
#define ZCL_IAS_ACE_CLUSTER_ID                       0x0501
#define ZCL_IAS_WD_CLUSTER_ID                        0x0502
#define ZCL_GENERIC_TUNNEL_CLUSTER_ID                0x0600
#define ZCL_BACNET_PROTOCOL_TUNNEL_CLUSTER_ID        0x0601
#define ZCL_11073_PROTOCOL_TUNNEL_CLUSTER_ID         0x0614
#define ZCL_ISO7816_PROTOCOL_TUNNEL_CLUSTER_ID       0x0615
#define ZCL_PRICE_CLUSTER_ID                         0x0700
#define ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID  0x0701
#define ZCL_SIMPLE_METERING_CLUSTER_ID               0x0702
#define ZCL_MESSAGING_CLUSTER_ID                     0x0703
#define ZCL_TUNNELING_CLUSTER_ID                     0x0704
#define ZCL_PREPAYMENT_CLUSTER_ID                    0x0705
#define ZCL_ENERGY_MANAGEMENT_CLUSTER_ID             0x0706
#define ZCL_CALENDAR_CLUSTER_ID                      0x0707
#define ZCL_DEVICE_MANAGEMENT_CLUSTER_ID             0x0708
#define ZCL_EVENTS_CLUSTER_ID                        0x0709
#define ZCL_MDU_PAIRING_CLUSTER_ID                   0x070A
#define ZCL_KEY_ESTABLISHMENT_CLUSTER_ID             0x0800
#define ZCL_INFORMATION_CLUSTER_ID                   0x0900
#define ZCL_DATA_SHARING_CLUSTER_ID                  0x0901
#define ZCL_GAMING_CLUSTER_ID                        0x0902
#define ZCL_DATA_RATE_CONTROL_CLUSTER_ID             0x0903
#define ZCL_VOICE_OVER_ZIGBEE_CLUSTER_ID             0x0904
#define ZCL_CHATTING_CLUSTER_ID                      0x0905
#define ZCL_PAYMENT_CLUSTER_ID                       0x0A01
#define ZCL_BILLING_CLUSTER_ID                       0x0A02
#define ZCL_APPLIANCE_IDENTIFICATION_CLUSTER_ID      0x0B00
#define ZCL_METER_IDENTIFICATION_CLUSTER_ID          0x0B01
#define ZCL_APPLIANCE_EVENTS_AND_ALERT_CLUSTER_ID    0x0B02
#define ZCL_APPLIANCE_STATISTICS_CLUSTER_ID          0x0B03
#define ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID        0x0B04
#define ZCL_DIAGNOSTICS_CLUSTER_ID                   0x0B05
#define ZCL_ZLL_COMMISSIONING_CLUSTER_ID             0x1000
#define ZCL_SAMPLE_MFG_SPECIFIC_CLUSTER_ID           0xFC00
#define ZCL_OTA_CONFIGURATION_CLUSTER_ID             0xFC01
#define ZCL_MFGLIB_CLUSTER_ID                        0xFC02
#define ZCL_RGBLIGHT_CLUSTER_ID                      0xFC08




//attribute id
// Attribute types for cluster: Simple Metering
#define ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID            0x0000 // Ver.: always
#define ZCL_INSTANTANEOUS_DEMAND_ATTRIBUTE_ID                   0x0400 // Ver.: always

// Attribute types for cluster: Electrical Measurement
#define ZCL_RMS_VOLTAGE_ATTRIBUTE_ID                            0x0505 // Ver.: always

// Attribute types for cluster: Temperature Measurement
#define ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID                    0x0000 // Ver.: always
// Attribute types for cluster: Relative Humidity Measurement
#define ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID       0x0000 // Ver.: always
// Attribute types for cluster: Power Configuration
#define ZCL_BATTERY_PERCENTAGE_REMAINING_ATTRIBUTE_ID          	0x0021 // Ver.: since ha-1.2-05-3520-29
#define ZCL_BATTERY_ALARM_STATE_ATTRIBUTE_ID                   	0x003E
// Attribute types for cluster: On/off
#define ZCL_ON_OFF_ATTRIBUTE_ID                              	0x0000 // Ver.: always

// Attribute types for cluster: Color Control
#define ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID                0x0003 // Ver.: always
#define ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID                0x0004 // Ver.: always

#define ZCL_COLOR_CONTROL_CURRENT_XY_ATTRIBUTE_ID               0x0001 // Ver.: always hardy add
#define ZCL_COLOR_CONTROL_CURRENT_WY_ATTRIBUTE_ID               0x0007 // Ver.: always hardy add



// Attribute types for cluster: Level Control// Cluster specification level: zcl-6.0-15-02017-001// Server attributes
#define ZCL_CURRENT_LEVEL_ATTRIBUTE_ID                          0x0000 // Ver.: always

#define ZCL_CURRENT_TIME_ATTRIBUTE_ID                          	0x0000 // Ver.: always
#define ZCL_CURRENT_TIMEZONE_ATTRIBUTE_ID                      	0x0002 // Ver.: always

#define ZCL_AIR_PM2_MEASURED_VALUE_ATTRIBUTE_ID      			0x0000 // Ver.: always
#define ZCL_AIR_CH2O_MEASURED_VALUE_ATTRIBUTE_ID      			0x0000 // Ver.: always

#define ZCL_AIR_LG_VALUE_ATTRIBUTE_ID      						0xF000 // Ver.: always
#define ZCL_AIR_TUI_VALUE_ATTRIBUTE_ID      					0xF001 // Ver.: always

#define ZCL_AIR_ACST_MEASURED_VALUE_ATTRIBUTE_ID      			0xF002 // Ver.: always
#define ZCL_AIR_PM10_MEASURED_VALUE_ATTRIBUTE_ID      			0xF003 // Ver.: always
#define ZCL_AIR_AQ_MEASURED_VALUE_ATTRIBUTE_ID      			0xF005 // Ver.: always
#define ZCL_AIR_ALARM_ENABLE_VALUE_ATTRIBUTE_ID      			0xF00A // Ver.: always


#define ZCL_BRIGHTNESS_ATTRIBUTE_ID        						0x0000 // Ver.: always
#define ZCL_COLOR_TEMPERATURE_ATTRIBUTE_ID 						0x0001 // Ver.: always

#define ZCL_CURRENT_ZONE_SENSITIVITY_LEVEL_ATTRIBUTE_ID         0x0013 // Ver.: always


//basic cluster 
#define ZCL_DEVICE_ENABLED_ATTRIBUTE_ID                 		0x0012 // Ver.: always

// Attribute types for cluster: Shade Configuration
// Cluster specification level: zcl-6.0-15-02018-001
// Server attributes
#define ZCL_SHADE_CONFIG_STATUS_ATTRIBUTE_ID                   0x0002 // Ver.: always
#define ZCL_SHADE_CONFIG_CLOSED_LIMIT_ATTRIBUTE_ID             0x0010 // Ver.: always





// ZCL attribute types
enum {
  ZCL_NO_DATA_ATTRIBUTE_TYPE                        = 0x00, // No data
  ZCL_DATA8_ATTRIBUTE_TYPE                          = 0x08, // 8-bit data
  ZCL_DATA16_ATTRIBUTE_TYPE                         = 0x09, // 16-bit data
  ZCL_DATA24_ATTRIBUTE_TYPE                         = 0x0A, // 24-bit data
  ZCL_DATA32_ATTRIBUTE_TYPE                         = 0x0B, // 32-bit data
  ZCL_DATA40_ATTRIBUTE_TYPE                         = 0x0C, // 40-bit data
  ZCL_DATA48_ATTRIBUTE_TYPE                         = 0x0D, // 48-bit data
  ZCL_DATA56_ATTRIBUTE_TYPE                         = 0x0E, // 56-bit data
  ZCL_DATA64_ATTRIBUTE_TYPE                         = 0x0F, // 64-bit data
  ZCL_BOOLEAN_ATTRIBUTE_TYPE                        = 0x10, // Boolean
  ZCL_BITMAP8_ATTRIBUTE_TYPE                        = 0x18, // 8-bit bitmap
  ZCL_BITMAP16_ATTRIBUTE_TYPE                       = 0x19, // 16-bit bitmap
  ZCL_BITMAP24_ATTRIBUTE_TYPE                       = 0x1A, // 24-bit bitmap
  ZCL_BITMAP32_ATTRIBUTE_TYPE                       = 0x1B, // 32-bit bitmap
  ZCL_BITMAP40_ATTRIBUTE_TYPE                       = 0x1C, // 40-bit bitmap
  ZCL_BITMAP48_ATTRIBUTE_TYPE                       = 0x1D, // 48-bit bitmap
  ZCL_BITMAP56_ATTRIBUTE_TYPE                       = 0x1E, // 56-bit bitmap
  ZCL_BITMAP64_ATTRIBUTE_TYPE                       = 0x1F, // 64-bit bitmap
  ZCL_INT8U_ATTRIBUTE_TYPE                          = 0x20, // Unsigned 8-bit integer
  ZCL_INT16U_ATTRIBUTE_TYPE                         = 0x21, // Unsigned 16-bit integer
  ZCL_INT24U_ATTRIBUTE_TYPE                         = 0x22, // Unsigned 24-bit integer
  ZCL_INT32U_ATTRIBUTE_TYPE                         = 0x23, // Unsigned 32-bit integer
  ZCL_INT40U_ATTRIBUTE_TYPE                         = 0x24, // Unsigned 40-bit integer
  ZCL_INT48U_ATTRIBUTE_TYPE                         = 0x25, // Unsigned 48-bit integer
  ZCL_INT56U_ATTRIBUTE_TYPE                         = 0x26, // Unsigned 56-bit integer
  ZCL_INT64U_ATTRIBUTE_TYPE                         = 0x27, // Unsigned 64-bit integer
  ZCL_INT8S_ATTRIBUTE_TYPE                          = 0x28, // Signed 8-bit integer
  ZCL_INT16S_ATTRIBUTE_TYPE                         = 0x29, // Signed 16-bit integer
  ZCL_INT24S_ATTRIBUTE_TYPE                         = 0x2A, // Signed 24-bit integer
  ZCL_INT32S_ATTRIBUTE_TYPE                         = 0x2B, // Signed 32-bit integer
  ZCL_INT40S_ATTRIBUTE_TYPE                         = 0x2C, // Signed 40-bit integer
  ZCL_INT48S_ATTRIBUTE_TYPE                         = 0x2D, // Signed 48-bit integer
  ZCL_INT56S_ATTRIBUTE_TYPE                         = 0x2E, // Signed 56-bit integer
  ZCL_INT64S_ATTRIBUTE_TYPE                         = 0x2F, // Signed 64-bit integer
  ZCL_ENUM8_ATTRIBUTE_TYPE                          = 0x30, // 8-bit enumeration
  ZCL_ENUM16_ATTRIBUTE_TYPE                         = 0x31, // 16-bit enumeration
  ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE                     = 0x38, // Semi-precision
  ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE                   = 0x39, // Single precision
  ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE                   = 0x3A, // Double precision
  ZCL_OCTET_STRING_ATTRIBUTE_TYPE                   = 0x41, // Octet string
  ZCL_CHAR_STRING_ATTRIBUTE_TYPE                    = 0x42, // Character string
  ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE              = 0x43, // Long octet string
  ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE               = 0x44, // Long character string
  ZCL_ARRAY_ATTRIBUTE_TYPE                          = 0x48, // Array
  ZCL_STRUCT_ATTRIBUTE_TYPE                         = 0x4C, // Structure
  ZCL_SET_ATTRIBUTE_TYPE                            = 0x50, // Set
  ZCL_BAG_ATTRIBUTE_TYPE                            = 0x51, // Bag
  ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE                    = 0xE0, // Time of day
  ZCL_DATE_ATTRIBUTE_TYPE                           = 0xE1, // Date
  ZCL_UTC_TIME_ATTRIBUTE_TYPE                       = 0xE2, // UTC Time
  ZCL_CLUSTER_ID_ATTRIBUTE_TYPE                     = 0xE8, // Cluster ID
  ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE                   = 0xE9, // Attribute ID
  ZCL_BACNET_OID_ATTRIBUTE_TYPE                     = 0xEA, // BACnet OID
  ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE                   = 0xF0, // IEEE address
  ZCL_SECURITY_KEY_ATTRIBUTE_TYPE                   = 0xF1, // 128-bit security key
  ZCL_UNKNOWN_ATTRIBUTE_TYPE                        = 0xFF // Unknown

};


//level sub cmd

enum{
MOVE_TO_LEVEL = 0,
MOVE,
STEP,
STOP,
MOVE_TO_LEVEL_WTONOFF,
MOVE_WTONOFF,
STEP_WTONOFF,
STOP_WTONOFF,
};

//color sub cmd
enum{
MOVE_TO_COLOR = 0x07,
MOVE_COLOR = 0x08,
STEP_COLOR = 0x09,
MOVE_COLOR_TEMPERATURE = 0x0A,
STOP_MOVE_STEP = 0x47
};

//wd sub cmd
enum{
START_WARNING,
SQUAWK
};
//ace com
enum{
ARMMODE = 0,
BYPASS = 1,
EMERGENCY = 2,
FIRE = 3,
PANIC = 4,
GETZONEMAP = 5,
GETZONEINFO = 6
};

//arm mode
enum{
disarm = 0,
armDay = 1,
armNight = 2,
arm = 3
};

//ias zone type
typedef enum {
  EMBER_ZCL_IAS_ZONE_TYPE_STANDARD_CIE              = 0x0000,
  EMBER_ZCL_IAS_ZONE_TYPE_MOTION_SENSOR             = 0x000D,
  EMBER_ZCL_IAS_ZONE_TYPE_CONTACT_SWITCH            = 0x0015,
  EMBER_ZCL_IAS_ZONE_TYPE_FIRE_SENSOR               = 0x0028,
  EMBER_ZCL_IAS_ZONE_TYPE_WATER_SENSOR              = 0x002A,
  EMBER_ZCL_IAS_ZONE_TYPE_GAS_SENSOR                = 0x002B,
  EMBER_ZCL_IAS_ZONE_TYPE_PERSONAL_EMERGENCY_DEVICE = 0x002C,
  EMBER_ZCL_IAS_ZONE_TYPE_VIBRATION_MOVEMENT_SENSOR = 0x002D,
  EMBER_ZCL_IAS_ZONE_TYPE_REMOTE_CONTROL            = 0x010F,
  EMBER_ZCL_IAS_ZONE_TYPE_KEY_FOB                   = 0x0115,
  EMBER_ZCL_IAS_ZONE_TYPE_KEYPAD                    = 0x021D,
  EMBER_ZCL_IAS_ZONE_TYPE_STANDARD_WARNING_DEVICE   = 0x0225,
  EMBER_ZCL_IAS_ZONE_TYPE_GLASS_BREAK_SENSOR        = 0x0226,
  EMBER_ZCL_IAS_ZONE_TYPE_CARBON_MONOXIDE_SENSOR    = 0x0227,
  EMBER_ZCL_IAS_ZONE_TYPE_SECURITY_REPEATER         = 0x0229,
  EMBER_ZCL_IAS_ZONE_TYPE_DOORBELL_SENSOR         	= 0x8001,
  EMBER_ZCL_IAS_ZONE_TYPE_INVALID_ZONE_TYPE         = 0xFFFF,
} EmberAfIasZoneType;


//for door lock define
#define DOOR_LOCK_CMD_ID_OPERATION_NOTIFICATION		0x20
typedef enum{
DOOR_LOCK_LOCKED					= 0x01,
DOOR_LOCK_UNLOCK					= 0x02,
DOOR_LOCK_TEMPER_ALARM				= 0x04,
DOOR_LOCK_NO_LOCK_ALARM				= 0x05,
DOOR_LOCK_FORCE_UNLOCK_ALARM		= 0x06,
DOOR_LOCK_FALSE_LOCK_ALARM			= 0x07,
DOOR_LOCK_ILLEGAL_OPERATION_ALARM   = 0x33
}DOORLOCKSTATUSENUM;

typedef enum{
DOOR_STATE_NOT_FULLY_LOCKED			= 0x00,
DOOR_STATE_LOCKED					= 0x01,
DOOR_STATE_UNLOCKED					= 0x02,
DOOR_STATE_UNDEFINED				= 0x03
}DOORSTATE;

//default dev name
#define NODE_INFO_TYPE_UNKNOWN_STRING				"Unknow_"
#define NODE_INFO_TYPE_LIGHT_RGB_STRING				"RGB_"//RGB
#define NODE_INFO_TYPE_LIGHT_COLORTEMPERATUR_STRING	"ColorTemperatur_"//WY
#define NODE_INFO_TYPE_AI1SW_STRING 				"Slight1_"	// 1·������𿪹�
#define NODE_INFO_TYPE_AI2SW_STRING					"Slight2_"	// 2·������𿪹�
#define NODE_INFO_TYPE_AI3SW_STRING					"Slight3_"	// 3·������𿪹�
#define NODE_INFO_TYPE_ZB_IR_TRANSPONDER_STRING		"IrTrans_"	// ����ת��
#define NODE_INFO_TYPE_RELAY_CONTROL_STRING			"Rlctrol_"	// �̵�������

#define NODE_INFO_TYPE_CUSTOM_RGB_AREA_STRING		"Cutrgb4_"	//�ͻ����������ƿ�ϵͳ


#define NODE_INFO_TYPE_MAGNET_DOOR_STRING			"WDoor_"	// �Ŵ�

#define NODE_INFO_TYPE_WATER_SENSOR_STRING 			"Water_"	//ˮ��
#define NODE_INFO_TYPE_PIR_SENSOR_STRING			"Pir_"	// �������
#define NODE_INFO_TYPE_SMOKE_SENSOR_STRING			"Smoke_"	//����̽����
#define NODE_INFO_TYPE_TEMP_HUM_SENSOR_STRING		"THP_"	//��ʪ��
#define NODE_INFO_TYPE_GAS_SENSOR_STRING	        "Gas_"	//��ȼ���崫����	
#define NODE_INFO_TYPE_WARNING_DEV_STRING			"Siren_"	//���⾯��
#define NODE_INFO_TYPE_CO_SENSOR_STRING				"CO_"	//CO ������
#define NODE_INFO_TYPE_ILLUMINATION_STRING 			"Illumin_"	//���նȴ�����
#define NODE_INFO_TYPE_AIR_QUALITY_STRING			"Airaction_"	//��������������
#define NODE_INFO_TYPE_THERMOSTAT_STRING 			"Thermost_"	//�¿���
#define NODE_INFO_TYPE_VIBRATION_STRING				"Vibration_"	//�𶯴�����

#define NODE_INFO_TYPE_SOS_STRING					"Eb_"	//������ť
#define NODE_INFO_TYPE_REMOTE_CONTROL_STRING		"Rc_"	//����ң����
#define NODE_INFO_TYPE_AI_SOCKET_USB_STRING		    "Uplug_"	//USB���ܲ���
#define NODE_INFO_TYPE_AI_SOCKET_MEASURE_STRING		"SPlug_"	//�������ܲ���	
#define NODE_INFO_TYPE_AI_CURTAIN_MOTOR_STRING		"CurtainMo_"//�������
#define NODE_INFO_TYPE_AI_DOOR_LOCK_STRING			"Dlock_"	//��������

#define NODE_INFO_TYPE_AI_WALLSOCKET_MEASURE_STRING		"WPlug_"	//ǽ�����	
#define NODE_INFO_TYPE_SCENE_SWITCH_STRING				"Scene_"	//�龰����
#define NODE_INFO_TYPE_WIRED_SWITCH_STRING				"Wired_"	//���߿���
#define NODE_INFO_TYPE_DOORBELL_STRING					"Doorbell_"	//����
#define NODE_INFO_TYPE_DIMMING_SWITCH_STRING			"Dimming_"	//���⿪��
#define NODE_INFO_TYPE_TEMPDIMMING_SWITCH_STRING		"TempDimming_"	//���⿪��



//dev type
#define NODE_INFO_TYPE_UNKNOWN					00
#define NODE_INFO_TYPE_LIGHT_RGB				01//RGB
#define NODE_INFO_TYPE_AI1SW 					02	// 1·������𿪹�
#define NODE_INFO_TYPE_AI2SW					03	// 2·������𿪹�
#define NODE_INFO_TYPE_AI3SW					04	// 3·������𿪹�
#define NODE_INFO_TYPE_ZB_IR_TRANSPONDER		05	// ����ת��
#define NODE_INFO_TYPE_RELAY_CONTROL			06	// �̵�������

#define NODE_INFO_TYPE_CUSTOM_RGB_AREA			10	//�ͻ����������ƿ�ϵͳ


#define NODE_INFO_TYPE_MAGNET_DOOR				17	// �Ŵ�

#define NODE_INFO_TYPE_WATER_SENSOR 			18	//ˮ��
#define NODE_INFO_TYPE_PIR_SENSOR				19	// �������
#define NODE_INFO_TYPE_SMOKE_SENSOR				20	//����̽����
#define NODE_INFO_TYPE_TEMP_HUM_SENSOR			21	//��ʪ��
#define NODE_INFO_TYPE_GAS_SENSOR	        	22	//��ȼ���崫����	
#define NODE_INFO_TYPE_WARNING_DEV				23	//���⾯��
#define NODE_INFO_TYPE_CO_SENSOR				24	//CO ������
#define NODE_INFO_TYPE_ILLUMINATION 			25	//���նȴ�����
#define NODE_INFO_TYPE_AIR_QUALITY				26	//��������������
#define NODE_INFO_TYPE_THERMOSTAT 				27	//�¿���
#define NODE_INFO_TYPE_VIBRATION				28	//�𶯴�����

#define NODE_INFO_TYPE_SOS						49	//������ť
#define NODE_INFO_TYPE_REMOTE_CONTROL			50	//����ң����
#define NODE_INFO_TYPE_AI_SOCKET_USB		    67	//USB���ܲ���
#define NODE_INFO_TYPE_AI_SOCKET_MEASURE		68	//�������ܲ���
#define NODE_INFO_TYPE_AI_CURTAIN_MOTOR			69	//���ܴ������
#define NODE_INFO_TYPE_AI_DOOR_LOCK				70	//��������

#define NODE_INFO_TYPE_AI_WALLSOCKET_MEASURE	71//����ǽ�����

#define NODE_INFO_TYPE_LIGHT_WY					11//WY ɫ�µ�
#define NODE_INFO_TYPE_LIGHT_RGBW				12//RGBW

#define NODE_INFO_TYPE_SCENE_SWITCH				9	//�龰���أ��ǹ̶�
#define NODE_INFO_TYPE_FSCENE_SWITCH			15	//�龰���أ��̶�
#define NODE_INFO_TYPE_WIRED_SWITCH				13	//���߿���
#define NODE_INFO_TYPE_DOORBELL					14	//����
#define NODE_INFO_TYPE_DIMMING_SWITCH  			16	//���⿪��
#define NODE_INFO_TYPE_TEMPDIMMING_SWITCH  		30	//ɫ�¿���
#define NODE_INFO_TYPE_CURTAINSWITCH_SWITCH  	31	//�������ƿ��� 




#endif
