/*
 * loraif.h
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#ifndef COMPONENTS_LORAIF_H_
#define COMPONENTS_LORAIF_H_

#include "component_config.h"
#if ENABLE_COMPONENT_LORAIF

#include "sdkconfig.h"
#include st_header
#include "stdio.h"
#include "list"
#include "algorithm"

#include "parse_packet/parse_packet.h"

#include "sx127x/sx127x.h"

using namespace std;

#define LORAIF_QUEUE_SIZE 30
typedef enum {
	LORA_ERR,

	/**
	 * Gateway passive.
	 */
	LORA_REQ_ADDRESS, // No address.
	LORA_UPDATE_ADDRESS,
	LORA_UPDATE_STATE,

	/**
	 * Gateway proactive.
	 */
	LORA_UPDATE_SETTINGS,
	LORA_REQ_DATA,
	LORA_RES_DATA,
	LORA_UPDATE_DATA,

	LORA_ADD_DEVICE,
	LORA_REMOVE_DEVICE,
	LORA_DEVICE_NOT_RESPONSE,

	LORA_CMD_NUM,
} lora_event_t;


typedef struct{
	uint32_t address;
	char *name;
	uint8_t err_count;
	void *data;
} loraif_dev_t;


#ifdef __cplusplus
extern "C"{
#endif

void loraif_init(sx127x *lora, uint8_t send_syncword, uint8_t recv_syncword, uint32_t timeout, uint8_t max_not_response);
void loraif_register_event_handler(void (*peventhandler)(lora_event_t event, char *data));

bool loraif_check_crc(char *data);

void loraif_request(uint32_t dev_address, lora_event_t cmd, char *data, int require_resp);
void loraif_request_data(void);
void loraif_rx_process(void *param);
void loraif_response(void);
void loraif_check_timeout(void);


void loraif_add_device(char *jdata, void *dev_data);
void loraif_remove_device(char *jdata);
loraif_dev_t *loraif_select_device(char *jdata);


#ifdef __cplusplus
}
#endif

#endif /* ENABLE_COMPONENT_WIFIIF */

#endif /* COMPONENTS_LORAIF_H_ */
