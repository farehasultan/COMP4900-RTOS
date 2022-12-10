/*
 * engine.h
 *
 * Created on: Nov 25, 2022
 * Author: Danny Nemec
 */

#ifndef ENGINE_H_   /* Include guard */
#define ENGINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/neutrino.h>

#define NUM_CYL            6
#define CYL_OFFSET         120 // 120 for 6 cylinders
#define MOMENT_OF_INERTIA  3   // I = mr^2 for engine: 0.5 * 24kg * 0.5m^2 = 3
#define PI                 3.141592653589
#define NS_TO_WAIT         10000000
#define NS_PER_MILLISECOND 1000000
#define NS_PER_SECOND      1000000000
#define POWERVAL           1000
#define MAXRPM             12000
#define MINRPM             1500

// Full cycle is 720 degrees
// Stage changes register at their exact start
// All cylinder math is calculated at the start, even if the real forces occur over time afterwards
// Stage 0: intake      (cylinder down)
// Stage 1: compression (cylinder up)
// Stage 2: ignition    (cylinder down)
// Stage 3: exhaust     (cylinder up)
typedef struct cylinder_data {
	char angle; // 0-179, modulo at 180
	char stage; // 0-3,   modulo at 4
} cylinder_data_t;

typedef struct engine_data {
	cylinder_data_t cylinders[NUM_CYL];
	struct timespec nextevent;
	int             enginejoules;
	int             currentforce;
	double          afterRPM;
	long            carryoverTime;
} engine_data_t;

int    stagecalc(char, int*, float, double);
double calcRPM(int);
long   rpmToEventNS(double);
void   initEngine(engine_data_t*);
double engine(float, engine_data_t*);

int stagecalc(char stage, int *force, float throttle, double rpm) {
	float useThrottle = throttle;
	// Every 1rpm below MINRPM is 0.01 throttle up to a max of 1.00
	if (rpm < MINRPM) {
		useThrottle = (float) (MINRPM - rpm) / 100;
		if (throttle > useThrottle) useThrottle = throttle;
	}
	if (useThrottle > 1) useThrottle = 1;
	if (useThrottle < 0) useThrottle = 0;
	if (rpm > MAXRPM)    useThrottle = 0; // Hard cut throttle rev limiter

	switch(stage) {
		case 0: // Intake
			*force = 0;
			break;
		case 1: // Compression
			*force = -1 * POWERVAL;
			break;
		case 2: // Ignition
			*force = 3 * POWERVAL * useThrottle; // Multiply by throttle setting
			break;
		case 3: // Exhaust
			*force = 0;
			break;
	}
	return 0;
}

// Convert joules to radians then RPM
double calcRPM(int energy) {
	double numerator = 2 * energy;
	double radians   = sqrt(numerator / MOMENT_OF_INERTIA);
	double rps       = radians / (2.0 * PI);
	double rpm       = rps * 60.0;
	return rpm;
}

// Nanoseconds until next cylinder event at given rpm
long rpmToEventNS(double rpm) {
	double eventsPerSecond = rpm / 10.0; // Multiply rps by 6, or divide rpm by 10
	long timeNS;
	if (eventsPerSecond < 1) eventsPerSecond = 1;
	timeNS = NS_PER_SECOND / eventsPerSecond;
	if (timeNS < 10000) timeNS = 10000;
	return timeNS;
}

void initEngine(engine_data_t *engine_data) {
	engine_data->enginejoules  = 0;
	engine_data->afterRPM      = 0;
	engine_data->carryoverTime = 0;

	int setangle = 0;
	for (int i=0; i < NUM_CYL; i++) {
		setangle = CYL_OFFSET * i;
		engine_data->cylinders[i].angle = setangle % 180;
		engine_data->cylinders[i].stage = (setangle / 180) % 4;
	}
}

double engine(float throttle, engine_data_t *engine_data) {
	int totalforce;
	// Repeat engine calculations until 10ms of virtual time has passed
	while(engine_data->carryoverTime < NS_TO_WAIT) {
		// Increase angle by 60 for everything, process state changes
		totalforce = 0;
		for (int i = 0; i < NUM_CYL; i++) {
			engine_data->cylinders[i].angle = (engine_data->cylinders[i].angle + 60) % 180;
			if (engine_data->cylinders[i].angle == 0) { // Next stage
				// Only stage transitions influence calculations
				engine_data->cylinders[i].stage = (engine_data->cylinders[i].stage + 1) % 4;
				stagecalc(engine_data->cylinders[i].stage, &engine_data->currentforce, throttle, engine_data->afterRPM);
				totalforce += engine_data->currentforce;
			}
		}
		engine_data->enginejoules += totalforce;
		// Never go below 0
		if (engine_data->enginejoules < 0) engine_data->enginejoules = 0;
		engine_data->afterRPM = calcRPM(engine_data->enginejoules);
		engine_data->carryoverTime += rpmToEventNS(engine_data->afterRPM);
	}
	// If carryoverTime / NS_TO_WAIT is 2 or greater, then what should be 20ms or greater will still be 10ms
	// This is because it's only a problem below idle RPM
	engine_data->carryoverTime = engine_data->carryoverTime % NS_TO_WAIT;
	return engine_data->afterRPM;
}

#endif
