/**
 * app_main.cpp
 *
 *  Created on: Jan 5, 2023
 *      Author: anh
 */

#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "system/system.h"
#include "system/ret_err.h"
#include "system/log.h"
#include "periph/systick.h"
#include "periph/gpio.h"
#include "periph/exti.h"
#include "periph/rng.h"
#include "periph/usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "sx127x/sx127x.h"
#include "fatfs.h"
#include "network/netconnect.h"
#include "protocols/tcp/tcp_client.h"
#include "wifiif/wifiif.h"
#include "loraif/loraif.h"
#include "app/app.h"

static const char *TAG = "Main";


spi_config_t spi5_conf = {
	.clkport  = GPIOF,
	.clkpin   = 7,
	.misoport = GPIOF,
	.misopin  = 8,
	.mosiport = GPIOF,
	.mosipin  = 9,
};
sx127x lora(GPIOF, 6, GPIOE, 3, GPIOA, 0);
QueueHandle_t lora_queue;
QueueHandle_t lora_device_queue;
void gpioA0_event_handler(void *);
void lora_event_handler(void *, uint8_t len);
void loraif_event_handler(lora_event_t event, char *data);
void lora_request(char *str);
void task_lorarx(void *);
void task_loratx(void *);

#define WF_BUFF_SIZE 2048
usart_config_t wf_uart_conf = {
	.baudrate = 115200,
	.control = USART_INTERRUPT_CONTROL,
	.interruptselect = USART_RECEIVE_INTERRUPT,
	.interruptpriority = 6,
	.txport = GPIOA,
	.txpin = 9,
	.rxport = GPIOA,
	.rxpin = 10,
};
void wifi_uart_handler(usart_event_t event, void *param);
void wifi_command_handler(wifi_cmd_t cmd, void *param);
void wifi_request(char *str);
void task_wifi(void *);

dev_struct_t kho3 = {
	kho3.env.temp = 31.2,
	kho3.env.humi = 65.8,
	kho3.env.curr = 3.25,
	kho3.env.time = (char *)"14:30:00 05/05/23",
	kho3.ctrl.relay1 = 1,
	kho3.ctrl.relay2 = 0,
	kho3.ctrl.relay3 = 1,
	kho3.ctrl.relay4 = 0,
	kho3.sett.mode = 1,
	kho3.sett.type = 0,
	kho3.sett.max_temp = 38.0,
	kho3.sett.min_temp = 26.0,
	kho3.sett.time_start = (char *)"00:00:00",
	kho3.sett.time_stop = (char *)"00:00:00",
	kho3.prop.address = 0xFDEA15DE,
	kho3.prop.name = (char *)"Kho3",
};

void app_main(void){
	extern void HAL_Driver_Init(void);
	HAL_Driver_Init();

	gpio_port_clock_enable(GPIOC);
	gpio_set_mode(GPIOC, 13, GPIO_OUTPUT_PUSHPULL);

//	xTaskCreate(task_lorarx, "task_lorarx", byte_to_word(8192), NULL, 5, NULL);
//	xTaskCreate(task_loratx, "task_loratx", byte_to_word(8192), NULL, 5, NULL);
	xTaskCreate(task_wifi, "task_wifi", byte_to_word(8192), NULL, 6, NULL);

	while(1){
		gpio_toggle(GPIOC, 13);
		vTaskDelay(100);
	}
}


void task_lorarx(void *){
	spi5->init(&spi5_conf);
	exti_register_event_handler(0, gpioA0_event_handler, NULL);

	if(lora.init(spi5, 433E6, 20, 7)) LOG_INFO(TAG, "Lora Initialize OKE.");
	else LOG_ERROR(TAG, "Lora Initialize Failed.");

	lora_queue = xQueueCreate(10, sizeof(uint32_t));
	loraif_init(&lora, tim2, 25000, 3);
	loraif_register_event_handler(loraif_event_handler);

	lora.setSyncWord(0x3F);
	lora.register_event_handler(NULL, lora_event_handler);
	lora.Receive(0);

	while(1){
		loraif_process(&lora_queue);
		loraif_response();
		loraif_check_timeout();
		vTaskDelay(2);
	}
}
void task_loratx(void *){
	lora_device_queue = xQueueCreate(20, sizeof(uint32_t));
	uint32_t addr;
	while(1){
		if(xQueueReceive(lora_device_queue, &addr, 10)){
			lora_request(addr, LORA_REQ_DATA, "?");
			vTaskDelay(5000);
		}
		loraif_response();
		loraif_check_timeout();
		vTaskDelay(2);
	}
}
void gpioA0_event_handler(void *){
	lora.IRQHandler();
}
void lora_event_handler(void *, uint8_t len){
	uint8_t packetSize = len;
	if(packetSize){
		char *lora_RxBuf;
		lora_RxBuf = (char *)malloc(packetSize+1);
		lora.receive(lora_RxBuf);
		lora_RxBuf[packetSize] = '\0';
		LOG_WARN(TAG, "Receive: %s, packet RSSI = %d, SNR = %.02f, RSSI = %d", lora_RxBuf, lora.packetRssi(), lora.packetSnr(), lora.rssi());

		BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
		if(xQueueSendFromISR(lora_queue, &lora_RxBuf, &pxHigherPriorityTaskWoken) == pdPASS){

		}
	}
}

void loraif_event_handler(lora_event_t event, char *data){
	if(data != NULL) LOG_ERROR(TAG, "LoRa data: %s", data);
	switch(event){
		case LORA_REQ_ADDRESS:
			LOG_RET(TAG, "LORA_REQ_ADDRESS");
		break;
		case LORA_UPDATE_ADDRESS:{
			LOG_RET(TAG, "LORA_UPDATE_ADDRESS");
			pkt_json_t json;
			pkt_err_t err = json_get_object(data, &json, (char *)"addr");
			uint32_t addr = strtol(json.value, NULL, 16);
			xQueueSend(lora_device_queue, &addr, 10);
		}
		break;
		case LORA_UPDATE_STATE:
			LOG_RET(TAG, "LORA_UPDATE_STATE");
		break;
		case LORA_UPDATE_SETTINGS:
			LOG_RET(TAG, "LORA_UPDATE_SETTINGS");
		break;
		case LORA_REQ_DATA:
			LOG_RET(TAG, "LORA_REQ_DATA");
		break;
		case LORA_UPDATE_DATA:
			LOG_RET(TAG, "LORA_UPDATE_DATA");
		break;
		case LORA_DEL_DEVICE:
			LOG_RET(TAG, "LORA_DEL_DEVICE");
		break;
		default:
			LOG_RET(TAG, "LoRa other event.");
		break;

	}
}



void task_wifi(void *){
	usart1->init(&wf_uart_conf);
	usart1->register_event_handler(wifi_uart_handler, NULL);
	usart1->receive_to_idle_start_it(WF_BUFF_SIZE);

	wifiif_register_request_function(wifi_request);
	wifiif_register_command_handler(wifi_command_handler);

	wifiif_restart();
	restart_wifi:
	if(!wifiif_state_is_running()) wifiif_restart();
	vTaskDelay(1000);
	wifiif_connect((char *)"FREE", (char *)"0986382835", (char *)"WIFI_AUTH_WPA2_PSK");
	wifiif_state_running(true);

	firebase_init((char *)"https://iotnhakho-default-rtdb.asia-southeast1.firebasedatabase.app", (char *)"YAg8QGH48Xlbjpk9UMh5JkjgYCCbeMSM4Ak5SNHp");

	firebase_new_device(&kho3);
	firebase_remove_device(&kho3);
	wifiif_http_client_set_url((char *)"/.json?auth=YAg8QGH48Xlbjpk9UMh5JkjgYCCbeMSM4Ak5SNHp");
	wifiif_http_client_set_url((char *)"/.json?auth=YAg8QGH48Xlbjpk9UMh5JkjgYCCbeMSM4Ak5SNHpsdgdfgsfdhfgdsvhjfvbygv cygfjsduygcevhbjdygfduvcbhjsxdygfvchbxsjuyfdgvch bwsxcdygvfch wbsxhcydgfvcwbshcgdvytdhfbjycdugfvhbxycgfvc bdshxcgydvt bhdcgyvydb tvg xhfdvt");

	while(1){
		if(!wifiif_state_is_running()) {
			LOG_ERROR(TAG, "WiFi module error reset.");
			wifiif_reset_response_state();
			goto restart_wifi;
		}

		wifiif_http_client_set_data((char *)"{}");
		wifiif_http_client_set_method((char *)"HTTP_METHOD_GET");
		wifiif_http_client_request();

		LOG_MEM(TAG, "Free heap size: %lu", sys_get_free_heap_size());

		vTaskDelay(5000);
	}
}
void wifi_request(char *str){
	while(*str) {
		USART1 -> DR = *str++;
		while(!(USART1 -> SR & USART_SR_TC));
	}
}
void wifi_command_handler(wifi_cmd_t cmd, void *param){
	char *resp_data = (char *)param;

	switch(cmd){
		case WIFI_ERR:
			wifiif_state_running(false);
			LOG_WARN(TAG, "WiFi module error reset.");
		break;
		case WIFI_SCAN:
			LOG_INFO("WIFI_CMD_WIFI_SCAN", "%s.", resp_data);
		break;
		case WIFI_HTTP_CLIENT_RESPONSE:
			LOG_INFO("HTTP DATA", "%s.", resp_data);
		break;

		default:
			LOG_INFO(TAG, "WiFi module responded: %s.", resp_data);
		break;
	}
}
void wifi_uart_handler(usart_event_t event, void *param){
	if(event == (USART_EVENT_IDLE_STATE)){
		char *rxdata;
		stm_ret_t ret = usart1->get_buffer((uint8_t **)&rxdata);
		if(!is_oke(&ret)){
			LOG_ERROR(TAG, "Can't get UART data.");
			return;
		}
//		LOG_WARN(TAG, "%s", rxdata);
		if(strcmp(rxdata, "WIFI_RESTART: OK") == 0) wifiif_state_running(false);

		wifiif_set_response_state(rxdata);

		usart1->receive_to_idle_start_it(WF_BUFF_SIZE);
	}
}
















