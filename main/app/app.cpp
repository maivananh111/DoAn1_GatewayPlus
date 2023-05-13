/*
 * app.cpp
 *
 *  Created on: May 2, 2023
 *      Author: anh
 */

#include "app.h"

#include "wifiif/wifiif.h"


char *secret = NULL;
char *data_struct = (char *)"{\"data\":{\"temp\":%.02f,\"humi\":%.02f,\"current\":%.02f,\"time\":\"%s\"}}";
char *ctrl_struct = (char *)"{\"control\":{\"relay1\":%d,\"relay2\":%d,\"relay3\":%d,\"relay4\":%d}}";
char *set1_struct = (char *)"{\"mode\":%d,\"type\":%d,\"max_temp\":%.02f,\"min_temp\":%.02f}";
char *set2_struct = (char *)"{\"time_start\":\"10:25:15\",\"time_stop\":\"11:30:00\"}";
char *prop_struct = (char *)"{\"properties\":{\"address\":\"0x%08x\",\"name\":\"%s\"}}";



void firebase_init(char *url, char *secret_key){
	char *tmp;

	asprintf(&tmp, "{\"url\":\"%s\"}", url);
	if(secret_key != NULL) asprintf(&secret, "%s", secret_key);

	wifiif_http_client_new();
	wifiif_http_client_config(tmp);
	wifiif_http_client_config((char *)"{\"transport_ssl\":1}");
	wifiif_http_client_config((char *)"{\"crt_bundle\":1}");
	wifiif_http_client_init();

	wifiif_http_client_set_header((char *)"Content-Type", (char *)"application/json");

	free(tmp);
}

void firebase_new_device(dev_struct_t *dev){
	char *path, *data;

	wifiif_http_client_set_method((char *)"HTTP_METHOD_PATCH");

	if(secret != NULL) asprintf(&path, "/%s/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/.json", dev->prop.name);
	wifiif_http_client_set_url(path);

	asprintf(&data, data_struct, dev->env.temp, dev->env.humi, dev->env.curr, dev->env.time);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();
	free(data);

	asprintf(&data, ctrl_struct, dev->ctrl.relay1, dev->ctrl.relay2, dev->ctrl.relay3, dev->ctrl.relay4);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();
	free(data);

	asprintf(&data, prop_struct, dev->prop.address, dev->prop.name);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();
	free(data);

	free(path);
	if(secret != NULL) asprintf(&path, "/%s/settings/.json?auth=%s", dev->prop.name, secret);
	else asprintf(&path, "/%s/settings/.json", dev->prop.name);

	wifiif_http_client_set_url(path);
	asprintf(&data, set1_struct, dev->sett.mode, dev->sett.type, dev->sett.max_temp, dev->sett.min_temp);
	wifiif_http_client_set_data(data);
	wifiif_http_client_request();
	free(data);
	asprintf(&data, set2_struct, dev->sett.time_start, dev->sett.time_stop);
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


