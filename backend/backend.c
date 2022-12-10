/*
 * backend.c
 *
 * Created on: Nov 18, 2022
 * Author: Iain Found
 */

#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/dispatch.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "engine.h"

#define ENGINE_PULSE_EVENT (_PULSE_CODE_MINAVAIL + 1)
#define SERVER_PORT    6000 // Port for the QNX back-end server
#define NUM_CLIENTS    1    // Number of clients the server will handle

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile float  globalThrottle;
volatile double globalRPM;

typedef union {
	struct _pulse pulse;
} pulse_t;

void* pulseHandler() {
	int               rcvid;
	struct sigevent   event;
	int               chid, coid;
	pulse_t           msg;
	timer_t           timerid;
	struct itimerspec it;

	// Pulse handler will store engine data once the engine function's timer is up
	engine_data_t engine_data;
	initEngine(&engine_data);

	// Creating the channel and timer for the engine event
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("ChannelCreate()");
		exit(-1);
	}
	coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
	SIGEV_PULSE_INIT(&event, coid, 1, ENGINE_PULSE_EVENT, 0);
	timer_create(CLOCK_REALTIME, &event, &timerid);

	// Engine goes off periodically every 10ms
	it.it_value.tv_sec     = 0;
	it.it_value.tv_nsec    = NS_PER_MILLISECOND;
	it.it_interval.tv_sec  = 0;
	it.it_interval.tv_nsec = 10 * NS_PER_MILLISECOND;
	timer_settime(timerid, 0, &it, NULL);

	while (1) {
		rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {
			perror("MsgReceive()");
		} else if (rcvid == 0) {
			switch (msg.pulse.code) {
				case ENGINE_PULSE_EVENT:
					// Updates the engine with the current throttle information so it can do its next cycle
					// Retrieves the RPM after the cycle is complete for the server to send back
					pthread_mutex_lock(&mutex);
					globalRPM = engine(globalThrottle, &engine_data);
					pthread_mutex_unlock(&mutex);
					break;

				case _PULSE_CODE_DISCONNECT:
					printf("Received: disconnect pulse\n");
					if (-1 == ConnectDetach(msg.pulse.scoid)) {
						perror("ConnectDetach");
						exit(-1);
					}
					break;

				default:
					printf("Unknown pulse received, code = %d\n", msg.pulse.code);
			}
		}
	}
	timer_delete(timerid);
}

void* server() {
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddr;
	unsigned int addrSize;
	int status;
	float  localThrottle;
	double localRPM;

	// Create the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Setup the server address
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons((unsigned short) SERVER_PORT);

	// Bind the server socket
	status = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (status < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Prepare the line for the middleman
	status = listen(serverSocket, NUM_CLIENTS);
	if (status < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Wait for the middleman to reach out
	while (1) {
		addrSize     = sizeof(clientAddr);
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
		if (clientSocket < 0) {
			perror("[SERVER ERROR]");
			exit(-1);
		}
		printf("[SERVER] Received client connection.\n");

		// Go into infinite loop to talk to client
		while (1) {
			recv(clientSocket, &localThrottle, sizeof(localThrottle), 0);
			pthread_mutex_lock(&mutex);
			globalThrottle = localThrottle;
			localRPM       = globalRPM;
			pthread_mutex_unlock(&mutex);
			send(clientSocket, &localRPM, sizeof(localRPM), 0);
		}
		// Currently no way to shutdown outside of Ctrl+C
		printf("[SERVER] Closing client connection.\n");
		close(clientSocket);
	}
	close(serverSocket);
	printf("[SERVER] Shutting down.\n");
}

int main(int argc, char *argv[]) {
	pthread_t          p_tid, s_tid;
	pthread_attr_t     attr;
	struct sched_param sp;

	// Set up the threads for the backend
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	// Pulse handler is higher priority as it handles more important information
	// but fires off less frequently
	sp.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&p_tid, &attr, pulseHandler, NULL);

	// Server is lower priority as it only handles updating throttle and sending
	// back RPM information. It fires off more frequently
	sp.sched_priority = 1;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&s_tid, &attr, server, NULL);

	// Just in case, though it should never reach this point
	pthread_join(p_tid, NULL);
	pthread_join(s_tid, NULL);

	return 0;
}
