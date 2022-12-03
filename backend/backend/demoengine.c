/*
 * demoengine.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Danny Nemec
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/neutrino.h>

//full cycle is 720 degrees
//stage changes register at their exact start
//all cylinder math is calculated at the start, even if the real forces occur over time afterwards
//stage 0: intake (cylinder down)
//stage 1: compression (cylinder up)
//stage 2: ignition (cylinder down)
//stage 3: exhaust (cylinder up)
typedef struct cylinder_data {
	char angle; //0-179, modulo at 180
	char stage; //0-3, modulo at 4
} cylinder_data_t;

#define NUM_CYL 6
#define CYL_OFFSET 120 //120 for 6 cylinders
#define MOMENT_OF_INERTIA 3 // I=mr^2 for engine: 0.5 * 24kg * 0.5m^2 = 3
#define PI 3.141592653589
#define NS_TO_WAIT 10000000
#define NANOSECOND 1000000000
#define POWERVAL 1000
#define MAXRPM 12000
#define MINRPM 1500


int stagecalc(char stage, int *force, float throttle, double rpm) {
	float useThrottle = throttle;
	//every 1rpm below MINRPM is 0.01 throttle up to a max of 1.00
	if (rpm < MINRPM) {
		useThrottle = (float) (MINRPM - rpm) / 100;
		if (throttle > useThrottle) {useThrottle = throttle;}
	}
	if (useThrottle > 1) {useThrottle = 1;}
	if (useThrottle < 0) {useThrottle = 0;}
	if (rpm > MAXRPM) {useThrottle = 0;} //hard cut throttle rev limiter

	switch(stage)
	{
	case 0: //intake
		*force = 0;
		break;
	case 1: //compression
		*force = -1*POWERVAL;
		break;
	case 2: //ignition
		*force = 3*POWERVAL*useThrottle; //multiply by throttle setting?
		break;
	case 3: //exhaust
		*force = 0;
		break;
	}
	//frictional losses:
	//*force = *force - 1;

	return 0;
}

//convert joules to radians then RPM
double calcRPM(int energy) {
	double numerator = 2*energy;
	double radians;
	radians = sqrt(numerator / MOMENT_OF_INERTIA);
	double rps = radians / (2.0*PI);
	double rpm = rps*60.0;
	return rpm;
}

//nanoseconds until next cylinder event at given rpm
long rpmToEventNS(double rpm) {
	double eventsPerSecond = rpm / 10.0; //multiply rps by 6, or divide rpm by 10
	long timeNS;
	if (eventsPerSecond < 1) {
		eventsPerSecond = 1;
		//printf("events per second increased to 1\n");
	}
	timeNS = NANOSECOND / eventsPerSecond;
	if (timeNS < 10000) {
		timeNS = 10000;
	}
	return timeNS;
}


int main(void) {
	cylinder_data_t cylinders[NUM_CYL];
	struct timespec nextevent;

	//initialize cylinder situation
	int setangle;
	for (int i=0; i < NUM_CYL; i++) {
		setangle = CYL_OFFSET * i;
		cylinders[i].angle = setangle % 180;
		cylinders[i].stage = (setangle / 180) % 4;
		printf("cylinder i=%d, angle=%d, stage=%d\n", i, cylinders[i].angle, cylinders[i].stage);
	}

	//initialize timer
	struct _clockperiod newval, oldval;
	newval.nsec = 1000*1000;
	newval.fract = 0;
	ClockPeriod(CLOCK_REALTIME, &newval, &oldval, 0);
	printf("oldval nsec=%d\n", oldval.nsec);

	int enginejoules = 0;
	int totalforce;
	int currentforce;
	float throttle = 0.4; //0 to 1
	char printchar;
	double afterRPM = 0;
	long waitTime;
	int count = 0;
	long carryoverTime = 0;
	long nswait;
	//long nswait_factor = NANOSECOND / NS_TO_WAIT;

	while (1) {
		//increase angle by 60 for everything, process state changes
		totalforce = 0;
		//printf("before rpm:%f (%d) ", calcRPM(enginejoules), enginejoules);
		for (int i=0; i < NUM_CYL; i++) {
			printchar = 97;
			cylinders[i].angle = (cylinders[i].angle + 60) % 180;
			if (cylinders[i].angle == 0) { //next stage
				//only stage transitions influence calculations
				cylinders[i].stage = (cylinders[i].stage + 1) % 4;
				stagecalc(cylinders[i].stage, &currentforce, throttle, afterRPM);
				totalforce += currentforce;
				printchar = printchar - 32;
			}
			printchar += cylinders[i].stage;
			//if (count == 0) {printf("%c",printchar);}
		}
		enginejoules += totalforce;
		//never go below 0
		if (enginejoules < 0) {enginejoules = 0;}

		afterRPM = calcRPM(enginejoules);
		waitTime = rpmToEventNS(afterRPM);
		carryoverTime += waitTime;
		if (carryoverTime > NS_TO_WAIT) {
			nswait = (carryoverTime / NS_TO_WAIT) * NS_TO_WAIT; //nswait_factor * 1000;
			carryoverTime = carryoverTime % NS_TO_WAIT;
			nextevent.tv_sec = 0;
			nextevent.tv_nsec = nswait;
			printf("rpm:%f (%dJ), ns:%ld, wait=%ld, carry=%ld, count=%d\n", afterRPM, enginejoules,
					waitTime, nswait, carryoverTime, count);
			count = 0;
			nanosleep(&nextevent, NULL);
		}
		count += 1;
		//nextevent.tv_sec = 0;
		//nextevent.tv_nsec = waitTime;
		//if (count == 0) {printf(" after rpm:%f (%d), ns:%ld\n", afterRPM, enginejoules, waitTime);}
		//nanosleep(&nextevent, NULL);
		//count = (count + 1) % 100;
	}
	return 0;
}
