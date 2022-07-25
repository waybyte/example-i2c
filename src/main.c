/**
 * @file main.c
 * @brief Exmaple application to demonstrate I2C API
 * @version 0.1
 * @date 2022-07-05
 * 
 * @copyright Copyright (c) 2022 Waybyte Solutions
 * 
 */

#include <stdio.h>
#include <unistd.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>
#include <hw/i2c.h>

#ifdef SOC_RDA8910
#define STDIO_PORT "/dev/ttyUSB0"
#else
#define STDIO_PORT "/dev/ttyS0"
#endif

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	int ret;
	uint8_t wrbuf[16];
	uint8_t rdbuf[16];
	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init(STDIO_PORT, urc_callback);

	printf("System Ready\n");

	sleep(5);
	/* I2C Example */
	do {
		ret = i2c_hw_init(I2C_PORT_0, 100);
		printf("\nI2C init: %d\n", ret);
		if (ret)
			break;
		
		printf("Scanning for I2C Devices\n     ");
		for (int i = 0; i < 16; i++)
			printf("%x ", i);

		/* Scan for i2c devices */
		for (int i = 0; i < 256; i++) {
			if ((i % 16) == 0)
				printf("\n0x%02x ", i);
			
			if (i == 0) {
				printf("- ");
				continue;
			}

			wrbuf[0] = 0x00;
			ret = i2c_hw_write(I2C_PORT_0, i, wrbuf, 1);
			if (ret != 1)
				printf("- "); /* not found */
			else
				printf("* "); /* found */
		}
		printf("\n\n");

		/* Access EEPROM - Address 0x50 */
		/* Read from 0x0080 address */
		wrbuf[0] = 0x00;
		wrbuf[1] = 0x80;
		ret = i2c_hw_write(I2C_PORT_0, 0x50, wrbuf, 2);
		printf("I2C Write: %d\n", ret);
		if (ret != 2)
			break;
		
		ret = i2c_hw_read(I2C_PORT_0, 0x50, rdbuf, sizeof(rdbuf));
		printf("I2C read: %d\n", ret);
		if (ret != sizeof(rdbuf))
			break;
		
		printf("Data @ 0x0080: ");
		for(int i = 0; i < ret; i++)
			printf("%02X ", rdbuf[i]);
		printf("\n");
	} while (0);

	while (1) {
		/* Main task */
		sleep(1);
	}
}
