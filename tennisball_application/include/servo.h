/*
 * servo.h
 *
 *  Created on: Fev 03, 2016
 *  Author:   Victor Santos (viic.santos@gmail.com)
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/time.h>
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void servopulse(int servopin,int myangle);//define a pulse function
int servo_init();
void servoCamera_move(unsigned char value);
void servoCamera_goHome();
void servoCamera_halfHome();
void servo_close();


#ifdef  __cplusplus
}
#endif

#endif /* RELAY_COLETOR_H_ */
