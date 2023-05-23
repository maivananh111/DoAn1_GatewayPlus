/*
 * loraif.cpp
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#include "component_config.h"
#if ENABLE_COMPONENT_LORAIF

#include "loraif/loraif.h"
#include "parse_packet/parse_packet.h"
#include "crc/crc.h"

#include "stdlib.h"
#include "string.h"

#include "system/log.h"
#include "system/system.h"
#include "periph/systick.h"
#include "periph/rng.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

using namespace std;


#if ENABLE_COMPONENT_LORAIF_DEBUG
static const char *TAG = "LoRaIF";
#define LOG_LEVEL LOG_DEBUG
#endif /* ENABLE_COMPONENT_LORAIF_DEBUG */
static const char *command_string[] = {
	"LORA_ERR",

	"LORA_REQ_ADDRESS",
	"LORA_UPDATE_ADDRESS",
	"LORA_UPDATE_STATE",

	"LORA_UPDATE_SETTINGS",
	"LORA_REQ_DATA",
	"LORA_RES_DATA",
	"LORA_UPDATE_DATA",

	"LORA_DEL_DEVICE",

	"LORA_CMD_NUM",
};

sx127x *loraif;
static void (*fpeventhandler)(lora_event_t event, char *data);

typedef struct {
	uint32_t address;
	lora_event_t cmd;
	uint32_t tick_start = 0;
} loraif_request_prop_t;

uint32_t addr_by_gw = 0x00;
uint32_t resp_timeout;
uint8_t max_not_resp;

QueueHandle_t wait_response_queue, response_queue;
SemaphoreHandle_t tranfer_smp;
list<loraif_dev_t *> loraif_device_list;

static void loraif_transmit(char *str);
static void loraif_debug(char *str, int line, const char *func);
static void show_device_list(void);
static bool device_is_available(uint32_t num);


static void loraif_debug(char *str, int line, const char *func){
#if ENABLE_COMPONENT_LORAIF_DEBUG
	LOG_LEVEL(TAG, "%s, Line: %d Function: %s", str, line, func);
#endif /* ENABLE_COMPONENT_LORAIF_DEBUG */
}

static void show_device_list(void){
    for (auto device = loraif_device_list.begin(); device != loraif_device_list.end(); ++device) {
    	char *tmp;
    	asprintf(&tmp, "Device 0x%08x name \"%s\".", (unsigned int)(*device)->address, (*device)->name);
    	LOG_MEM("LoRaIF Device properties", tmp);
    	free(tmp);
    }
}

static bool device_is_available(uint32_t num){
    for (auto device = loraif_device_list.begin(); device != loraif_device_list.end(); ++device) {
        if ((*device)->address == num) {
            return true;
        }
    }
    return false;
}


void loraif_init(sx127x *lora, uint32_t timeout, uint8_t max_not_response){
	loraif = lora;
	resp_timeout = timeout;
	max_not_resp = max_not_response;

	wait_response_queue = xQueueCreate(LORAIF_QUEUE_SIZE, sizeof(loraif_request_prop_t *));
	response_queue = xQueueCreate(LORAIF_QUEUE_SIZE, sizeof(char *));
	tranfer_smp = xSemaphoreCreateBinary();
	xSemaphoreGive(tranfer_smp);
}

void loraif_register_event_handler(void (*peventhandler)(lora_event_t event, char *data)){
	fpeventhandler = peventhandler;
}

bool loraif_check_crc(char *data){
	char *src_cpy = data;
	char *crc_start;
	uint16_t crc, icrc, len;
	pkt_t pkt;
	pkt_err_t err;
	pkt_json_t json;

	crc_start = strstr(src_cpy, "\"crc\"");
	if(crc_start == NULL) return false;

	err = parse_packet(src_cpy, &pkt);
	if(err != PKT_ERR_OK) return false;
	err = json_get_object(pkt.data_str, &json, (char *)"crc");
	if(err != PKT_ERR_OK) return false;
	icrc = strtol(json.value, NULL, 16);
	json_release_object(&json);
	release_packet(&pkt);

	len = (uint32_t)crc_start - (uint32_t)src_cpy;
	crc = cal_crc16((uint8_t *)src_cpy, len);
	if(crc != icrc) return false;

	return true;
}

static void loraif_transmit(char *str){
	if(xSemaphoreTake(tranfer_smp, portMAX_DELAY)){
		loraif->beginPacket();
		loraif->transmit((uint8_t *)str, (size_t)strlen(str));
		loraif->endPacket();
		loraif->Receive(0);
		xSemaphoreGive(tranfer_smp);
	}
}

static void set_response_ok(uint32_t addr, lora_event_t cmd){
	uint16_t crc = 0;
	char *temp;
	char *response_to_device;
	char *cmd_str = cmd_to_str(cmd, command_string);

	asprintf(&temp, "%s: {\"addr\":0x%08x,\"state\":OK,", cmd_str, (unsigned int)addr);
	crc = cal_crc16((uint8_t *)temp, strlen(temp));
	free(temp);

	asprintf(&response_to_device, "%s: {\"addr\":0x%08x,\"state\":OK,\"crc\":0x%04x}", cmd_str, (unsigned int)addr, crc);

	if(xQueueSend(response_queue, &response_to_device, 2) == pdFALSE){
		loraif_debug((char *)"Can't send to response_queue", __LINE__, __FUNCTION__);
	}
}

void loraif_request(uint32_t dev_address, lora_event_t cmd, char *data, int require_resp){
	uint16_t crc = 0;
	char *req_data, *temp;
	char *cmd_str = cmd_to_str(cmd, command_string);
	asprintf(&temp, "%s: {\"addr\":0x%08x,\"data\":%s,\"require_response\":%d,", cmd_str, (unsigned int)dev_address, data, require_resp);
	crc = cal_crc16((uint8_t *)temp, strlen(temp));
	free(temp);

	asprintf(&req_data, "%s: {\"addr\":0x%08x,\"data\":%s,\"require_response\":%d,\"crc\":0x%04x}", cmd_str, (unsigned int)dev_address, data, require_resp, crc);

	if(require_resp != 0){
		/** Create new require response profile */
		loraif_request_prop_t *wait_response = (loraif_request_prop_t *)malloc(sizeof(loraif_request_prop_t));
		wait_response->address = dev_address;
		wait_response->cmd = cmd;
		wait_response->tick_start = get_tick();
		/** Send wait_response to_queue */
		loraif_request_prop_t *in_queue;
		uint8_t queue_len = uxQueueMessagesWaiting(wait_response_queue);
/*		for(uint8_t i=0; i<queue_len; i++){
			if(xQueueReceive(wait_response_queue, &in_queue, 2) == pdTRUE){
				if(in_queue->address == wait_response->address && in_queue->cmd == wait_response->cmd){
					wait_response->tick_start = in_queue->tick_start;
					free(in_queue);
				}
				else{
					if(xQueueSend(wait_response_queue, &in_queue, 2) != pdTRUE){
						loraif_debug((char *)"Can't send to wait_response_queue", __LINE__, __FUNCTION__);
					}
				}
			}
		}

		if(xQueueSend(wait_response_queue, &wait_response, 2) != pdTRUE){
			loraif_debug((char *)"Can't send to wait_response_queue", __LINE__, __FUNCTION__);
		}
*/
		for(uint8_t i=0; i<queue_len; i++){
			if(xQueuePeek(wait_response_queue, &in_queue, 2) == pdTRUE){
				if(in_queue->address == wait_response->address && in_queue->cmd == wait_response->cmd){
					free(wait_response);
					wait_response = NULL;
				}
			}
		}
		if(wait_response != NULL){
			if(xQueueSend(wait_response_queue, &wait_response, 2) != pdTRUE){
				loraif_debug((char *)"Can't send to wait_response_queue", __LINE__, __FUNCTION__);
			}
		}
	}

	loraif_transmit(req_data);

	free(req_data);
}


void loraif_rx_process(void *param){
	QueueHandle_t *queue = (QueueHandle_t *)param;
	char *rx_full;

	if(xQueueReceive(*queue, &rx_full, 2)){
		pkt_err_t err;
		pkt_t pkt;
		pkt_json_t json;
		lora_event_t cmd;
		char *evt_data = NULL;
		char *response_to_device;

		err = parse_packet(rx_full, &pkt);
/** Parse packet success */
		if(err == PKT_ERR_OK){
			cmd = (lora_event_t)str_to_cmd(pkt.cmd_str, command_string, (int)LORA_CMD_NUM);

		/** Response without device address */
			if(cmd == LORA_REQ_ADDRESS){
				err = json_get_object(pkt.data_str, &json, (char *)"random_number");

				if(err == PKT_ERR_OK){
					/** Generate new address */
					uint32_t rand_num = strtol(json.value, NULL, 16);

					rng_set_seed(rand_num);
					while(1){
						addr_by_gw = rng_generate_random_number();
					    if(device_is_available(addr_by_gw)) continue;
					    else break;
					}
					addr_by_gw &= 0x7FFFFFFFU;
					/** Response new address to new device */
					uint16_t crc = 0;
					char *temp;

					asprintf(&temp, "LORA_RES_ADDRESS: {\"addr\":0x%08x,", (unsigned int)addr_by_gw);
					crc = cal_crc16((uint8_t *)temp, strlen(temp));
					free(temp);

					asprintf(&response_to_device, "LORA_RES_ADDRESS: {\"addr\":0x%08x,\"crc\":0x%04x}", (unsigned int)addr_by_gw, crc);
					loraif_debug(response_to_device, __LINE__, __FUNCTION__);
					if(xQueueSend(response_queue, &response_to_device, 2) == pdFALSE){
						loraif_debug((char *)"Can't send address to new device", __LINE__, __FUNCTION__);
					}

					goto event_handle;
				}
				else{
					loraif_debug((char *)"Can't allocate new device address", __LINE__, __FUNCTION__);
				}

				json_release_object(&json);
			}

		/** Response with device address */
			else{
				err = json_get_object(pkt.data_str, &json, (char *)"addr");
				uint32_t addr = strtol(json.value, NULL, 16);

				if(err == PKT_ERR_OK){
					if(cmd == LORA_ERR){
						loraif_debug((char *)"Device error", __LINE__, __FUNCTION__);
						cmd = LORA_ERR;
					}

					else if(cmd == LORA_UPDATE_ADDRESS){
							if(addr == addr_by_gw){
								set_response_ok(addr, cmd);
								loraif_debug((char *)"New device update address", __LINE__, __FUNCTION__);
								goto event_handle;
							}
					}

					else if(cmd == LORA_UPDATE_STATE){
							set_response_ok(addr, cmd);
							loraif_debug((char *)"Device update state", __LINE__, __FUNCTION__);
							goto event_handle;
					}

					/** LORA_UPDATE_SETTINGS, LORA_RES_DATA, LORA_UPDATE_DATA */
					else{
						if(cmd == LORA_RES_DATA) cmd = LORA_REQ_DATA;
						bool valid_addr = false;
						loraif_request_prop_t *require_resp_prop;

						for(uint8_t i=0; i<uxQueueMessagesWaiting(wait_response_queue); i++){
							if(xQueueReceive(wait_response_queue, &require_resp_prop, 2) == pdTRUE){
								if(require_resp_prop->address == addr && require_resp_prop->cmd == cmd){
									free(require_resp_prop);
									valid_addr = true;
									break;
								}
								else{
									if(xQueueSend(wait_response_queue, &require_resp_prop, 2) != pdTRUE){
										loraif_debug((char *)"Can't send to wait_response_queue", __LINE__, __FUNCTION__);
									}
								}
							}
						}

						if(valid_addr){
							loraif_dev_t *err_dev = loraif_select_device(pkt.data_str);
							err_dev->err_count = 0;
							set_response_ok(addr, cmd);
							goto event_handle;
						}

					}
				}
				else{
					loraif_debug((char *)"Unknown device", __LINE__, __FUNCTION__);
					cmd = LORA_ERR;
				}

				json_release_object(&json);
			}

			event_handle:
			asprintf(&evt_data, "%s", pkt.data_str);
			if(fpeventhandler != NULL) fpeventhandler(cmd, evt_data);
			free(evt_data);
		}

/** Parse packet error */
		else{
			loraif_debug((char *)"Can't parse received packet", __LINE__, __FUNCTION__);
		}

		json_release_object(&json);
		release_packet(&pkt);
		free(rx_full);
	}
}


void loraif_response(void){
	char *response;

	if(xQueueReceive(response_queue, &response, 2)){
		loraif_transmit(response);
		free(response);
	}
	loraif_check_timeout();
	vTaskDelay(500);
}

void loraif_check_timeout(void){
	loraif_request_prop_t *wait_response;

	uint8_t queue_len = uxQueueMessagesWaiting(wait_response_queue);
	for(uint8_t i=0; i<queue_len; i++){
		if(xQueueReceive(wait_response_queue, &wait_response, 2) == pdTRUE){

			uint32_t dt = 0, tick_now = get_tick();
			if(tick_now >= wait_response->tick_start) dt = tick_now - wait_response->tick_start;
			else                                      dt = (4294967295 - wait_response->tick_start) + tick_now;

			if(dt >= resp_timeout){
				char *evt_data;
				asprintf(&evt_data, "{\"addr\":0x%08x}", (unsigned int)wait_response->address);
				loraif_dev_t *err_dev = loraif_select_device(evt_data);
				err_dev->err_count++;

				char *tmp_dbg;
				asprintf(&tmp_dbg, "Device 0x%08x time = %lu, %d time not response", (unsigned int)wait_response->address, dt, err_dev->err_count);
				loraif_debug(tmp_dbg, __LINE__, __FUNCTION__);
				free(tmp_dbg);

				if(err_dev->err_count >= max_not_resp){
					if(fpeventhandler != NULL) fpeventhandler(LORA_DEL_DEVICE, evt_data);
				}
				free(evt_data);
				free(wait_response);
			}
			else{
				if(xQueueSend(wait_response_queue, &wait_response, 2) != pdTRUE){
					loraif_debug((char *)"Can't send to wait_response_queue", __LINE__, __FUNCTION__);
				}
			}
		}
	}
}

void loraif_request_data(void){
    if(loraif_device_list.empty()) {
        return;
    }

    for (auto device = loraif_device_list.begin(); device != loraif_device_list.end(); ++device) {
    	vTaskDelay(1000);
        loraif_request((*device)->address, LORA_REQ_DATA, (char *)"?", 1);
    }
}


void loraif_new_device(char *jdata, void *dev_data){
	pkt_err_t err;
	pkt_json_t json;
    loraif_dev_t *newdev = (loraif_dev_t *)malloc(sizeof(loraif_dev_t));

    newdev->err_count = 0;
    newdev->data = dev_data;

	err = json_get_object(jdata, &json, (char *)"addr");
	if(err == PKT_ERR_OK)
		newdev->address = strtol(json.value, NULL, 16);
	json_release_object(&json);

	err = json_get_object(jdata, &json, (char *)"name");
	if(err == PKT_ERR_OK)
		asprintf(&newdev->name, "%s", json.value);
	json_release_object(&json);

	loraif_device_list.push_back(newdev);

	char *tmp;
	asprintf(&tmp, "Add device 0x%08x(%s)", (unsigned int)newdev->address, newdev->name);
	loraif_debug(tmp, __LINE__, __FUNCTION__);
	free(tmp);

	show_device_list();
}

void loraif_remove_device(char *jdata){
	pkt_err_t err;
	pkt_json_t json;
	uint32_t del_addr = 0x00U;

	err = json_get_object(jdata, &json, (char *)"addr");
	if(err == PKT_ERR_OK)
		del_addr = strtol(json.value, NULL, 16);
	json_release_object(&json);

    if (loraif_device_list.empty()) {
    	loraif_debug((char *)"Device list empty", __LINE__, __FUNCTION__);
        return;
    }

    auto device = loraif_device_list.begin();
    while (device != loraif_device_list.end()) {
        if ((*device)->address == del_addr) {
            break;
        }
        ++device;
    }

    if (device == loraif_device_list.end()) {
    	loraif_debug((char *)"This device not available in device list", __LINE__, __FUNCTION__);
        return;
    }

    free((*device)->name);
    free(*device);

    loraif_device_list.erase(device);

	show_device_list();
}

loraif_dev_t *loraif_select_device(char *jdata){
	pkt_err_t err;
	pkt_json_t json;
	uint32_t sel_addr = 0x00;

	err = json_get_object(jdata, &json, (char *)"addr");
	if(err == PKT_ERR_OK)
		sel_addr = strtol(json.value, NULL, 16);
	json_release_object(&json);

    if (loraif_device_list.empty()) {
    	loraif_debug((char *)"Device list empty", __LINE__, __FUNCTION__);
        return NULL;
    }

    auto device = loraif_device_list.begin();
    while (device != loraif_device_list.end()) {
        if ((*device)->address == sel_addr) {
            break;
        }
        ++device;
    }

    if (device == loraif_device_list.end()) {
    	loraif_debug((char *)"This device not available in device list", __LINE__, __FUNCTION__);
        return NULL;
    }

    return (*device);
}



#endif /* ENABLE_COMPONENT_WIFIIF */
