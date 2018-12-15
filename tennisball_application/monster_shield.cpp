/*
 * monster_shield.cpp
 *
 *  Author:   Victor Santos (viic.santos@gmail.com)
 */

#include "monster_shield.h"

/*variables to filtering ADC results*/
/* Number of samples I read from ADC to give some result*/
#define N_SAMPLES 8
#define N_SHIFTS  3		//to avoid divisions I take 8 samples and I do three shifts left to divide result by 8 or 2^3

#define PWM_FREQ 20000 //20kHz

//PWM0 -- RIGHT MOTOR
static const char *pwm0enablePath = "/sys/devices/virtual/misc/pwmtimer/enable/pwm5";
static const char *pwm0freqPath = "/sys/devices/virtual/misc/pwmtimer/freq/pwm5";
static const char *pwm0levelPath = "/sys/devices/virtual/misc/pwmtimer/level/pwm5";
//PWM0 GPIO5 (J8 header)
static const char *pwm0ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio5";

//PWM1 -- LEFT MOTOR
static const char *pwm1enablePath = "/sys/devices/virtual/misc/pwmtimer/enable/pwm6";
static const char *pwm1freqPath = "/sys/devices/virtual/misc/pwmtimer/freq/pwm6";
static const char *pwm1levelPath = "/sys/devices/virtual/misc/pwmtimer/level/pwm6";
//PWM1 GPIO6 (J8 header)
static const char *pwm1ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio6";

//Clockwise direction of Motor 0 (A1)
static const char *a1ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio7";
static const char *a1DataPath = "/sys/devices/virtual/misc/gpio/pin/gpio7";

//Counterclockwise direction of Motor 0 (B1)
static const char *b1ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio8";
static const char *b1DataPath = "/sys/devices/virtual/misc/gpio/pin/gpio8";

//Clockwise direction of Motor 1 (A2)
static const char *a2ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio4";
static const char *a2DataPath = "/sys/devices/virtual/misc/gpio/pin/gpio4";

//Counterclockwise direction of Motor 1 (B2)
static const char *b2ModePath = "/sys/devices/virtual/misc/gpio/mode/gpio9";
static const char *b2DataPath = "/sys/devices/virtual/misc/gpio/pin/gpio9";


//ADC
static const char *adc_cs0Path = "/proc/adc2";
static const char *adc_cs1Path = "/proc/adc3";

//pwm file descriptors
int pwm0enable, pwm0freq, pwm0level, pwm0mode;
int pwm1enable, pwm1freq, pwm1level, pwm1mode;

//gpio file descriptors
int a1mode, a1data, b1mode, b1data, a2mode, a2data, b2mode, b2data;
 
//adc file descriptors
int adc_cs0, adc_cs1;

char scratch[128]; //scratch buffer

int init_monster_shield(void) {
	int ret;

	//gpio config
	a1mode = open(a1ModePath, O_RDWR );
	if (a1mode < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",a1ModePath);
        return 1;
    }
    a1data = open(a1DataPath, O_RDWR );
    if (a1data < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",a1DataPath);
        return 1;
    }
    b1mode = open(b1ModePath, O_RDWR );
    if (b1mode < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",b1ModePath);
        return 1;
    }
    b1data = open(b1DataPath, O_RDWR );
	if (b1data < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",b1DataPath);
        return 1;
    }
	a2mode = open(a2ModePath, O_RDWR );
    if (a2mode < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",a2ModePath);
        return 1;
    }
    a2data = open(a2DataPath, O_RDWR );
    if (a2data < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",a2DataPath);
        return 1;
    }
    b2mode = open(b2ModePath, O_RDWR );
    if (b2mode < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",b2ModePath);
        return 1;
    }
    b2data = open(b2DataPath, O_RDWR );
	if (b2data < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",b2DataPath);
        return 1;
    }

    // "0" - pin INPUT
    // "1" - pin OUTPUT
    // "8" - pin INPUT with pull-up resistor
    write(a1mode, "1", 1);
    write(b1mode, "1", 1);
    write(a2mode, "1", 1);
    write(b2mode, "1", 1);

    write(a1data, "0", 1);
    write(b1data, "0", 1);
    write(a2data, "0", 1);
    write(b2data, "0", 1);

	ret = ADC_init();
	if (ret < 0) return 1;
	ret = PWM_init();
	if (ret < 0) return 1;
	return 0;
}

/*************************************************************************
 Function: PWM_init
 Purpose: used to generate PWM to the motor wheels
 **************************************************************************/
int PWM_init(void) {

	//pwm0
	pwm0enable = open(pwm0enablePath, O_RDWR );
	if (pwm0enable < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm0enablePath);
        return 1;
    }
    pwm0freq = open(pwm0freqPath, O_RDWR );
    if (pwm0freq < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm0freqPath);
        return 1;
    }
    pwm0level = open(pwm0levelPath, O_RDWR );
    if (pwm0level < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm0levelPath);
        return 1;
    }
    pwm0mode = open(pwm0ModePath, O_RDWR );
    if (pwm0mode < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm0ModePath);
        return 1;
    }

    //http://linux-sunxi.org/A10/PIO
    //both PWM0 and PWM1 are MUX2, so gpio 5 and 6 must be configured
    //before use
    write(pwm0mode, "2", 1);
    write(pwm0enable, "0", 1);

	memset((void *)scratch, 0, sizeof(scratch)); //clear buffer
    sprintf(scratch, "%d", PWM_FREQ);
    write(pwm0freq, scratch, strlen(scratch));

    write(pwm0enable, "1", 1); //enable works ONLY after cfg
	write(pwm0level, "0", 1);

    //pwm1
	pwm1enable = open(pwm1enablePath, O_RDWR );
	if (pwm1enable < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm1enablePath);
        return 1;
    }
    pwm1freq = open(pwm1freqPath, O_RDWR );
    if (pwm1freq < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm1freqPath);
        return 1;
    }
    pwm1level = open(pwm1levelPath, O_RDWR );
    if (pwm1level < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm1levelPath);
        return 1;
    }
    pwm1mode = open(pwm1ModePath, O_RDWR );
    if (pwm1mode < 0) {
    	DBG(DBG_ERROR,"Unable to open mode file: %s\n",pwm1ModePath);
        return 1;
    }

    write(pwm1mode, "2", 1);
    write(pwm1enable, "0", 1);

	memset((void *)scratch, 0, sizeof(scratch)); //clear buffer
    sprintf(scratch, "%d", PWM_FREQ);
    write(pwm1freq, scratch, strlen(scratch));

    write(pwm1enable, "1", 1); //enable works ONLY after cfg
 	write(pwm1level, "0", 1);

    return 0;  

}


/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.
 
 motor: this should be either 0 or 1, will selet which of the two
 motors to be controlled
 
 direct: Should be between 0 and 3, with the following result
 0: Brake to VCC
 1: Clockwise
 2: CounterClockwise
 3: Brake to GND
 
 pwm: should be a value between 0 and 9, higher the number, the faster
 it'll go
 */
void motorGo(uint8_t motor, uint8_t direction, char * pwm) {

    //uint8_t pwm_value = 0;
    if (motor == 0) {
        if (direction <= 4) //testa o padrao de direcao
        {
            if (direction <= 1)
                write(a1data, "1", 1);
            else
                write(a1data, "0", 1);

            if ((direction == 0) || (direction == 2))
                write(b1data, "1", 1);
            else
                write(b1data, "0", 1);

            if(atoi(pwm) < 0) {
                strcpy(pwm, "0");
            }
            if(atoi(pwm) > 9) {
                strcpy(pwm, "9");
            }


            printf("pwm_rcv: %d\npwm_size: %u\n", atoi(pwm), sizeof(pwm));
            write(pwm0level, pwm, strlen(pwm));
            printf("0: pwm: %d\npwm_size: %d\n", atoi(pwm), strlen(pwm));

       } 
   }
    else {
        if (direction <= 4) //testa o padrao de direcao
        {
            if (direction <= 1)
                write(a2data, "1", 1);
            else
                write(a2data, "0", 1);

            if ((direction == 0) || (direction == 2))
                write(b2data, "1", 1);
            else
                write(b2data, "0", 1);


            if(atoi(pwm) < 0) {
                strcpy(pwm, "0");
            }
            if(atoi(pwm) > 9) {
                strcpy(pwm, "9");
            }
            write(pwm1level, pwm, strlen(pwm));
            printf("1: pwm: %d\npwm_size: %d\n", atoi(pwm), strlen(pwm));
        }
    }
}

void motorOff(int motor) {    //Função para desligar o motor se o mesmo travar
    
	write(a1data, "0", 1);
	write(b1data, "0", 1);
	write(a2data, "0", 1);
	write(b2data, "0", 1);

	if (motor == 0) {
		write(pwm0level, "0", 1);
		
	} else {
		write(pwm1level, "0", 1);
	}
}


//at a frequency of 20kHz, pcDuino v2 PWM ranges from 0 to 9, 10% increments
void move_forward(){
    motorGo(0, CCW, (char *)"5");
    motorGo(1, CCW, (char *)"5");
}
void move_backward(){
    motorGo(0, CW, (char *)"5");
    motorGo(1, CW, (char *)"5");
}
void move_left(){
    motorGo(0, CCW, (char *)"5");
    motorGo(1, CW, (char *)"5");
}
void move_right(){
    motorGo(0, CW, (char *)"5");
    motorGo(1, CCW, (char *)"5");
}


void autonomous_forward(char *pwm){

    if(atoi(pwm) < 0) {
        strcpy(pwm, "0");
    }
    if(atoi(pwm) > 9) {
        strcpy(pwm, "9");
    } 
	motorGo(0, CCW, pwm);
	motorGo(1, CCW, pwm);
}

void autonomous_left(char *pwm){

    if(atoi(pwm) < 0) {
        strcpy(pwm, "0");
    }
    if(atoi(pwm) > 9) {
        strcpy(pwm, "9");
    }	
	motorGo(0, CCW, pwm);
	motorGo(1, CW, pwm);
}
void autonomous_right(char *pwm){
    if(atoi(pwm) < 0) {
        strcpy(pwm, "0");
    }
    if(atoi(pwm) > 9) {
        strcpy(pwm, "9");
    }
	motorGo(0, CW, pwm);
	motorGo(1, CCW, pwm);
}

void move_stop(){
	motorOff(0); 
	motorOff(1); 	
}


/*************************************************************************
 Function: ADC_init ex: ADC sample rate fixed at 125ksps for 12bits ports (A2-A5)
 Purpose: set up the ADC to sense motor current
 **************************************************************************/
int ADC_init(void) {

	adc_cs0 = open(adc_cs0Path, O_RDWR);
	if (adc_cs0 < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",adc_cs0Path);
        return 1;
    }
	adc_cs1 = open(adc_cs1Path, O_RDWR);
	if (adc_cs1 < 0) {
		DBG(DBG_ERROR,"Unable to open mode file: %s\n",adc_cs1Path);
        return 1;
    }
    return 0;
}


int readADC(int adc_fd)
{
    int value=0, c;

    lseek(adc_fd,0,SEEK_SET);
#if 0
    // pcDuino's ADC returns a string "adcx:[value]n"
    // so we need to read and ignore until we get to the value
    c = getc(adc_fd);
    while(c != EOF && c != ':') {
		   c = getc(adc_fd);   	
    }

    // check to see if we're at EOF. If yes, return an error (-1)
    if(c == EOF) return -1;

    // we are at the value so read as long as what we
    // are reading is a number
    c = getc(adc_fd);
    while(c >= '0' && c <= '9')
    {
            value = value * 10 + c -'0';
            c = getc(adc_fd);
    }

    // Continue to read to the end of file but we ignore the data now
    while(c != EOF) {
       c = getc(adc_fd);  	
    }

    // we're going to return the value as an integer
    return value;
#endif
    return 0; //TODO: Fazer funcionar
}


float current_measure(int motor) {
	uint8_t i = 0;
	int result_tmp = 0;
	float current_mA = 0;
	int cs0_value = 0, cs1_value =0;

	if (motor == 0) {

		for (i = N_SAMPLES; i > 0; i--) {
			cs0_value = readADC(adc_cs0);
			result_tmp += cs0_value;
		}
		current_mA = (result_tmp >> N_SHIFTS); //I just get a average of read samples to avoid false positives

		// 5V (ADC reference) / 1024 (ADC 10 bits) / 130 mV per A (approximation) = 38 mA per count
		current_mA = (current_mA * 5.0)/1024.0;
		current_mA = (current_mA * 9665.0)/1500.0; //ajuste de acordo com o datasheet do VNH2SP30
		return current_mA;
	}
	else {

		for (i = N_SAMPLES; i > 0; i--) {
			cs1_value = readADC(adc_cs1);
			result_tmp += cs1_value;
		}
		current_mA = (result_tmp >> N_SHIFTS); //I just get a average of read samples to avoid false positives
		// 5V (ADC reference) / 4096 (ADC 12 bits) / 130 mV per A (approximation) = 38 mA per count
		current_mA = (current_mA * 5.0)/4096.0;
		current_mA = (current_mA * 9665.0)/1500.0; //ajuste conforme o datasheet do VNH2SP30
		return current_mA;
	}
}


void monster_shield_close() {

	move_stop();
	close(adc_cs1);
	close(adc_cs0);
	close(b2data);
	close(b2mode);
	close(a2data);
	close(a2mode);
	close(b1data);
	close(b1mode);
	close(a1data);
	close(a1mode);
	close(pwm1mode);
	close(pwm1level);
	close(pwm1freq);
	close(pwm1enable);
	close(pwm0mode);
	close(pwm0level);
	close(pwm0freq);	
	close(pwm0enable);	

}
