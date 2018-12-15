/*
 * monster_shield.h
 *
 *  Created on: Fev 03, 2016
 *  Author:   Victor Santos (viic.santos@gmail.com)
 */

#ifndef MONSTER_SHIELD_H_
#define MONSTER_SHIELD_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRAKEVCC 	0
#define CW 			1
#define CCW 		2
#define BRAKEGND 	3
#define CS_THRESHOLD 100   // Definição da corrente de segurança (Consulte: "1.3) Monster Shield Exemplo").

/* IMPORTANT !!!
If the ADC functions need to be used, changes in Voltage Translator board need to be done
in order to acquire 4096 (12 bit) data in ADC channels. The changes are relate to the resistors relation in ADC channels.
EX: 12K and 24K, so when the voltage divider is read at 24k resistance, the ADC channel will read a voltage of 3.3V
*/

int init_monster_shield(void);
void monster_shield_close(void);

int ADC_init(void);
int PWM_init(void);
void motorGo(uint8_t motor, uint8_t direction, char * pwm);
void motorOff(int motor);
float current_measure(int motor);
int readADC(FILE *adc_fd);

void move_forward();
void move_backward();
void move_left();
void move_right();
void move_stop();
void autonomous_forward(char *pwm);
void autonomous_left(char *pwm);
void autonomous_right(char *pwm);


#ifdef  __cplusplus
}
#endif

#endif /* MONSTER_SHIELD_H_ */
