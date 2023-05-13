/*
 * gpio.cpp
 *
 *  Created on: Jan 5, 2023
 *      Author: anh
 */
#include "peripheral_config.h"
#include "periph/gpio.h"


/**
 * @fn void gpio_allport_clock_enable(void)
 * @brief Enable all gpio port clock.
 *
 * @pre
 * @post
 */
void gpio_allport_clock_enable(void){
#if defined(GPIOA)
	GPIOA_CLOCKENABLE();
#endif /* defined(GPIOA */
#if defined(GPIOB)
	GPIOB_CLOCKENABLE();
#endif /* defined(GPIOB */
#if defined(GPIOC)
	GPIOC_CLOCKENABLE();
#endif /* defined(GPIOC */
#if defined(GPIOD)
	GPIOD_CLOCKENABLE();
#endif /* defined(GPIOD */
#if defined(GPIOE)
	GPIOE_CLOCKENABLE();
#endif /* defined(GPIOE */
#if defined(GPIOF)
	GPIOF_CLOCKENABLE();
#endif /* defined(GPIOF */
#if defined(GPIOG)
	GPIOG_CLOCKENABLE();
#endif /* defined(GPIOG */
#if defined(GPIOH)
	GPIOH_CLOCKENABLE();
#endif /* defined(GPIOH */
#if defined(GPIOI)
	GPIOI_CLOCKENABLE();
#endif /* defined(GPIOI */
#if defined(GPIOJ)
	GPIOJ_CLOCKENABLE();
#endif /* defined(GPIOJ */
}

/**
 * @fn void gpio_port_clock_enable(GPIO_TypeDef*)
 * @brief Enable gpio port selected clock.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 */
void gpio_port_clock_enable(GPIO_TypeDef *port){
#if defined(GPIOA)
	if(port == GPIOA) {GPIOA_CLOCKENABLE(); return;}
#endif /* defined(GPIOA */
#if defined(GPIOB)
	if(port == GPIOB) {GPIOB_CLOCKENABLE(); return;}
#endif /* defined(GPIOB */
#if defined(GPIOC)
	if(port == GPIOC) {GPIOC_CLOCKENABLE(); return;}
#endif /* defined(GPIOC */
#if defined(GPIOD)
	if(port == GPIOD) {GPIOD_CLOCKENABLE(); return;}
#endif /* defined(GPIOD */
#if defined(GPIOE)
	if(port == GPIOE) {GPIOE_CLOCKENABLE(); return;}
#endif /* defined(GPIOE */
#if defined(GPIOF)
	if(port == GPIOF) {GPIOF_CLOCKENABLE(); return;}
#endif /* defined(GPIOF */
#if defined(GPIOG)
	if(port == GPIOG) {GPIOG_CLOCKENABLE(); return;}
#endif /* defined(GPIOG */
#if defined(GPIOH)
	if(port == GPIOH) {GPIOH_CLOCKENABLE(); return;}
#endif /* defined(GPIOH */
#if defined(GPIOI)
	if(port == GPIOI) {GPIOI_CLOCKENABLE(); return;}
#endif /* defined(GPIO */

}

/**
 * @fn void gpio_config(GPIO_Config_t*)
 * @brief Configuration gpio pin.
 *
 * @pre
 * @post
 * @param conf gpio configuration struct.
 */
void gpio_init(gpio_config_t *conf){
	conf -> port -> MODER &=~ (3U << (conf -> pin*2));
	conf -> port -> MODER |= (conf -> direction << (conf -> pin*2));

	conf -> port -> OTYPER &=~ (1U << conf -> pin);
	conf -> port -> OTYPER |= (conf -> outputtype << conf -> pin);

	conf -> port -> OSPEEDR &=~ (3U << (conf -> pin*2));
	conf -> port -> OSPEEDR |= (conf -> outputspeed << (conf -> pin*2));

	conf -> port -> PUPDR &=~ (3U << (conf -> pin*2));
	conf -> port -> PUPDR |= (conf -> pullresister << (conf -> pin*2));

	if(conf -> direction == GPIO_AlternateFunction){
		if(conf -> pin < 8){
			conf -> port -> AFR[0] &=~ (0x0FU << (conf -> pin*4));
			conf -> port -> AFR[0] |=  (conf -> function  << (conf -> pin*4));
		}
		else{
			conf -> port -> AFR[1] &=~ (0x0FU << ((conf -> pin-8)*4));
			conf -> port -> AFR[1] |=  (conf -> function  << ((conf -> pin-8)*4));
		}
	}
}

/**
 * @fn void gpio_deinit(GPIO_TypeDef*, uint16_t)
 * @brief
 *
 * @pre
 * @post
 * @param port
 * @param pin
 */
void gpio_deinit(GPIO_TypeDef *port, uint16_t pin){
	port -> MODER &=~ (3U << (pin*2));
	if((port == GPIOA && pin >= 13U) || (port == GPIOB && (pin == 3U || pin == 4U))) port -> MODER |= (2U << (pin*2));

	port -> OTYPER &=~ (1U << pin);

	port -> OSPEEDR &=~ (3U << (pin*2));
	if((port == GPIOA && pin == 13U) || (port == GPIOB && pin == 3U)) port -> OSPEEDR |= (3U << (pin*2));

	port -> PUPDR &=~ (3U << (pin*2));
	if(port == GPIOA){
		if(pin == 13 || pin == 15) port -> PUPDR |= (1U << (pin*2));
		else if(pin == 14) port -> PUPDR |= (2U << (pin*2));
	}
	if(port == GPIOB && pin == 4) port -> PUPDR |= (1U << (pin*2));

	if(pin < 8) port -> AFR[0] &=~ (0x0FU << (pin*4));
	else port -> AFR[1] &=~ (0x0FU << ((pin-8)*4));

	port -> BSRR |= 0xFFFF0000U;
}

/**
 * @fn void gpio_set_mode(GPIO_TypeDef*, uint16_t, GPIOMode_t)
 * @brief Set mode for the gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 * @param mode gpio pin mode.
 */
void gpio_set_mode(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode){
	__IO uint32_t tmpreg = 0U;
	/* *************************************************** */
	if(mode <=  GPIO_INPUT_PULLDOWN){ // GPIO_INPUT.
		port -> MODER &=~ (3U << (pin * 2));

		tmpreg = port -> PUPDR;
		tmpreg &=~ (3U << (pin * 2));
		switch(mode){
			case GPIO_INPUT_PULLUP:
				tmpreg |=  (1U << (pin * 2));
			break;
			case GPIO_INPUT_PULLDOWN:
				tmpreg |=  (2U << (pin * 2));
			break;
			default:
			break;
		}
		port -> PUPDR |=tmpreg;
	}
	/* *************************************************** */
	else if(mode >= GPIO_OUTPUT_OPENDRAIN && mode <=  GPIO_OUTPUT_PUSHPULL_PULLDOWN){ // GPIO_OUTPUT.
		port -> MODER &=~ (3U << (pin * 2));
		port -> MODER |=  (1U << (pin * 2));

		if(mode >= GPIO_OUTPUT_OPENDRAIN && mode <=  GPIO_OUTPUT_OPENDRAIN_PULLDOWN) port -> OTYPER |= (1U << pin);
		else port -> OTYPER &=~ (1U << pin);

		port -> OSPEEDR &=~ (3U << (pin * 2));
		port -> OSPEEDR |=  (GPIO_OUTPUTSPEED_DEFAULT << (pin * 2));

		tmpreg = port -> PUPDR;
		tmpreg &=~ (3U << (pin * 2));
		if(mode == GPIO_OUTPUT_OPENDRAIN_PULLUP || mode == GPIO_OUTPUT_PUSHPULL_PULLUP) tmpreg |=  (1U << (pin * 2));
		else if(mode == GPIO_OUTPUT_OPENDRAIN_PULLDOWN || mode == GPIO_OUTPUT_PUSHPULL_PULLDOWN) tmpreg |=  (2U << (pin * 2));
		port -> PUPDR |=tmpreg;

	}
	/* *************************************************** */
	else{ // GPIO_ANALOG.
		port -> MODER |= (3U << (pin * 2));
	}
}

/**
 * @fn void gpio_set_alternatefunction(GPIO_TypeDef*, uint16_t, GPIOAlternateFunction_t)
 * @brief Set altẻnate function for the gpio pin (peripheral).
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 * @param function Alternate function for gpio pin.
 */
void gpio_set_alternatefunction(GPIO_TypeDef *port, uint16_t pin, gpio_alternatefunction_t function){
	port -> MODER &=~ (3U << (pin*2));
	port -> MODER |=  (2U << (pin*2));

	port -> OTYPER &=~ (1U<<pin);

	port -> OSPEEDR &=~ (3U << (pin * 2));
	port -> OSPEEDR |=  (GPIO_OUTPUTSPEED_DEFAULT << (pin * 2));

	if(pin < 8){
		port -> AFR[0] &=~ (0x0FU << (pin*4));
		port -> AFR[0] |=  (function  << (pin*4));
	}
	else{
		port -> AFR[1] &=~ (0x0FU << ((pin-8)*4));
		port -> AFR[1] |=  (function  << ((pin-8)*4));
	}
}

/**
 * @fn void gpio_set_alternatefunction_type(GPIO_TypeDef*, uint16_t, GPIOMode_t)
 * @brief Set gpio type for alternate function of gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 * @param mode gpio mode(type).
 */
void gpio_set_alternatefunction_type(GPIO_TypeDef *port, uint16_t pin, gpio_mode_t mode){
	if(mode == GPIO_OUTPUT_OPENDRAIN) port -> OTYPER |= (1U<<pin);
	else if(mode == GPIO_OUTPUT_PUSHPULL) port -> OTYPER &=~ (1U<<pin);
}

/**
 * @fn void gpio_set_pullup(GPIO_TypeDef*, uint16_t)
 * @brief Set pullup resistor for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 */
void gpio_set_pullup(GPIO_TypeDef *port, uint16_t pin){
	port ->PUPDR &=~ (3U << (pin*2));
	port ->PUPDR |= (1U << (pin*2));
}

/**
 * @fn void gpio_set_pulldown(GPIO_TypeDef*, uint16_t)
 * @brief Set pulldown resistor for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 */
void gpio_set_pulldown(GPIO_TypeDef *port, uint16_t pin){
	port ->PUPDR &=~ (3U << (pin*2));
	port ->PUPDR |= (2U << (pin*2));
}

/**
 * @fn void gpio_set(GPIO_TypeDef*, uint16_t)
 * @brief Set high level for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 */
void gpio_set(GPIO_TypeDef *port, uint16_t pin){
	port -> BSRR |= (1 << pin);
}

/**
 * @fn void gpio_reset(GPIO_TypeDef*, uint16_t)
 * @brief Set low level for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 */
void gpio_reset(GPIO_TypeDef *port, uint16_t pin){
	port -> BSRR |= (1 << (pin + 16));
}

/**
 * @fn void gpio_toggle(GPIO_TypeDef*, uint16_t)
 * @brief Toggle level for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 */
void gpio_toggle(GPIO_TypeDef *port, uint16_t pin){
	(port -> ODR & (1<<pin))? gpio_reset(port, pin) : gpio_set(port, pin);
}

/**
 * @fn void gpio_set_level(GPIO_TypeDef*, uint16_t, int)
 * @brief Set level for gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 * @param level level for gpio pin.
 */
void gpio_set_level(GPIO_TypeDef *port, uint16_t pin, int level){
	(level == 0)? gpio_reset(port, pin) :  gpio_set(port, pin);
}

/**
 * @fn int gpio_get_level(GPIO_TypeDef*, uint16_t)
 * @brief Get level of gpio pin.
 *
 * @pre
 * @post
 * @param port gpio port selected.
 * @param pin  gpio pin selected.
 * @return Level of gpio pin.
 */
int gpio_get_level(GPIO_TypeDef *port, uint16_t pin){
	return (port -> IDR >> pin) & 1UL;
}





















