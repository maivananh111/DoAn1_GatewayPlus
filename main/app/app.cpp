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

#include "system/log.h"


using namespace std;

static const char *TAG = "Device";
#define LOG_LEVEL LOG_DEBUG

char *secret = NULL;
char *data_struct = (char *)"{\"data\":{\"temp\":%.02f,\"humi\":%.02f,\"current\":%.02f,\"time\":\"%s\"}}";
char *ctrl_struct = (char *)"{\"control\":{\"relay1\":%d,\"relay2\":%d,\"relay3\":%d,\"relay4\":%d}}";
char *set1_struct = (char *)"{\"mode\":%d,\"type\":%d,\"max_temp\":%.02f,\"min_temp\":%.02f}";
char *set2_struct = (char *)"{\"time_start\":\"10:25:15\",\"time_stop\":\"11:30:00\"}";
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

static void show_device_list(void){
    for (auto device = device_properties_list.begin(); device != device_properties_list.end(); ++device) {
    	LOG_WARN(TAG, "Device 0x%08x[%s].", (unsigned int)(*device)->prop.address, (*device)->prop.name);
    }
}

static void assign_struct(char ** str, dev_struct_t *dev){
	asprintf(str, full_struct,
			dev->env.temp, dev->env.humi, dev->env.curr, dev->env.time,
			dev->ctrl.relay1, dev->ctrl.relay2, dev->ctrl.relay3, dev->ctrl.relay4,
			dev->prop.address, dev->prop.name,
			dev->sett.mode, dev->sett.type, dev->sett.max_temp, dev->sett.min_temp, dev->sett.time_start, dev->sett.time_stop
	);
}

dev_struct_t *add_device_properties(char *jdata){
	pkt_err_t err;
	pkt_json_t json;

	dev_struct_t *dev_prop = (dev_struct_t *)malloc(sizeof(dev_struct_t));

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
	asprintf(&(dev_prop->env.time), "14:30:00 05/05/23");

	err = json_get_object(jdata, &json, (char *)"addr");
	if(err == PKT_ERR_OK)
		dev_prop->prop.address = strtol(json.value, NULL, 16);
	json_release_object(&json);

	err = json_get_object(jdata, &json, (char *)"name");
	if(err == PKT_ERR_OK)
		asprintf(&(dev_prop->prop.name), "%s", json.value);
	json_release_object(&json);

	device_properties_list.push_back(dev_prop);
	show_device_list();

	return dev_prop;
}

void remove_device_properties(char *jdata){
	pkt_err_t err;
	pkt_json_t json;
	uint32_t rm_addr = 0x00U;

	err = json_get_object(jdata, &json, (char *)"addr");
	if(err == PKT_ERR_OK)
		rm_addr = strtol(json.value, NULL, 16);
	json_release_object(&json);

    if (device_properties_list.empty()) {
    	device_debug((char *)"Device properties list empty", __LINE__, __FUNCTION__);
        return;
    }

    auto device = device_properties_list.begin();
    while (device != device_properties_list.end()) {
        if ((*device)->prop.address == rm_addr) {
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
	free((*device));

	device_properties_list.erase(device);
	show_device_list();
}

void firebase_init(char *url, char *secret_key){
	char *tmp;

	asprintf(&tmp, "{\"url\":\"%s\", \"transport_ssl\":1, \"crt_bundle\":1}", url);
	if(secret_key != NULL) asprintf(&secret, "%s", secret_key);

	wifiif_http_client_new();
	wifiif_http_client_config(tmp);
	wifiif_http_client_init();
	wifiif_http_client_set_header((char *)"Content-Type", (char *)"application/json");

	free(tmp);
}

void firebase_new_device(dev_struct_t *dev){
	char *path, *data;

	if(secret != NULL) asprintf(&path, "/%s/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/.json", dev->prop.name);

	wifiif_http_client_set_url(path);
	wifiif_http_client_set_method((char *)"HTTP_METHOD_PATCH");
	assign_struct(&data, dev);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();

	free(data);
	free(path);
}

void firebase_remove_device(dev_struct_t *dev){
	char *path;

	wifiif_http_client_set_method((char *)"HTTP_METHOD_DELETE");
	wifiif_http_client_set_data((char *)"{}");

	if(secret != NULL) asprintf(&path, "/%s/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/.json", dev->prop.name);
	wifiif_http_client_set_url(path);

	wifiif_http_client_request();
	free(path);
}

void send_envdata_to_firebase (dev_struct_t *dev){
	char *path, *data;

	if(secret != NULL) asprintf(&path, "/%s/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/.json", dev->prop.name);

	asprintf(&data, data_struct, dev->env.temp, dev->env.humi, dev->env.curr, dev->env.time);

	wifiif_http_client_set_method((char *)"HTTP_METHOD_PATCH");
	wifiif_http_client_set_url(path);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();

	free(path);
	free(data);
}

void send_devctrl_to_firebase(dev_struct_t *dev){
	char *path, *data;

	if(secret != NULL) asprintf(&path, "/%s/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/.json", dev->prop.name);

	asprintf(&data, ctrl_struct, dev->ctrl.relay1, dev->ctrl.relay2, dev->ctrl.relay3, dev->ctrl.relay4);

	wifiif_http_client_set_method((char *)"HTTP_METHOD_PATCH");
	wifiif_http_client_set_url(path);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();

	free(path);
	free(data);
}

void get_devctrl(dev_struct_t *dev){
	char *path;

	if(secret != NULL) asprintf(&path, "/%s/control/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/control/.json", dev->prop.name);

	wifiif_http_client_set_method((char *)"HTTP_METHOD_GET");
	wifiif_http_client_set_url(path);
	wifiif_http_client_set_data((char *)"{}");
	wifiif_http_client_request();

	free(path);
}

void get_devsettings(dev_struct_t *dev){
	char *path;

	if(secret != NULL) asprintf(&path, "/%s/settings/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/settings/.json", dev->prop.name);

	wifiif_http_client_set_method((char *)"HTTP_METHOD_GET");
	wifiif_http_client_set_url(path);
	wifiif_http_client_set_data((char *)"{}");
	wifiif_http_client_request();

	free(path);
}


