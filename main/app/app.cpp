/*
 * app.cpp
 *
 *  Created on: May 2, 2023
 *      Author: anh
 */

#include "app.h"

#include "wifiif/wifiif.h"
#include "parse_packet/parse_packet.h"

#include "list"
#include "string.h"

#include "system/log.h"
#include "FreeRTOS.h"
#include "task.h"


using namespace std;

static const char *TAG = "Device";
#define LOG_LEVEL LOG_DEBUG


char *data_struct = (char *)"{\"data\":{\"temp\":%.02f,\"humi\":%.02f,\"current\":%.02f,\"time\":\"%s\"}}";
char *ctrl_struct = (char *)"{\"control\":{\"relay1\":%d,\"relay2\":%d,\"relay3\":%d,\"relay4\":%d}}";
char *sett_struct = (char *)"{\"settings\":{\"mode\":%d,\"type\":%d,\"max_temp\":%.02f,\"min_temp\":%.02f,\"time_start\":\"%s\",\"time_stop\":\"%s\"}}";
char *prop_struct = (char *)"{\"properties\":{\"address\":\"0x%08x\",\"name\":\"%s\"}}";

char *full_struct = (char *)"{\"data\":{\"temp\":%.02f,\"humi\":%.02f,\"current\":%.02f,\"time\":\"%s\"},"
						     "\"control\":{\"relay1\":%d,\"relay2\":%d,\"relay3\":%d,\"relay4\":%d},"
							 "\"properties\":{\"address\":\"0x%08x\",\"name\":\"%s\"},"
							 "\"settings\":{\"mode\":%d,\"type\":%d,\"max_temp\":%.02f,\"min_temp\":%.02f,\"time_start\":\"%s\",\"time_stop\":\"%s\"}"
							 "}";

list<dev_struct_t *> device_properties_list;

static void device_debug(char *str, int line, const char *func){
#if ENABLE_COMPONENT_LORAIF_DEBUG
	LOG_LEVEL(TAG, "%s, Line: %d Function: %s", str, line, func);
#endif /* ENABLE_COMPONENT_LORAIF_DEBUG */
}

static void assign_struct(char ** str, dev_struct_t *dev){
	asprintf(str, full_struct,
			dev->env.temp, dev->env.humi, dev->env.curr, dev->env.time,
			dev->ctrl.relay1, dev->ctrl.relay2, dev->ctrl.relay3, dev->ctrl.relay4,
			dev->prop.address, dev->prop.name,
			dev->sett.mode, dev->sett.type, dev->sett.max_temp, dev->sett.min_temp, dev->sett.time_start, dev->sett.time_stop
	);
}

dev_struct_t *add_device_properties(uint32_t device_address, char *jdata){
	pkt_err_t err;
	pkt_json_t json;

	dev_struct_t *dev_prop = (dev_struct_t *)malloc(sizeof(dev_struct_t));

	dev_prop->prop.address = device_address;

	dev_prop->env.temp = 0.0;
	dev_prop->env.humi = 0.0;
	dev_prop->env.curr = 0.0;
	dev_prop->ctrl.relay1 = 0;
	dev_prop->ctrl.relay2 = 0;
	dev_prop->ctrl.relay3 = 0;
	dev_prop->ctrl.relay4 = 0;
	dev_prop->sett.mode = 0;
	dev_prop->sett.type = 0;
	dev_prop->sett.max_temp = 0.0;
	dev_prop->sett.min_temp = 0.0;
	asprintf(&(dev_prop->sett.time_start), "00:00:00");
	asprintf(&(dev_prop->sett.time_stop), "00:00:00");
	asprintf(&(dev_prop->env.time), "14:30:00 05/05/23 thu 2");

	err = json_get_object(jdata, &json, (char *)"name");
	if(err == PKT_ERR_OK)
		asprintf(&(dev_prop->prop.name), "%s", json.value);
	json_release_object(&json);

	device_properties_list.push_back(dev_prop);

	return dev_prop;
}

void remove_device_properties(uint32_t device_address){
    if (device_properties_list.empty()) {
    	device_debug((char *)"Device properties list empty", __LINE__, __FUNCTION__);
        return;
    }

    auto device = device_properties_list.begin();
    while (device != device_properties_list.end()) {
        if ((*device)->prop.address == device_address) {
            break;
        }
        ++device;
    }

    if (device == device_properties_list.end()) {
    	device_debug((char *)"This device not available in device list", __LINE__, __FUNCTION__);
        return;
    }

	if((*device) == NULL) return;
	if((*device)->prop.name != NULL) free((*device)->prop.name);
	if((*device)->sett.time_start != NULL) free((*device)->sett.time_start);
	if((*device)->sett.time_stop != NULL) free((*device)->sett.time_stop);
	if((*device)->env.time != NULL) free((*device)->env.time);

	device_properties_list.erase(device);
	if((*device) != NULL) free((*device));
}

dev_struct_t *select_device_properties(uint32_t device_address){
    if (device_properties_list.empty()) {
    	device_debug((char *)"Device properties list empty", __LINE__, __FUNCTION__);
        return NULL;
    }

    auto device = device_properties_list.begin();
    while (device != device_properties_list.end()) {
        if ((*device)->prop.address == device_address) {
            break;
        }
        ++device;
    }

    if (device == device_properties_list.end()) {
    	device_debug((char *)"This device not available in device list", __LINE__, __FUNCTION__);
        return NULL;
    }

    return (*device);
}

void firebase_init(char *url, char *secret_key){
	wifiif_firebase_init(url, secret_key);
	vTaskDelay(50);
}

void firebase_new_device(dev_struct_t *dev){
	char *data;

	assign_struct(&data, dev);
	wifiif_firebase_set_data(dev->prop.name, data);
	vTaskDelay(50);

	free(data);
}

void firebase_remove_device(dev_struct_t *dev){
	if(dev == NULL || dev->prop.name == NULL) return;

	wifiif_firebase_remove_data(dev->prop.name);
	vTaskDelay(50);
}

void send_data_to_firebase(uint32_t address, char *jdata){
	char *data = NULL;
	pkt_json_t json;

	dev_struct_t *dev = select_device_properties(address);
	if(json_get_object(jdata, &json, (char *)"temp") == PKT_ERR_OK)
		dev->env.temp = atof(json.value);
	json_release_object(&json);
	if(json_get_object(jdata, &json, (char *)"humi") == PKT_ERR_OK)
		dev->env.humi = atof(json.value);
	json_release_object(&json);
	if(json_get_object(jdata, &json, (char *)"current") == PKT_ERR_OK)
		dev->env.curr = atof(json.value);
	json_release_object(&json);
	if(json_get_object(jdata, &json, (char *)"time") == PKT_ERR_OK)
		memcpy(dev->env.time, json.value, strlen(json.value));
	json_release_object(&json);

	assign_struct(&data, dev);

	wifiif_firebase_set_data(dev->prop.name, data);
	vTaskDelay(50);

	free(data);
}


void firebase_get_device_data(uint32_t address){
	dev_struct_t *dev = select_device_properties(address);

	wifiif_firebase_get_data(dev->prop.name);
	vTaskDelay(50);
}


