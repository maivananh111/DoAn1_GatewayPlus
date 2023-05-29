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
#include "periph/tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

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

#define LORAIF_SEND_SYNCWORD  0x12
#define LORAIF_RECV_SYNCWORD  0xAB
#define LORAIF_DEVICE_TIMEOUT 10000 // 10s
#define LORAIF_REQUEST_DATA_E_BITS       (1<<11)

spi_config_t lora_spi_conf = {
	.clkport  = GPIOF,
	.clkpin   = 7,
	.misoport = GPIOF,
	.misopin  = 8,
	.mosiport = GPIOF,
	.mosipin  = 9,
};
sx127x lora(GPIOF, 6, GPIOE, 3, GPIOA, 0);
tim_config_t loraif_request_tim = {
	.prescaler = 54000, // 108MHz/54000 = 2KHz.
	.reload = 120000,   // 60s.
	.interrupt = TIM_INTERRUPT_ENABLE,
	.interruptpriority = 6,
};
TaskHandle_t h_task_loraif_receive_process, h_task_loraif_request_device_data;
QueueHandle_t q_loraif_receive_data;
SemaphoreHandle_t s_lora_enable_recv;
EventGroupHandle_t e_loraif_request_data;

void lora_gpioA0_event_handler(void *);
void lora_event_handler(void *, uint8_t len);
void loraif_event_handler(lora_event_t event, uint32_t device_address, char *data);
void loraif_request_data_tim_event_handler(tim_channel_t channel, tim_event_t event, void *param);

void task_loraif_receive_process(void *);
void task_loraif_request_device_data(void *);


#define WIFIIF_BUFF_SIZE              1024
#define WIFIIF_GET_DEVICE_DATA_E_BITS (1<<12)

usart_config_t wifiif_uart_conf = {
	.baudrate = 115200,
	.control = USART_INTERRUPT_CONTROL,
	.interruptoption = USART_RECEIVE_INTERRUPT,
	.interruptpriority = 6,
	.txport = GPIOA,
	.txpin = 9,
	.rxport = GPIOA,
	.rxpin = 10,
};
tim_config_t wifiif_get_control_tim = {
	.prescaler = 54000, // 108MHz/54000 = 2KHz.
	.reload = 20000,     // 5s.
	.interrupt = TIM_INTERRUPT_ENABLE,
	.interruptpriority = 7,
};

EventGroupHandle_t e_wifiif_get_device_data;
TaskHandle_t q_task_wifiif_get_device_data;

void wifi_uart_handler(usart_event_t event, void *param);
void wifiif_uart_send_string(char *str, uint16_t size);
void wifiif_command_handler(wifi_cmd_t cmd, void *param);
void wifiif_get_device_data_tim_event_handler(tim_channel_t channel, tim_event_t event, void *param);

void task_wifiif_start(void *);
void task_wifiif_get_device_data(void *);

/**
 * Main application.
 */
void app_main(void){
	extern void HAL_Driver_Init(void);
	HAL_Driver_Init();

	gpio_port_clock_enable(GPIOB);
	gpio_port_clock_enable(GPIOC);
	gpio_set_mode(GPIOC, 13, GPIO_OUTPUT_PUSHPULL);
	gpio_set_mode(GPIOB, 14, GPIO_OUTPUT_PUSHPULL);
	register_exception_handler(exception_handler);

	xTaskCreate(task_loraif_receive_process,     "task_loraif_receive_process",     byte_to_word(8192), NULL, 15, &h_task_loraif_receive_process);
	xTaskCreate(task_loraif_request_device_data, "task_loraif_request_device_data", byte_to_word(4096), NULL, 5,  &h_task_loraif_request_device_data);

	xTaskCreate(task_wifiif_start,               "task_wifiif_start",               byte_to_word(4096), NULL, 2, NULL);
	xTaskCreate(task_wifiif_get_device_data,     "task_wifiif_get_device_data",     byte_to_word(4096), NULL, 6,  &q_task_wifiif_get_device_data);

	while(1){
		gpio_toggle(GPIOC, 13);
		vTaskDelay(100);
	}
}


/**
 * LoRa application.
 */

void task_loraif_receive_process(void *){
	spi5->init(&lora_spi_conf);
	exti_register_event_handler(0, lora_gpioA0_event_handler, NULL);

	if(lora.init(spi5, 433E6, 20, 7)) LOG_INFO(TAG, "Lora Initialize successful.");
	else LOG_ERROR(TAG, "Lora Initialize Failed.");
	lora.register_event_handler(NULL, lora_event_handler);

	loraif_init(&lora, LORAIF_SEND_SYNCWORD, LORAIF_RECV_SYNCWORD, 10000, 3);
	loraif_register_event_handler(loraif_event_handler);

	q_loraif_receive_data = xQueueCreate(20, sizeof(uint32_t));
	s_lora_enable_recv = xSemaphoreCreateBinary();

	while(1){
		loraif_response_to_device();
		loraif_receive_process(&q_loraif_receive_data);
		loraif_check_device_timeout();
	}
}
void task_loraif_request_device_data(void *){
	e_loraif_request_data = xEventGroupCreate();

	tim5->init(&loraif_request_tim);
	tim5->register_event_handler(loraif_request_data_tim_event_handler, NULL);
	tim5->start_it();

	while(1){
		EventBits_t bit = xEventGroupWaitBits(e_loraif_request_data, LORAIF_REQUEST_DATA_E_BITS, pdTRUE, pdFALSE, 10);

		if(bit == LORAIF_REQUEST_DATA_E_BITS && !loraif_device_list.empty()) {
			auto device = loraif_device_list.begin();
			__IO uint32_t tick = get_tick();
		    while(device != loraif_device_list.end()) {
		    	if(get_tick() - tick > 1000){
					loraif_send_request((*device)->address, LORA_REQ_DATA, (char *)"?", 1);
					tick = get_tick();
					++device;
		    	}
		    }
		}

	}
}
void lora_gpioA0_event_handler(void *){
	lora.IRQHandler();
}
void lora_event_handler(void *, uint8_t len){
	uint8_t packetSize = len;
	if(packetSize){
		char *lora_RxBuf;
		lora_RxBuf = (char *)malloc(packetSize+1);
		lora.receive(lora_RxBuf);
		lora_RxBuf[packetSize] = '\0';
		if(loraif_check_receive_data_crc(lora_RxBuf) == true){
			BaseType_t pxHigherPriorityTaskWoken = pdTRUE;
			if(xSemaphoreTakeFromISR(s_lora_enable_recv, &pxHigherPriorityTaskWoken)){
				if(xQueueSendFromISR(q_loraif_receive_data, &lora_RxBuf, &pxHigherPriorityTaskWoken) != pdPASS)
					LOG_ERROR(TAG, "LoRa error queue receive.");
				xSemaphoreGiveFromISR(s_lora_enable_recv, &pxHigherPriorityTaskWoken);
			}
			else
				free(lora_RxBuf);
		}
		else{
			LOG_ERROR(TAG, "LoRa received packet error CRC.");
			free(lora_RxBuf);
		}
	}
}
void loraif_event_handler(lora_event_t event, uint32_t device_address, char *data){
	vTaskSuspend(h_task_loraif_request_device_data);
	vTaskSuspend(q_task_wifiif_get_device_data);
	if(data != NULL) LOG_INFO(TAG, "%s  [packet RSSI = %d, RSSI = %d]", data, lora.packetRssi(), lora.rssi());
	switch(event){
		case LORA_REQ_ADDRESS:
			LOG_EVENT(TAG, "LORA_REQ_ADDRESS");
			beep_loop(2, 50, 50);
		break;
		case LORA_ADD_DEVICE:{ //148016
			LOG_EVENT(TAG, "LORA_ADD_DEVICE");
			beep_loop(3, 50, 50);
			if(loraif_isvalid_address(device_address)){
				dev_struct_t *dev = add_device_properties(device_address, data);
				loraif_add_device(device_address, data, dev);
				firebase_new_device(dev);
			}
			else{
				LOG_ERROR(TAG, "Event device address invalid.");
			}
		}
		break;
		case LORA_REQ_DATA:
			LOG_EVENT(TAG, "LORA_REQ_DATA");
			if(loraif_isvalid_address(device_address))
				send_envdata_to_firebase(device_address, data);
			else
				LOG_ERROR(TAG, "Event device address invalid.");
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
			if(loraif_isvalid_address(device_address)){
				dev_struct_t *dev = select_device_properties(device_address);
				if(dev != NULL) firebase_remove_device(dev);
				else LOG_ERROR(TAG, "Event device error.");
				remove_device_properties(device_address);
				loraif_remove_device(device_address);
				beep_loop(1, 500, 1);
			}
			else{
				LOG_ERROR(TAG, "Event device address invalid.");
			}
		break;
		case LORA_ERR:
			LOG_EVENT(TAG, "LORA_ERR");
		break;
		default:
			LOG_EVENT(TAG, "LoRa other event.");
		break;
	}
	vTaskResume(h_task_loraif_request_device_data);
	vTaskResume(q_task_wifiif_get_device_data);
}
void loraif_request_data_tim_event_handler(tim_channel_t channel, tim_event_t event, void *param){
	if(event == TIM_EVENT_UPDATE){
		BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(e_loraif_request_data, LORAIF_REQUEST_DATA_E_BITS, &pxHigherPriorityTaskWoken);
	}
}

/**
 * WiFi application.
 */
void task_wifiif_start(void *){
	usart1->init(&wifiif_uart_conf);
	usart1->register_event_handler(wifi_uart_handler, NULL);
	usart1->receive_to_idle_start_it(WIFIIF_BUFF_SIZE);

	wifiif_init(wifiif_uart_send_string);
	wifiif_register_command_handler(wifiif_command_handler);

	wifiif_restart();
	restart_wifi:

	LOG_ERROR(TAG, "Stopped LoRa and WiFi interface services.");
	vTaskSuspend(h_task_loraif_receive_process);
	vTaskSuspend(h_task_loraif_request_device_data);
	vTaskSuspend(q_task_wifiif_get_device_data);
	xSemaphoreTake(s_lora_enable_recv, 5);

	beep_loop(1, 20, 1);
	if(!wifiif_state_is_running()) wifiif_restart();
	vTaskDelay(1000);

	wifiif_connect((char *)"NTTAX", (char *)"66668888", (char *)"WIFI_AUTH_WPA2_PSK");
	vTaskDelay(1000);
	uint8_t reconn_num = 0;
	while(wifiif_wificonnected() == false) {
		wifiif_checkconnect();
		vTaskDelay(2000);
		reconn_num++;
		if(reconn_num > 2) goto restart_wifi;
	}
	wifiif_state_running(true);

	vTaskDelay(1000);
	firebase_init((char *)"https://iotnhakho-default-rtdb.asia-southeast1.firebasedatabase.app/", NULL);
	vTaskDelay(1000);

	vTaskResume(h_task_loraif_receive_process);
	vTaskResume(h_task_loraif_request_device_data);
	vTaskResume(q_task_wifiif_get_device_data);
	xSemaphoreGive(s_lora_enable_recv);
	LOG_INFO(TAG, "Started LoRa and WiFi interface services.");

	while(1){
		wifiif_checkconnect();
		if(wifiif_state_is_running() == false || wifiif_wificonnected() == false) {
			LOG_ERROR(TAG, "WiFi module error.");
			goto restart_wifi;
		}
		vTaskDelay(10000);
	}
}
void task_wifiif_get_device_data(void *){
	e_wifiif_get_device_data = xEventGroupCreate();

	tim2->init(&wifiif_get_control_tim);
	tim2->register_event_handler(wifiif_get_device_data_tim_event_handler, NULL);
	tim2->start_it();

	while(1){
		EventBits_t bit = xEventGroupWaitBits(e_wifiif_get_device_data, WIFIIF_GET_DEVICE_DATA_E_BITS, pdTRUE, pdFALSE, 5);
		if(bit == WIFIIF_GET_DEVICE_DATA_E_BITS && !loraif_device_list.empty()) {
			auto device = loraif_device_list.begin();
			__IO uint32_t tick = get_tick();
		    while(device != loraif_device_list.end()) {
		    	if(get_tick() - tick > 1000){
			    	dev_struct_t *dev = select_device_properties((*device)->address);
			    	firebase_get_device_data(dev);
					tick = get_tick();
					++device;
		    	}
		    }
		}
	}
}
void wifiif_uart_send_string(char *str, uint16_t size){
	usart1->transmit((uint8_t *)str, size);
	delay_ms(1);
}
void wifiif_command_handler(wifi_cmd_t cmd, void *param){
	char *resp_data = (char *)param;
	pkt_json_t json;

	switch(cmd){
		case WIFI_RESTART:
			LOG_ERROR(TAG, "wifi module restart.");
		break;
		case WIFI_ISCONNECTED:
			LOG_EVENT(TAG, "WiFi check connect state: %s", resp_data);
			LOG_MEM(TAG, "Free heap = %lu.", sys_get_free_heap_size());
		break;
		case WIFI_ERR:
			LOG_EVENT(TAG, "WIFI error.");
			beep_loop(1, 100, 1);
		break;
		case WIFI_SCAN:
			wifiif_state_running(true);
			LOG_EVENT(TAG, "WiFi scan access point list: %s", resp_data);
		break;
		case WIFI_CONN:
			wifiif_state_running(true);
			wifiif_set_wificonnect_state(true);
			LOG_EVENT(TAG, "WiFi connect state: %s", resp_data);
		break;
		case WIFI_HTTP_CLIENT_RESPONSE:{
			LOG_RET(TAG, "HTTP data: %s", resp_data);
			pkt_json_t json, json_ctrl, json_sett;
			pkt_err_t err;
			uint32_t device_address;

			err = json_get_object(resp_data, &json, (char *)"address");
			if(err != PKT_ERR_OK){
				json_release_object(&json);
				break;
			}
			device_address = strtol(json.value, NULL, 16);
			json_release_object(&json);

			err = json_get_object(resp_data, &json_ctrl, (char *)"control");
			if(err != PKT_ERR_OK){
				json_release_object(&json_ctrl);
				break;
			}
			err = json_get_object(resp_data, &json_sett, (char *)"settings");
			if(err != PKT_ERR_OK){
				json_release_object(&json_sett);
				break;
			}

			char *tx_data;
			asprintf(&tx_data, "{\"control\":%s,\"settings\":%s}", json_ctrl.value, json_sett.value);
			loraif_send_request(device_address, LORA_UPDATE_DATA, tx_data, 1);

			free(tx_data);
			json_release_object(&json_ctrl);
			json_release_object(&json_sett);
		}
		break;
		default:
//			LOG_EVENT(TAG, "WiFi module responded: %s.", resp_data);
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

		wifiif_get_break_data(rxdata);

		usart1->receive_to_idle_start_it(WIFIIF_BUFF_SIZE);
	}
}
void wifiif_get_device_data_tim_event_handler(tim_channel_t channel, tim_event_t event, void *param){
	if(event == TIM_EVENT_UPDATE){
		BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(e_wifiif_get_device_data, WIFIIF_GET_DEVICE_DATA_E_BITS, &pxHigherPriorityTaskWoken);
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
