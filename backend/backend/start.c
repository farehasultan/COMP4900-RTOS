#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include "engine.h"
#include "getch.h"

#define THROTTLE_INC 0.1
#define THROTTLE_MAX 1.0
#define THROTTLE_MIN 0.0

void* input() {
	char input;
	while (1) {
		input = getch();
		pthread_mutex_lock(&mutex);
		if      (input == 'w') throttle = (throttle + THROTTLE_INC > THROTTLE_MAX) ? THROTTLE_MAX: throttle + THROTTLE_INC; // Gas
		else if (input == 's') throttle = (throttle - THROTTLE_INC < THROTTLE_MIN) ? THROTTLE_MIN: throttle - THROTTLE_INC; // Brake
		pthread_mutex_unlock(&mutex);
	}
}

int main(void) {
	pthread_t          e_tid, i_tid;
	pthread_attr_t     attr;
	struct sched_param sp;
	throttle = 0.0;
	pthread_mutex_init(&mutex, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	sp.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&e_tid, &attr, engine, NULL);

	sp.sched_priority = 1;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&i_tid, &attr, input, NULL);

	pthread_join(e_tid, NULL);
	pthread_join(i_tid, NULL);

	return 0;
}
