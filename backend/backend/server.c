#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/json.h>

#define SERVER_PORT    6000 // Port for the QNX back-end server
#define NUM_CLIENTS    1    // Number of clients the server will handle
#define MAX_STRING_LEN 255  // Max string length for the server's buffer

#define RPM_MAX           12000.0
#define ACCELERATION_RATE 200.0
#define FUEL_MAX          10000
#define FUEL_DRAIN_RATE   5
#define FUEL_EMPTY        0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

// declare a struct with two properties
// instantiate a new struct
struct Car {
	float throttle ;
	double rpm;
} car;

void *engine_check(void *);

int main() {
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddr;
	int status, addrSize;
	float throttle;
	double rpm;

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
			// Get the message from the client
			recv(clientSocket, &throttle, sizeof(car), 0);
			printf("[SERVER] Received client request\n");
			printf("Received throttle from client %f \n",throttle);


			printf("[SERVER] Sending back data ! \n");
			rpm=2.34542;
			send(clientSocket, &rpm, sizeof(car), 0);
			//if ((strcmp(buffer,"done") == 0) || (strcmp(buffer,"stop") == 0))
				break;
		}
		printf("[SERVER] Closing client connection.\n");
		close(clientSocket); // Closes the client's socket
	}

	// Closes the server's socket
	close(serverSocket);
	printf("[SERVER] Shutting down.\n");
}


/*
 * no need for engine check as we got rid of multiple properties for the struct car
 */
/*
void *engine_check(void *arg) {
	while (1) {
		pthread_mutex_lock(&mutex);
		if (car.engine_start) {
			car.rpm         = (car.rpm + ACCELERATION_RATE <= RPM_MAX) ? car.rpm + ACCELERATION_RATE : RPM_MAX;
			car.fuel_amount = (car.fuel_amount >= FUEL_DRAIN_RATE) ? car.fuel_amount - FUEL_DRAIN_RATE : FUEL_EMPTY;
		}
		pthread_mutex_unlock(&mutex);
	}
	return (NULL);
}
*/
//const char* prepare_json(){
//	json_encoder_t *enc = json_encoder_create();
//	json_encoder_start_object(enc,NULL);
//	json_encoder_add_int(enc, "id", 1); //set id 1 by default
//	json_encoder_start_object(enc, "info");
//	json_encoder_add_int(enc, "engine_start", car.engine_start);
//	json_encoder_add_string(enc, "someString", car.someString);
//	json_encoder_add_int(enc, "fuel_amount", car.fuel_amount);
//	json_encoder_end_object(enc);
//	json_encoder_end_object(enc);
//	json_encoder_error_t status = json_encoder_get_status(enc);
//
//	// If everything above has succeeded, json_encoder_get_status() will return
//	// JSON_ENCODER_OK
//	if ( status != JSON_ENCODER_OK ) {
//		printf("Data preparation failed\\n");
//		return false;
//	}
//
//	// Write the JSON data into the string space
//	char str[json_encoder_length(enc)*100];
//	snprintf(str, sizeof(str), "%s", json_encoder_buffer(enc));
//	json_encoder_destroy(enc);
//	printf("%s \\n",str);
//	return str;
//
//}


