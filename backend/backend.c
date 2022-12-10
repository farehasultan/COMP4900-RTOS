/*
 * testStart.c
 *
 *      Author: Iain Found
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "engine.h"

#define ENGINE_PULSE_EVENT (_PULSE_CODE_MINAVAIL + 1)
#define NS_PER_MS 1000000
#define SERVER_PORT    6000 // Port for the QNX back-end server
#define NUM_CLIENTS    1    // Number of clients the server will handle
#define MAX_STRING_LEN 255  // Max string length for the server's buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile float  globalThrottle;
volatile double globalRPM;

typedef union {
	struct _pulse pulse;
} message_t;

void* pulseHandler() {
	int               rcvid;
	struct sigevent   event;
	int               chid, coid;
	message_t         msg;
	timer_t           timerid;
	struct itimerspec it;

	engine_data_t engine_data;
	initEngine(&engine_data);

	chid = ChannelCreate(0);
	if (chid == -1) perror("ChannelCreate()");

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
		} else if (rcvid == 0) {
			switch (msg.pulse.code) {
				case ENGINE_PULSE_EVENT:
					pthread_mutex_lock(&mutex);
					globalRPM = engine(globalThrottle, &engine_data);
					pthread_mutex_unlock(&mutex);
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
}

void* server() {
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddr;
	int status, addrSize;
	float  localThrottle;
	double localRPM;

	// Create the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Setup the server address
	memset(&serverAddress, 0, sizeof(serverAddress)); // zeros the struct
	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons((unsigned short) SERVER_PORT);

	// Bind the server socket
	status = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (status < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Set up the line-up to handle a single client
	status = listen(serverSocket, NUM_CLIENTS);
	if (status < 0) {
		perror("[SERVER ERROR]");
		exit(-1);
	}

	// Wait for clients now
	while (1) {
		addrSize = sizeof(clientAddr);
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
		printf("[SERVER] Closing client connection.\n");
		close(clientSocket); // Closes the client's socket
	}

	// Closes the server's socket
	close(serverSocket);
	printf("[SERVER] Shutting down.\n");
}

int main(int argc, char *argv[]) {
	pthread_t          e_tid, i_tid;
	pthread_attr_t     attr;
	struct sched_param sp;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	sp.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&e_tid, &attr, pulseHandler, NULL);

	sp.sched_priority = 1;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&i_tid, &attr, server, NULL);

	pthread_join(e_tid, NULL);
	pthread_join(i_tid, NULL);

	return 0;
}
