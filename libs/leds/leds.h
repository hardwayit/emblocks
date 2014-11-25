/*
 * leds.h
 *
 *  Created on: 22.11.2014
 *      Author: ALev
 */

#ifndef LEDS_H_
#define LEDS_H_

#define LED_RED 0
#define LED_GREEN 1

void led_init(void);
void led_set(unsigned char led, char value);


#endif /* LEDS_H_ */
