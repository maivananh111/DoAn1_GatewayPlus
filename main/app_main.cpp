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


static volatile void exception_handler(void);
static void beep_loop(uint8_t loop, uint16_t active_time, uint16_t idle_time);

spi_config_t spi5_conf = {
	.clkport  = GPIOF,
	.clkpin   = 7,
	.misoport = GPIOF,
	.misopin  = 8,
	.mosiport = GPIOF,
	.mosipin  = 9,
};
sx127x lora(GPIOF, 6, GPIOE, 3, GPIOA, 0);
#define LORA_SEND_SYNCWORD 0x12
#define LORA_RECV_SYNCWORD 0xAB
QueueHandle_t lora_queue;
void gpioA0_event_handler(void *);
void lora_event_handler(void *, uint8_t len);
void loraif_event_handler(lora_event_t event, char *data);
void lora_request(char *str);
void task_loratx(void *);
void task_lorarx(void *);

#define WF_BUFF_SIZE 2048
usart_config_t wf_uart_conf = {
	.baudrate = 115200,
	.control = USART_INTERRUPT_CONTROL,
	.interruptoption = USART_RECEIVE_INTERRUPT,
	.interruptpriority = 6,
	.txport = GPIOA,
	.txpin = 9,
	.rxport = GPIOA,
	.rxpin = 10,
};

void wifi_uart_handler(usart_event_t event, void *param);
void wifi_command_handler(wifi_cmd_t cmd, void *param);
void wifi_request(char *str, uint16_t size);
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

static void wifiif_transmit(char *str){
	uint8_t MAX_UART_TX_BUFFER_SIZE = 100;
	int16_t len = strlen(str);
	int16_t remaining = len;

	while(remaining > 0){
		int16_t sendSize = (remaining > MAX_UART_TX_BUFFER_SIZE)? MAX_UART_TX_BUFFER_SIZE : remaining;
		usart1->transmit((uint8_t *)str, sendSize);
		remaining -= sendSize;
		str += sendSize;
		vTaskDelay(1);
	}
	vTaskDelay(1);
	usart1->transmit((uint8_t *)"\r\nend\r\n", 7);
}

void app_main(void){
	extern void HAL_Driver_Init(void);
	HAL_Driver_Init();

	gpio_port_clock_enable(GPIOB);
	gpio_port_clock_enable(GPIOC);
	gpio_set_mode(GPIOC, 13, GPIO_OUTPUT_PUSHPULL);
	gpio_set_mode(GPIOB, 14, GPIO_OUTPUT_PUSHPULL);
	register_exception_handler(exception_handler);

	xTaskCreate(task_lorarx, "task_lorarx", byte_to_word(8192), NULL, 15, NULL);

	xTaskCreate(task_loratx, "task_loratx", byte_to_word(4096), NULL, 5, NULL);

//	xTaskCreate(task_wifi, "task_wifi", byte_to_word(8192), NULL, 2, NULL);

	while(1){
		gpio_toggle(GPIOC, 13);
		vTaskDelay(100);
	}
}



void task_loratx(void *){

	while(1){
		loraif_request_data();
		LOG_MEM(TAG, "Free heap = %lu.", sys_get_free_heap_size());
		vTaskDelay(5000);
	}
}

void task_lorarx(void *){
	spi5->init(&spi5_conf);
	exti_register_event_handler(0, gpioA0_event_handler, NULL);

	if(lora.init(spi5, 433E6, 20, 7)) LOG_INFO(TAG, "Lora Initialize OKE.");
	else LOG_ERROR(TAG, "Lora Initialize Failed.");

	lora_queue = xQueueCreate(20, sizeof(uint32_t));
	loraif_init(&lora, LORA_SEND_SYNCWORD, LORA_RECV_SYNCWORD, 10000, 3);
	loraif_register_event_handler(loraif_event_handler);

	lora.setSyncWord(0x3F);
	lora.register_event_handler(NULL, lora_event_handler);
	lora.Receive(0);

	while(1){
		loraif_rx_process(&lora_queue);
		loraif_response();
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
		if(loraif_check_crc(lora_RxBuf) == true){
			BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
			if(xQueueSendFromISR(lora_queue, &lora_RxBuf, &pxHigherPriorityTaskWoken) != pdPASS){
				LOG_ERROR(TAG, "LoRa error queue receive.");
			}
		}
		else{
			LOG_ERROR(TAG, "LoRa received packet error CRC.");
		}
	}
}

void loraif_event_handler(lora_event_t event, char *data){
	if(data != NULL) LOG_INFO(TAG, "LoRa data: %s [packet RSSI = %d, RSSI = %d]", data, lora.packetRssi(), lora.rssi());
	switch(event){
		case LORA_REQ_ADDRESS:
			LOG_EVENT(TAG, "LORA_REQ_ADDRESS");
			beep_loop(2, 50, 50);
		break;
		case LORA_ADD_DEVICE:{ //148016
			LOG_EVENT(TAG, "LORA_ADD_DEVICE");
			beep_loop(3, 50, 50);
			dev_struct_t *dev = add_device_properties(data);
			loraif_add_device(data, dev);
//			vTaskDelay(500);
//			firebase_new_device(dev);
		}
		break;
		case LORA_UPDATE_STATE:
			LOG_EVENT(TAG, "LORA_UPDATE_STATE");
		break;
		case LORA_UPDATE_SETTINGS:
			LOG_EVENT(TAG, "LORA_UPDATE_SETTINGS");
		break;
		case LORA_REQ_DATA:
			LOG_EVENT(TAG, "LORA_REQ_DATA");
			beep_loop(1, 50, 1);
		break;
		case LORA_UPDATE_DATA:
			LOG_EVENT(TAG, "LORA_UPDATE_DATA");
		break;
		case LORA_DEVICE_NOT_RESPONSE:
			LOG_WARN(TAG, "LORA_DEVICE_NOT_RESPONSE");
		break;
		case LORA_REMOVE_DEVICE:
			LOG_WARN(TAG, "LORA_REMOVE_DEVICE");
			loraif_remove_device(data);
			remove_device_properties(data);
		break;
		case LORA_ERR:
			LOG_EVENT(TAG, "LORA_ERR");
		break;
		default:
			LOG_EVENT(TAG, "LoRa other event.");
		break;
	}
}





void task_wifi(void *){
	usart1->init(&wf_uart_conf);
	usart1->register_event_handler(wifi_uart_handler, NULL);
	usart1->receive_to_idle_start_it(WF_BUFF_SIZE);

	wifiif_init(wifi_request);
	wifiif_register_command_handler(wifi_command_handler);

	wifiif_restart();
	restart_wifi:
	beep_loop(1, 20, 1);
	if(!wifiif_state_is_running()) wifiif_restart();
	vTaskDelay(1000);

	wifiif_connect((char *)"D305GV2.4GHz", (char *)"NoPassword132132", (char *)"WIFI_AUTH_WPA2_PSK");
	vTaskDelay(1000);
	uint8_t reconn_num = 0;
	while(wifiif_wificonnected() == false) {
		wifiif_checkconnect();
		vTaskDelay(1000);
		reconn_num++;
		if(reconn_num > 5) goto restart_wifi;
	}
	wifiif_state_running(true);

	vTaskDelay(1000);
	firebase_init((char *)"https://iotnhakho-default-rtdb.asia-southeast1.firebasedatabase.app", (char *)"YAg8QGH48Xlbjpk9UMh5JkjgYCCbeMSM4Ak5SNHp");

	wifiif_http_client_set_url((char *)"/Kho3/.json");
	wifiif_http_client_set_data((char *)"{}");
	wifiif_http_client_set_method((char *)"HTTP_METHOD_GET");



	while(1){
		wifiif_checkconnect();
		if(!wifiif_state_is_running() || wifiif_wificonnected() == false) {
			LOG_ERROR(TAG, "WiFi module error reset.");
			goto restart_wifi;
		}

		wifiif_http_client_set_url((char *)"/Kho3/.json");
		wifiif_http_client_set_method((char *)"HTTP_METHOD_GET");
		wifiif_http_client_request();

		vTaskDelay(5000);
	}
}
void wifi_request(char *str, uint16_t size){
	usart1->transmit((uint8_t *)str, size);
	delay_ms(1);
}
void wifi_command_handler(wifi_cmd_t cmd, void *param){
	char *resp_data = (char *)param;

	switch(cmd){
		case WIFI_ISCONNECTED:
//			LOG_EVENT("WIFI_ISCONNECTED", "%s.", resp_data);
		break;
		case WIFI_ERR:
			wifiif_state_running(false);
			LOG_EVENT(TAG, "WiFi module error reset.");
			wifiif_restart();
//			beep_loop(1, 50, 1);
		break;
		case WIFI_SCAN:
			LOG_EVENT("WIFI_CMD_WIFI_SCAN", "%s.", resp_data);
		break;
		case WIFI_HTTP_CLIENT_RESPONSE:
			LOG_EVENT("HTTP DATA", "%s.", resp_data);
		break;

		default:
			LOG_EVENT(TAG, "WiFi module responded: %s.", resp_data);
//			wifiif_restart();
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
		if(strcmp(rxdata, "WIFI_RESTART: OK") == 0) wifiif_state_running(false);

		wifiif_get_break_data(rxdata);

		usart1->receive_to_idle_start_it(WF_BUFF_SIZE);
	}
}


static void beep_loop(uint8_t loop, uint16_t active_time, uint16_t idle_time){
	while(loop--){
		gpio_set(GPIOB, 14);
		vTaskDelay(active_time);
		gpio_reset(GPIOB, 14);
		vTaskDelay(idle_time);
	}
}

static volatile  void exception_handler(void){
	gpio_set(GPIOB, 14);
}









