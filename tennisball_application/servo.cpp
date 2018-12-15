/* Author:   Victor Santos (viic.santos@gmail.com)
 * servo test program
 */
#include "servo.h"


//servo attached to pin 3
static const char *servoModePath = "/sys/devices/virtual/misc/gpio/mode/gpio3";
static const char *servoDataPath = "/sys/devices/virtual/misc/gpio/pin/gpio3";

#define INIT_POSITION 60
#define HALF_HOME 30

//pin handlers
int servopin = 3;
int servopinMode, servopinData;

int myangle;
int pulsewidth;
int val;
int i=0;

//under construct
//can refine with timer interrupt, but need more implement 
void delay(unsigned long ms) {
    usleep(ms*1000);
}

//under construct
//can refine with timer interrupt, but need more implement 
void delayMicroseconds(unsigned int us) {
    usleep(us);
}

void servopulse(int servopin,int myangle) {//define a pulse function

    pulsewidth=(myangle*11)+500;//translate angle to a pulse width value between 500-2480
    write(servopinData, "1", 1);//pull the interface signal level to high
    delayMicroseconds(pulsewidth);//delay in microseconds
    write(servopinData, "0", 1);//pull the interface signal level to low
    delay(20-pulsewidth/1000);
}


int servo_init() {
    servopinMode = open(servoModePath, O_WRONLY);
    if(servopinMode < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",servoModePath);
        return 1;
    }
    servopinData = open(servoDataPath, O_WRONLY);
    if(servopinData < 0) {
        DBG(DBG_ERROR,"Unable to open mode file: %s\n",servoDataPath);
        return 1;
    }

    // "0" - pin INPUT
    // "1" - pin OUTPUT
    // "8" - pin INPUT with pull-up resistor
    write(servopinMode, "1", 1); 
    return 0;  
}


void servoCamera_move(unsigned char value) { //value in degree
    int i = 0;
    if (value < 0) value = 0;
    if (value > 180) value = 180;
    for(i=0;i<=30;i++) {//wait enough time so that the servo can rotate to the specified angle
        servopulse(servopin,value);
    }   
}


void servoCamera_goHome() {
    int i = 0;
    for(i=0;i<=50;i++) {
        servopulse(servopin,INIT_POSITION);
    }
}

void servoCamera_halfHome() {
    int i = 0;
    for(i=0;i<=50;i++) {
        servopulse(servopin,HALF_HOME);
    }
}

void servo_close(){
    
    servoCamera_goHome(); 
    close(servopinMode);
    close(servopinData);
}

//teste
#if 0 
void main() {
    servo_init();
    while (1) {
        printf("Enter with a angle value (0-180):\n");
        scanf("%d", &val);
        if ((val < 0) || (val > 180)) {
            printf("Wrong value! Enter with a angle value (0-180):\n");
            continue; 
        }
        else {
            for(i=0;i<=50;i++)//wait enough time so that the servo can rotate to the specified angle
            {
                servopulse(servopin,val);//call the pulse function
            }
            for(i=0;i<=50;i++)//wait enough time so that the servo can rotate to the specified angle
            {
                servopulse(servopin,0);//call the pulse function
            }
        }
    }
}
#endif
