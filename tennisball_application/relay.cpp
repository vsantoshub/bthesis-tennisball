/*
 * relay_coletor.cpp
 *
 *  Created on: Fev 03, 2016
 *      Author: viic.santos@gmail.com
 */

//GPIO-02 (J8 header) used to control collector mechanism
//GPIO-00 (J8 header) used to control right motor on/off
#include "relay.h"


static const char *coletorModePath = "/sys/devices/virtual/misc/gpio/mode/gpio2";
static const char *coletorDataPath = "/sys/devices/virtual/misc/gpio/pin/gpio2";

static const char *motorModePath = "/sys/devices/virtual/misc/gpio/mode/gpio0";
static const char *motorDataPath = "/sys/devices/virtual/misc/gpio/pin/gpio0";

#define OFF 0

//pin handlers
int coletorMode, coletorData;
int motorMode, motorData;

int collector_on = OFF;

int init_relay() {

    coletorMode = open(coletorModePath, O_WRONLY);
    if(coletorMode < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",coletorModePath);
        return 1;
    }
    coletorData = open(coletorDataPath, O_WRONLY);
    if(coletorData < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",coletorDataPath);
        return 1;
    }

    motorMode = open(motorModePath, O_WRONLY);
    if(motorMode < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",motorModePath);
        return 1;
    }
    motorData = open(motorDataPath, O_WRONLY);
    if(motorData < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",motorDataPath);
        return 1;
    }

    // "0" - pin INPUT
    // "1" - pin OUTPUT
    // "8" - pin INPUT with pull-up resistor
    write(coletorMode, "1", 1);
    coletor_off();

    write(motorData, "1", 1);
    rightMotor_on();
    return 0;
}

void coletor_on() {

    // "0" - LOW
    // "1" - HIGH
    write(coletorData, "0", 1);    
}

void coletor_off() { 
    collector_on = OFF;
    DBG(DBG_TRACE, "coletor_stop");
    write(coletorData, "1", 1);
}

void coletor_toggle() {

    collector_on = !collector_on;
    if (collector_on)
        coletor_on();
    else
        coletor_off();
} 

void rightMotor_on() {

    // "0" - LOW
    // "1" - HIGH
    write(motorData, "0", 1);    
}

void rightMotor_off() { 
    collector_on = OFF;
    DBG(DBG_TRACE, "rightMotor_off");
    write(motorData, "1", 1);
}

void relay_close(){
    
    coletor_off(); 
    rightMotor_off();
    close(coletorData);
    close(coletorMode);
    close(motorData);
    close(motorMode);
}
