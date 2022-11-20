#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT    6000 // Port for the QNX back-end server
#define NUM_CLIENTS    1    // Number of clients the server will handle
#define MAX_STRING_LEN 255  // Max string length for the server's buffer

int calculate_checksum(char *text);

int main() {
	int serverSocket, clientSocket;
	struct sockaddr_in serverAddress, clientAddr;
	int status, addrSize, bytesRcv;
	char buffer[MAX_STRING_LEN + 1]; // +1 for null terminator
	char response[MAX_STRING_LEN + 1];

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
			bytesRcv = recv(clientSocket, buffer, MAX_STRING_LEN, 0);
			buffer[bytesRcv] = 0; // put a 0 at the end so we can display the string
			printf("[SERVER] Received client request: %s\n", buffer);

			// Respond with the checksum
			snprintf(response, MAX_STRING_LEN, "%d", calculate_checksum(buffer));
			printf("[SERVER] Sending \"%s\" to client\n", response);
			send(clientSocket, response, strlen(response), 0);
			if ((strcmp(buffer,"done") == 0) || (strcmp(buffer,"stop") == 0))
				break;
		}
		printf("[SERVER] Closing client connection.\n");
		close(clientSocket); // Closes the client's socket
		if (strcmp(buffer,"stop") == 0)
			break;
	}

	// Closes the server's socket
	close(serverSocket);
	printf("[SERVER] Shutting down.\n");
}

int calculate_checksum(char *text) {
	char *c;
	int cksum = 0;

	for (c = text; *c != '\0'; c++)
		cksum += *c;
	return cksum;
}
