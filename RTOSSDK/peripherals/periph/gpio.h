/*
 * gpio.h
 *
 *  Created on: Jan 5, 2023
 *      Author: anh
 */

#ifndef PERIPH_GPIO_H_
#define PERIPH_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"
#include st_header
#include "stdio.h"


typedef enum{
	GPIO_Input = 0,
	GPIO_Output,
	GPIO_AlternateFunction,
} gpio_derection_t;

typedef enum{
	GPIO_PushPull = 0,
	GPIO_OpenDrain,
} gpio_outputtype_t;

typedef enum{
	GPIO_Speed_Low = 0,
	GPIO_Speed_Medium,
	GPIO_Speed_High,
	GPIO_Speed_VeryHigh,
} gpio_outputspeed_t;

typedef enum{
	GPIO_NoPull = 0,
	GPIO_PullUp,
	GPIO_PullDown,
} gpio_pullresistor_t;

typedef enum{
	GPIO_INPUT = 0,
	GPIO_INPUT_PULLUP,
	GPIO_INPUT_PULLDOWN,

	GPIO_OUTPUT_OPENDRAIN,
	GPIO_OUTPUT_OPENDRAIN_PULLUP,
	GPIO_OUTPUT_OPENDRAIN_PULLDOWN,
	GPIO_OUTPUT_PUSHPULL,
	GPIO_OUTPUT_PUSHPULL_PULLUP,
	GPIO_OUTPUT_PUSHPULL_PULLDOWN,

	GPIO_ANALOG,
} gpio_mode_t;

typedef enum{
	AF0_SYSTEM = 0,
	AF1_TIM1_2,
	AF2_TIM3_5,
	AF3_TIM8_11,
	AF4_I2C1_3,
#if defined(SPI4) & defined(SPI5) & defined(SPI6)
	AF5_SPI1_6,
#else
	AF5_SPI1_2,
#endif /* defined(SPI4) & defined(SPI5) & defined(SPI6) */
#if defined(SAI1) & defined(SPI2) & defined(SPI3)
	AF6_SPI2_3_SAI1,
#else
	AF6_SPI3,
#endif /* defined(SAI1) & defined(SPI2) & defined(SPI3) */
	AF7_USART1_3,
#if defined(UART7) && defined(UART8)
	AF8_USART4_8,
#else
	AF8_USART4_6,
#endif /* defined(UART7) && defined(UART8) */

#if defined(LTDC)
	AF9_CAN1_2_LTDC_TIM12_14, // ONLY AFRL.
#else
	AF9_CAN1_2_TIM12_14,
#endif /* defined(LTDC) */
	AF10_USB,
	AF11_ETH,
	AF12_FSMC_SDIO_USB,
	AF13_DCMI,
#if defined(LTDC)
	AF14_LTDC,
#else
	AF14,
#endif /* defined(LTDC) */
	AF15_EVENTOUT,
} gpio_alternatefunction_t;

typedef struct gpio_config{
	GPIO_TypeDef *port = GPIOA;
	uint16_t pin = 0U;
	gpio_derection_t direction = GPIO_Input;
	gpio_outputtype_t outputtype = GPIO_PushPull;
	gpio_outputspeed_t outputspeed = GPIO_Speed_Low;
	gpio_pullresistor_t pullresister = GPIO_NoPull;
	gpio_alternatefunction_t function = AF0_SYSTEM;
} gpio_config_t;


void gpio_allport_clock_enable(void);
void gpio_port_clock_enable(GPIO_TypeDef *port);

void gpio_init(gpio_config_t *conf);
void gpio_deinit(GPIO_TypeDef *port, uint16_t pin);

void gpio_set_mode(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode);
void gpio_set_alternatefunction(GPIO_TypeDef *port, uint16_t pin, gpio_alternatefunction_t function);
void gpio_set_alternatefunction_type(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode);

void gpio_set_pullup(GPIO_TypeDef *port, uint16_t pin);
void gpio_set_pulldown(GPIO_TypeDef *port, uint16_t pin);

void gpio_set(GPIO_TypeDef *port, uint16_t pin);
void gpio_reset(GPIO_TypeDef *port, uint16_t pin);
void gpio_toggle(GPIO_TypeDef *port, uint16_t pin);

void gpio_set_level(GPIO_TypeDef *port, uint16_t pin, int level);
int gpio_get_level(GPIO_TypeDef *port, uint16_t pin);

#if defined(GPIOA)
#define GPIOA_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN
#endif /* defined(GPIOA */
#if defined(GPIOB)
#define GPIOB_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN
#endif /* defined(GPIOB */
#if defined(GPIOC)
#define GPIOC_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOCEN
#endif /* defined(GPIOC */
#if defined(GPIOD)
#define GPIOD_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIODEN
#endif /* defined(GPIOD */
#if defined(GPIOE)
#define GPIOE_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOEEN
#endif /* defined(GPIOE */
#if defined(GPIOF)
#define GPIOF_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOFEN
#endif /* defined(GPIOF */
#if defined(GPIOG)
#define GPIOG_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOGEN
#endif /* defined(GPIOG */
#if defined(GPIOH)
#define GPIOH_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOHEN
#endif /* defined(GPIOH */
#if defined(GPIOI)
#define GPIOI_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOIEN
#endif /* defined(GPIOI */
#if defined(GPIOJ)
#define GPIOJ_CLOCKENABLE() RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOJEN
#endif /* defined(GPIOJ */

#ifdef __cplusplus
}
#endif


#endif /* PERIPH_GPIO_H_ */
