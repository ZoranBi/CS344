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
	int valid = 0;
	int ascii = (int)*input;

	if (ascii >= 65 && ascii <= 90) valid = 1;
	else if (ascii == 32) valid = 1;
	else if (ascii >= 97 && ascii <= 122) valid = 1;
	else valid = 0;
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
	memset(buffer, '\0', sizeof(buffer));
	strcat(buffer, this_send);
	printf("Client: Sending:\n%s\nto Server\n", buffer);
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
}

void receive_server(int socketFD){
	char buffer[BUFFER_SIZE];
	int charsRead;
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	if(strcmp(buffer, "SYN_ACK")) error("ERROR: CANNOT CONNECT WITH SERVER\n");
	printf("CLIENT: Receiving:\n\"%s\"\nfrom Server\n", buffer);
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUFFER_SIZE];
	char content[MAX_SIZE];
	char mykey[MAX_SIZE];
	int num_pkt = 0;

	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// // Get input message from user
	// // printf("CLIENT: Enter text to send to the server, and then hit enter: ");
	// memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	// // fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
	// strcat(buffer, "SYN");
	// // buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	//
	// // Send message to server
	// charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	// if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	// if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	send_server("SYN", socketFD);

	// // Get return message from server
	// memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	// charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	// if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	// if(strcmp(receive_server(socketFD), "SYN_ACK")) error("ERROR: CANNOT CONNECT WITH SERVER\n");
	// printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	receive_server(socketFD);

	// // work on send file
	int size_file = 0, size_key = 0, i = 0;
	// FILE *file_ptr = fopen(argv[1], "r");
	// fgets(content, MAX_SIZE,file_ptr);
	// content[strcspn(content, "\n")] = '\0';
	// size_file = strlen(content);
	//
	// for(i = 0; i < size_file; i++){
	// 	if(!badinput_check(&content[i])) error("CLIENT: BAD INPUT DETECTED\n");
	// }
	//
	// FILE *key_ptr = fopen(argv[2], "r");
	// fgets(mykey, MAX_SIZE, key_ptr);
	// mykey[strcspn(mykey, "\n")] = '\0';
	// size_key = strlen(mykey);
	//
	// if(size_file > size_key) error("CLIENT: KEY IS TOO SHORT\n");
	size_file = 2048;
	num_pkt = number_pkt(size_file);
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	sprintf(buffer, "%d", num_pkt);
	// printf("my packet number is: %s\n", buffer);

	// Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("CLIENT: I received this from the server: \"%s\"\n", buffer);


	// fclose(file_ptr);
	// fclose(key_ptr);
	close(socketFD); // Close the socket

	return 0;
}
