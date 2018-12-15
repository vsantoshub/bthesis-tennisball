/*
 *  i2c_libs.h
 *  
 *  Created on: Sep 14, 2014
 *  Author:   Victor Santos (viic.santos@gmail.com)
 *      
 */

#ifndef I2C_LIBS_H_
#define I2C_LIBS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int i2c_init(void); 
void i2c_close(void); 
void i2c_test(void ); 
void i2c_write_motor(int value); 
void i2c_write_servo(int value); 
void robot_moveUp(); 
void robot_moveDown();
void robot_moveLeft(); 
void robot_moveRight(); 
void robot_stop(); 
void robot_collector_on(); 
void robot_collector_off(); 
void toggle_collector(); 
void servoCamera_move(unsigned char value); 
void servoCamera_goHome(); 
void servoCamera_halfHome();
void robot_aut_moveUp(char pwm); 
void robot_aut_moveLeft(char pwm); 
void robot_aut_moveRight(char pwm); 

#ifdef __cplusplus
}
#endif

#endif /* I2C_LIBS_H_ */

