#!/usr/bin/env python

import time, os

## For simplicity's sake, we'll create a string for our paths.
GPIO_MODE_PATH= os.path.normpath('/sys/devices/virtual/misc/gpio/mode/')
GPIO_PIN_PATH=os.path.normpath('/sys/devices/virtual/misc/gpio/pin/')
GPIO_FILENAME="gpio"

## Create a few strings for file I/O equivalence
HIGH = "1"
LOW =  "0"
INPUT = "0"
OUTPUT = "1"
INPUT_PU = "8"


def gpio_out(gpio_number, gpio_state): 

	## create a couple of empty arrays to store the pointers for our files
	pinMode = os.path.join(GPIO_MODE_PATH, gpio_number)
	pinData = os.path.join(GPIO_PIN_PATH, gpio_number)

	file = open(pinMode, 'r+')  ## open the file in r/w mode
	file.write(OUTPUT)      ## set the mode of the pin
	file.close()            ## IMPORTANT- must close file to make changes!
	file = open(pinData, 'r+')
	file.write(gpio_state)
	file.close()

def gpio_in(gpio_number, gpio_state): 

	## create a couple of empty arrays to store the pointers for our files
	pinMode = os.path.join(GPIO_MODE_PATH, gpio_number)
	pinData = os.path.join(GPIO_PIN_PATH, gpio_number)

	file = open(pinMode, 'r+')  ## open the file in r/w mode
	file.write(OUTPUT)      ## set the mode of the pin
	file.close()            ## IMPORTANT- must close file to make changes!
	file = open(pinData, 'r+')
	file.write(gpio_state)
	file.close()

#gpio_out('gpio1', '1')