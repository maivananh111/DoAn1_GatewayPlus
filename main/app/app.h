/*
 * app.h
 *
 *  Created on: May 2, 2023
 *      Author: anh
 */

#ifndef APP_APP_H_
#define APP_APP_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

extern char *full_struct;

struct dev_env_t{
	float temp = 0.0;
	float humi = 0.0;
	float curr = 0.0;
	char *time = NULL;
};
struct dev_ctrl_t{
	uint8_t relay1 = 0;
	uint8_t relay2 = 0;
	uint8_t relay3 = 0;
	uint8_t relay4 = 0;
};
struct dev_set_t{
	uint8_t mode = 0;
	uint8_t type = 0;
	float max_temp = 0.0;
	float min_temp = 0.0;
	char *time_start = NULL;
	char *time_stop = NULL;
};
struct dev_prop_t{
	uint32_t address = 0x00;
	char *name = NULL;
};

typedef struct {
	dev_env_t env;
	dev_ctrl_t ctrl;
	dev_set_t sett;
	dev_prop_t prop;
} dev_struct_t;

dev_struct_t *add_device_properties(uint32_t device_address, char *jdata);
void remove_device_properties(uint32_t device_address);
dev_struct_t *select_device_properties(uint32_t device_address);

void firebase_init(char *url, char *secret_key);

void firebase_new_device(dev_struct_t *);
void firebase_remove_device(dev_struct_t *);

void send_envdata_to_firebase(uint32_t address, char *jdata);
void send_devctrl_to_firebase(dev_struct_t *);
void firebase_get_device_data(dev_struct_t *);


#ifdef __cplusplus
}
#endif

#endif /* APP_APP_H_ */
