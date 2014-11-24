#include "RASDemo.h"
#include <stdio.h>
#include <time.h>
#include <RASLib/inc/common.h>
#include <RASLib/inc/adc.h>
#include <RASLib/inc/motor.h>
#include <stdlib.h>
#include <RASLib/inc/linesensor.h>
#include <RASLib/inc/servo.h>

tADC *irs[3];
int IR1val;
int IR2val;
int IR3val;
float line[8];
static tMotor *leftMotor;
static tServo *clappy;
static tMotor *rightMotor;

static tLineSensor *gls;
static tBoolean initialized = false;
static float speed = 0.3;
int wallPrev = 2;
int clapLength = 100;
int inc = 1;
float clapSpeed = -.001;
tBoolean led_on;
int wallTurned=0;
int side;
int countdown = 0;
int actualLine = 0;


void initGPIOLineSensors(void) {
    // don't initialize this if we've already done so
    if (initialized) {
        return;
    }
    
    initialized = true;
		//line[0] == sensor 8
    // use 8 I/O pins to initialize a GPIO line sensor
    gls = InitializeGPIOLineSensor(
        PIN_A6, 
        PIN_A5, 
        PIN_B4, 
        PIN_E5, 
        PIN_E4, 
        PIN_B1, 
        PIN_B0, 
        PIN_B5
        );
}

void initServos(void) {
    clappy = InitializeServo(PIN_F0);
}

void initIRSensors(void) {
	
    irs[0] = InitializeADC(PIN_E3); //left
    irs[1] = InitializeADC(PIN_E2); //center
		irs[2] = InitializeADC(PIN_E1);	//right
		//SetPin(PIN_F1, 1);
}

void initMotor(void) {
		rightMotor = InitializeServoMotor(PIN_A3, false);
		leftMotor = InitializeServoMotor(PIN_F4, true);
}

void goMarvinGo(void){
	initMotor();
	initServos();
	initIRSensors();
	initGPIOLineSensors();
	SetServo(clappy, 0.0f);
	while(1){
		IR1val = (int) (1000 * ADCRead(irs[0]));
		IR2val = (int) (1000 * ADCRead(irs[1]));
		IR3val = (int) (1000 * ADCRead(irs[2]));
		LineSensorReadArray(gls, line);
		if(countdown > 0){
			countdown--;
		}
		if(actualLine > 0){
			actualLine--;
		}
		//clap();
		lineHandle();
		walls();
		//holdTheLine();
		//followThatLine();
		
	}
}

void clap(void){

	if(clapLength > 170){
		SetServo(clappy, .0f);
		inc = -1;
		SetPin(PIN_F1, 1);
	}
	if(clapLength < 1){
		SetServo(clappy, .1f);
		inc = 1;
		SetPin(PIN_F1, 0);
	}
	clapLength += inc;
}

//right side low
//side 2 = right
void lineHandle(void){
	//side = 0;
	int i;
	if(actualLine > 100){
		
	for(i = 0; i < 4; i++){
		if(line[i] > 10){
			side = 2;
			countdown = 2500;
			break;
		}
		if(line[7-i] > 10){
			side = 1;
			countdown = 2500;
			break;
		}	
	}
}
}

void walls(void){
	int i;
	int inf = 0;
	float newSpeed;
	
	for(i = 0; i< 8; i++){
		if(line[i] > 10){
			inf++;
		}
	}
	
	if (wallTurned >0 && wallTurned < 500){
		wallTurned++;
	}
	
		
	if(inf > 0){
		if(actualLine> 100){
			SetPin(PIN_F1, 1);
			SetMotor(leftMotor, speed);
			SetMotor(rightMotor, speed);
		}
		if(actualLine < 150){
			actualLine += 2;
		}
		
	}else{
		if(actualLine < 100){
			SetPin(PIN_F1, 0);
		}
		
		if(IR2val > 300 && wallTurned < 500 && wallPrev < 2){
			if(wallPrev == 0){
				SetMotor(leftMotor, 0.05f);
				SetMotor(rightMotor, speed);
				wallTurned = 1;
			}else if(wallPrev == 1){
				SetMotor(leftMotor, speed);
				SetMotor(rightMotor, 0.05f);
				wallTurned = 1;
			}
		}else if(IR1val > 300){
			newSpeed = speed - (IR1val/1000.0 * speed);
			SetMotor(leftMotor, newSpeed);
			SetMotor(rightMotor, speed);
			wallPrev = 0;
		}else if(IR3val > 300){
			newSpeed = speed - (IR3val/1000.0 * speed);
			SetMotor(leftMotor, speed);
			SetMotor(rightMotor, newSpeed);
			wallPrev = 1;
		}else{
			if(wallPrev == 0 || (side == 1 && countdown > 0)){
				SetMotor(rightMotor, 0.1f);
				SetMotor(leftMotor, speed);
			}else if(wallPrev == 1 || (side == 2 && countdown > 0)){
				SetMotor(rightMotor, speed);
				SetMotor(leftMotor, 0.1f);
			}else{
				SetMotor(leftMotor, speed);
				SetMotor(rightMotor, speed);
			}
		}
		if(wallPrev == 0){
			if(IR1val < 200){
				wallPrev = 2;
			}
		}else if(wallPrev == 1){
			if(IR3val < 200){
				wallPrev = 2;
			}
		}
		if(IR2val > 200 && wallTurned > 500){
			wallPrev = 2;
			wallTurned = 0;
		}
		
	}
	

}
