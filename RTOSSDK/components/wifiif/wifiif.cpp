/*
 * wifi.cpp
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#include "component_config.h"
#if ENABLE_COMPONENT_WIFIIF

#include "wifiif/wifiif.h"
#include "parse_packet/parse_packet.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "system/log.h"
#include "periph/systick.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#if ENABLE_COMPONENT_WIFIIF_DEBUG
static const char *TAG = "WiFiIF";
#define LOG_LEVEL LOG_DEBUG
#endif /* ENABLE_COMPONENT_WIFIIF_DEBUG */
static const char *command_string[] = {
	"WIFI_ERR",

	"WIFI_RESTART",
	/**
	 * Network control command.
	 */
	"WIFI_SCAN",
	"WIFI_ISCONNECTED",
	"WIFI_CONN",
	"WIFI_DISCONN",
	"WIFI_GETIP",

	/**
	 * HTTP client command.
	 */
	"WIFI_HTTP_CLIENT_NEW",
	"WIFI_HTTP_CLIENT_CONFIG",
	"WIFI_HTTP_CLIENT_INIT",
	"WIFI_HTTP_CLIENT_CLEAN",
	"WIFI_HTTP_CLIENT_SET_HEADER",
	"WIFI_HTTP_CLIENT_SET_URL",
	"WIFI_HTTP_CLIENT_SET_METHOD",
	"WIFI_HTTP_CLIENT_SET_DATA",
	"WIFI_HTTP_CLIENT_REQUEST",
	"WIFI_HTTP_CLIENT_RESPONSE",

	"WIFI_CMD_NUM",
};


static void (*fprequest)(char *str, uint16_t len) = NULL;
static void (*fpcommand_handler)(wifi_cmd_t cmd, void *param);
static volatile bool wifi_state = false, wifi_connected = false;
static QueueHandle_t q_response;
static EventGroupHandle_t e_response;


static void wifiif_debug(char *str, int line, const char *func){
#if ENABLE_COMPONENT_WIFIIF_DEBUG
	LOG_LEVEL(TAG, "%s, Line: %d Function: %s", str, line, func);
#endif /* ENABLE_COMPONENT_WIFIIF_DEBUG */
}

static void wifiif_transmit(char *str){
	uint8_t MAX_UART_TX_BUFFER_SIZE = 100;
	int16_t len = strlen(str);
	int16_t remaining = len;

	while(remaining > 0){
		int16_t sendSize = (remaining > MAX_UART_TX_BUFFER_SIZE)? MAX_UART_TX_BUFFER_SIZE : remaining;
		if(fprequest) fprequest(str, sendSize);
		remaining -= sendSize;
		str += sendSize;
	}
	delay_ms(1);
	if(fprequest) fprequest((char *)"\r\nend\r\n", 7);
}

void wifiif_get_break_data(char *brk_data){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(strcmp(brk_data, "\r\nend\r\n") != 0) {
		if(xQueueSendFromISR(q_response, &brk_data, &xHigherPriorityTaskWoken) != pdTRUE) LOG_ERROR(TAG, "Send to queue fail.");
    }
    else{
    	free(brk_data);
    	xEventGroupSetBitsFromISR(e_response, DATA_EVENTBIT, &xHigherPriorityTaskWoken);
    }
}

static void wifiif_merge_data(char **dest_buffer){
 	   char *break_data;
 	   uint16_t total_len = 0;
 	   uint8_t queue_len = uxQueueMessagesWaiting(q_response);
 	   /** get total string length */
 	   for(uint8_t i=0; i<queue_len; i++){
 		   if(xQueueReceive(q_response, &break_data, 2) == pdTRUE){
 			   total_len += strlen(break_data);
 			   xQueueSend(q_response, &break_data, 2);
 		   }
 	   }

 	   /** string concatenate */
 	  *dest_buffer = (char *)malloc(total_len + 1);
 	   char *tmp_data = *dest_buffer;
 	   for(uint8_t i=0; i<queue_len; i++){
 		   if(xQueueReceive(q_response, &break_data, 2) == pdTRUE){
 			   uint16_t len = strlen(break_data);
 			   memcpy(tmp_data, break_data, len);
 			   tmp_data += len;
 			   free(break_data);
 		   }
 	   }
 	  (*dest_buffer)[total_len] = '\0';
}

static int wifiif_is_err(char *str){
	return strcmp(str, "ERR");
}

static void wifiif_request(wifi_cmd_t cmd, char *data){
	char *cmd_str = cmd_to_str(cmd, command_string);
	char *req_data;
	asprintf(&req_data, "%s: %s", cmd_str, data);
	wifiif_transmit(req_data);
#if ENABLE_COMPONENT_WIFIIF_DEBUG
//	wifiif_debug(req_data, __LINE__, __FUNCTION__);
#endif /* ENABLE_COMPONENT_WIFIIF_DEBUG */
	free(req_data);

	EventBits_t bits = xEventGroupWaitBits(e_response, DATA_EVENTBIT, pdTRUE, pdFALSE, WIFI_DEFAULT_TIMEOUT);
	if(bits == DATA_EVENTBIT){
		char *response_data;
		pkt_t pkt;
		pkt_err_t err = PKT_ERR_OK;

		wifiif_merge_data(&response_data);
		err = parse_packet(response_data, &pkt);
		if(err != PKT_ERR_OK){
			wifiif_debug((char *)"Can't parse response.", __LINE__, __FUNCTION__);
			release_packet(&pkt);
			if(response_data != NULL) free(response_data);

			return;
		}
		if(wifiif_is_err(pkt.data_str) != 0){ // Is not wifi command error.
			char *data = (char *)malloc(strlen(pkt.data_str)+1);
			memcpy(data, pkt.data_str, strlen(pkt.data_str));
			data[strlen(pkt.data_str)] = '\0';

			wifi_cmd_t command = (wifi_cmd_t)str_to_cmd(pkt.cmd_str, command_string, WIFI_CMD_NUM);
			if(command == WIFI_ISCONNECTED){
				pkt_json_t json;
				pkt_err_t err = json_get_object(pkt.data_str, &json, (char *)"isconnected");
				if(err == PKT_ERR_OK){
					if(strcmp(json.value, "1") == 0) {
						wifi_state = true;
						wifi_connected = true;
					}
					else if(strcmp(json.value, "0") == 0) {
						wifi_state = false;
						wifi_connected = false;
					}
				}
				json_release_object(&json);
			}
			else if(command == WIFI_RESTART){
				wifi_state = false;
				wifi_connected = false;
			}

			if(fpcommand_handler) fpcommand_handler(command, data); // Handle wifiif event.

			if(data != NULL) free(data);
		}
		else{ // Wifi command error.
			wifiif_debug((char *)"WiFi module error.", __LINE__, __FUNCTION__);
			if(fpcommand_handler) fpcommand_handler(WIFI_ERR, NULL);
		}
		release_packet(&pkt);
		if(response_data != NULL) free(response_data);
	}
	else{ // Parse packet fail.
		wifiif_debug((char *)"WiFi module not response the request", __LINE__, __FUNCTION__);
		if(fpcommand_handler) fpcommand_handler(WIFI_ERR, NULL);
	}

}

/**
 * WiFi setup function.
 */
void wifiif_init(void (*prequest)(char *, uint16_t)){
	fprequest = prequest;

	q_response = xQueueCreate(20, sizeof(char *));
	e_response = xEventGroupCreate();
}

void wifiif_register_command_handler(void (*pcommand_handler)(wifi_cmd_t cmd, void *param)){
	fpcommand_handler = pcommand_handler;
}

/**
 * WiFi control.
 */
void wifiif_restart(void){
	wifiif_request(WIFI_RESTART, (char *)"{}");
	wifi_state = false;
	wifi_connected = false;
}
void wifiif_scan(void){
	wifiif_request(WIFI_SCAN, (char *)"{}");
}
void wifiif_checkconnect(void){
	wifiif_request(WIFI_ISCONNECTED, (char *)"{}");
}
bool wifiif_wificonnected(void){
	return wifi_connected;
}
void wifiif_set_wificonnect_state(bool state){
	wifi_connected = state;
}
void wifiif_connect(char *ssid, char *pass, char *auth){
	char *data;
	asprintf(&data, "{\"ssid\":\"%s\",\"pass\":\"%s\",\"auth\":\"%s\"}", ssid, pass, auth);

	wifiif_request(WIFI_CONN, data);

	free(data);
}
void wifiif_disconnect(void){
	wifiif_request(WIFI_DISCONN, (char *)"{}");
}
void wifiif_getIP(void){
	wifiif_request(WIFI_GETIP, (char *)"{}");
}


/**
 * HTTP Client.
 */
void wifiif_http_client_new(void){
	wifiif_request(WIFI_HTTP_CLIENT_NEW, (char *)"{}");
}

void wifiif_http_client_config(char *config){
	wifiif_request(WIFI_HTTP_CLIENT_CONFIG, config);
}

void wifiif_http_client_init(void){
	wifiif_request(WIFI_HTTP_CLIENT_INIT, (char *)"{}");
}

void wifiif_http_client_clean(void){
	wifiif_request(WIFI_HTTP_CLIENT_CLEAN, (char *)"{}");
}

void wifiif_http_client_set_header(char *key, char *value){
	char *data;
	asprintf(&data, "{\"key\":\"%s\",\"value\":\"%s\"}", key, value);

	wifiif_request(WIFI_HTTP_CLIENT_SET_HEADER, data);

	free(data);
}

void wifiif_http_client_set_url(char *url){
	char *tmp;
	asprintf(&tmp, "{\"url\":\"%s\"}", url);

	wifiif_request(WIFI_HTTP_CLIENT_SET_URL, tmp);

	free(tmp);
}

void wifiif_http_client_set_method(char *method){
	char *tmp;
	asprintf(&tmp, "{\"method\":\"%s\"}", method);

	wifiif_request(WIFI_HTTP_CLIENT_SET_METHOD, tmp);

	free(tmp);
}

void wifiif_http_client_set_data(char *data){
	char *tmp;
	asprintf(&tmp, "{\"data\":%s}", data);

	wifiif_request(WIFI_HTTP_CLIENT_SET_DATA, tmp);

	free(tmp);
}

void wifiif_http_client_request(void){
	wifiif_request(WIFI_HTTP_CLIENT_REQUEST, (char *)"{}");
}


void wifiif_state_running(bool state){
	wifi_state = state;
}
bool wifiif_state_is_running(void){
	return wifi_state;
}



#endif /* ENABLE_COMPONENT_WIFIIF */
