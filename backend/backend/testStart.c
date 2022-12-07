#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "engine.h"
#include "getch.h"

#define ENGINE_PULSE_EVENT (_PULSE_CODE_MINAVAIL + 1)
#define NS_PER_MS 1000000
#define THROTTLE_INC 0.1
#define THROTTLE_MAX 1.0
#define THROTTLE_MIN 0.0

pthread_mutex_t mutex;
volatile float throttle;

typedef union {
	struct _pulse pulse;
} message_t;

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

int main(int argc, char *argv[]) {
	int               rcvid;
	struct sigevent   event;
	int               chid, coid;
	message_t         msg;
	timer_t           timerid;
	struct itimerspec it;
	struct sched_param sp;
	float  localThrottle;
	double localRPM;
	throttle = 0.0;


	engine_data_t engine_data;
	initEngine(&engine_data);
	pthread_t          i_tid;
	pthread_attr_t     attr;
	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	sp.sched_priority = 1;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&i_tid, &attr, input, NULL);

	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("ChannelCreate()");
		return chid;
	}

	coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	SIGEV_PULSE_INIT(&event, coid, 1, ENGINE_PULSE_EVENT, 0);
	timer_create(CLOCK_REALTIME, &event, &timerid);

	it.it_value.tv_sec     = 0;
	it.it_value.tv_nsec    = NS_PER_MS;
	it.it_interval.tv_sec  = 0;
	it.it_interval.tv_nsec = 10 * NS_PER_MS;
	timer_settime(timerid, 0, &it, NULL);

	while (1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			perror("MsgReceive()");
			return rcvid;
		} else if (rcvid == 0) {
			switch (msg.pulse.code) {
				case ENGINE_PULSE_EVENT:
					pthread_mutex_lock(&mutex);
					localThrottle = throttle;
					pthread_mutex_unlock(&mutex);
					localRPM = engine(localThrottle, &engine_data);
					printf("Current RPM: %f\r", localRPM);

					break;

				case _PULSE_CODE_DISCONNECT:
					printf("Received: disconnect pulse\n");
					if (-1 == ConnectDetach(msg.pulse.scoid))
						perror("ConnectDetach");
					break;

				default:
					printf("Unknown pulse received, code = %d\n", msg.pulse.code);
			}
		}
	}
	timer_delete(timerid);
	return 0;
}
