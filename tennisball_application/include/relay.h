/*
 * relay_coletor.h
 *
 *  Created on: Fev 03, 2016
 *  Author:   Victor Santos (viic.santos@gmail.com)
 */

#ifndef RELAY_H_
#define RELAY_H_

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/* para corrigir (provisoriamente) o problema do GPIO5 (PWM) estar em nivel logico alto 
durante o boot, foi utilizando um relay em estado normalmente aberto para cortar a energia do 
motor durante o boot. Na subida do programa a energia eh ligada novamente atraves da funcao
rightMotor_on. */

int init_relay(void);
void coletor_on(void);
void coletor_off(void);
void coletor_toggle(void);
void relay_close(void);
void rightMotor_on(void); //corrigir o problema do GPIO5 (pwm0) estar em 5V no boot
void rightMotor_off(void); 

#ifdef  __cplusplus
}
#endif

#endif /* RELAY_H_ */
