import smbus
import time
import sys
#communication frequency = 200kHz, fixed on pcDuino
# 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)
bus = smbus.SMBus(2)
g_collector_on = False

# This is the arduinoServo_address we setup in the Arduino Program
arduinoServo_address = 0x2F

def writeNumber(value):
    try:
        bus.write_byte(arduinoServo_address, value)
        #bus.write_byte_data(arduinoServo_address, 0, value)
        time.sleep(0.1)
    except IOError as e:
        print e
        time.sleep(0.1)
    return -1

def readNumber():
    number = bus.read_byte(arduinoServo_address)
    #number = bus.read_byte_data(arduinoServo_address, 1)
    return number

def led_on():
    writeNumber(7)
    writeNumber(1)
    return -1

def led_off():
    writeNumber(7)
    writeNumber(0)
    return -1

#used for tests only
#22 lines below false
if False:
    while True:
        print('[15]: MOTOR')
        print('[10]: COLLECTOR MOTOR')
        var1 = input("Enter data type:")
#    print 'debug var1', var1
        if var1 != 15 and var1 != 10:
            print("Command not found")
            sys.exit(1)
        if var1 == 15:
            print('MOTOR: [1]-UP, [2]-DOWN, [4]-LEFT, [8]-RIGHT, [16]-STOP')
        elif var1 == 10:
            print('COLLECTOR MOTOR: [1]-ON, [2]-OFF')
        var2 = input("Enter command:")
        if not var2:
            continue
        writeNumber(var1)
        print "pcDuino: Hi Arduino, I sent you ", (var1)
        writeNumber(var2)
        print "pcDuino: Hi Arduino, I sent you ", (var2)

        # sleep one second
        time.sleep(1)

#example getting response from i2c device
#    number = readNumber()
#    print "Arduino: Hey pcDuino, I received a digit msb-like ", number
#    number = readNumber()
#    print "Arduino: Hey pcDuino, I received a digit lsb-like ", number

