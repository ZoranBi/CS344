//  Assignment: OTP
//  File Name: otp_enc.c
//  Author: Zongzhe Bi
//  Class: CS 344
//  Created by Zongzhe Bi on 11/25/18.
//  Copyright © 2018 Zongzhe Bi. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define MAX_SIZE 65535

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int badinput_check(char* input){
	int valid = 0, i =0, ascii;

	for(i = 0; i < strlen(input); i ++){
		ascii = input[i];
		if (ascii >= 65 && ascii <= 90) valid = 1;
		else if (ascii == 32) valid = 1;
		else if (ascii >= 97 && ascii <= 122) valid = 1;
		else valid = 0;
	}
	return valid;
}

int number_pkt(int size_file){
	int is_factor, num_pkt;
	num_pkt = size_file / BUFFER_SIZE;
	is_factor = size_file % BUFFER_SIZE;
	if(is_factor) num_pkt++;
	return num_pkt;
}

void send_server(char* this_send, int socketFD){
	char buffer[BUFFER_SIZE];
	int charsWritten;
	memset(buffer, '\0', BUFFER_SIZE);
	strcat(buffer, this_send);
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
}

void receive_server(char* cipher, int socketFD){
	char buffer[BUFFER_SIZE];
	int charsRead;
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	if(strcmp(buffer, "ACK: SYN")) error("ERROR: CANNOT CONNECT WITH SERVER_1\n");
   strcat(cipher, buffer);
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUFFER_SIZE];

	char *content, *mykey, *cipher;
	int num_pkt = 0, i;

	if (argc < 4) error("Invalid input\n"); // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]);                                 // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET;                         // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber);                 // Store the port number
	serverHostInfo = gethostbyname("localhost");                    // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

  // handshake
	send_server("SYN", socketFD);

   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//

	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	if(strcmp(buffer, "ACK: SYN")) error("ERROR: CANNOT! CONNECT WITH SERVER\n");


	content = malloc(sizeof(char)*MAX_SIZE);
	memset(content, '\0', MAX_SIZE);
	mykey = malloc(sizeof(char)*MAX_SIZE);
	memset(mykey, '\0', MAX_SIZE);
	cipher = malloc(sizeof(char)*MAX_SIZE);
	memset(cipher, '\0', MAX_SIZE);
	// work on send file
	int size_file = 0, size_key = 0;


	FILE *file_ptr = fopen(argv[1], "r");
	fgets(content, MAX_SIZE, file_ptr);
	content[strcspn(content, "\n")] = '\0';
	size_file = strlen(content);
   fclose(file_ptr);

	FILE *key_ptr = fopen(argv[2], "r");
	fgets(mykey, MAX_SIZE, key_ptr);
	mykey[strcspn(mykey, "\n")] = '\0';
	size_key = strlen(mykey);
   fclose(key_ptr);


  // Bad input check
	if(!badinput_check(content)) error("CLIENT: BAD INPUT DETECTED\n");
  // text file size & key file size Cmp
	if(size_file > size_key) error("CLIENT: KEY IS TOO SHORT\n");

	num_pkt = number_pkt(size_file);
	sprintf(buffer, "%d", num_pkt);

   // Send pkt number
   send_server(buffer, socketFD);

   memset(buffer, '\0', sizeof(buffer));                       // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);  // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	if(strcmp(buffer, "ACK: pkt_num")) error("ERROR: CANNOT CONNECT WITH SERVER_2\n");

  // Sending text and key
   for(i = 0; i < num_pkt; i ++){
		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		memcpy(buffer, (content + i * (BUFFER_SIZE-1)), (BUFFER_SIZE-1));
		// Send message to server
		send_server(buffer, socketFD);


		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		if(strcmp(buffer, "ACK: Content Get")) error("ERROR: CANNOT CONNECT WITH SERVER_3\n");

		// Send key to server
		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		memcpy(buffer, (mykey + i * (BUFFER_SIZE-1)), (BUFFER_SIZE-1));
		send_server(buffer, socketFD);


		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		if(strcmp(buffer, "ACK: Key Get")) error("ERROR: CANNOT! CONNECT WITH SERVER_4\n");

		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		send_server("Req: Need Cipher", socketFD);
		// Get return message from server
		memset(buffer, '\0', sizeof(buffer));                    // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		strcat(cipher, buffer);
   }

	// output
	printf("%s\n", cipher);

	close(socketFD); // Close the socket
	free(content);
	free(mykey);
	free(cipher);
	return 0;
}
