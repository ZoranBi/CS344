//  Assignment: OTP
//  File Name: otp_enc_d.c
//  Author: Zongzhe Bi
//  Class: CS 344
//  Created by Zongzhe Bi on 11/25/18.
//  Copyright © 2018 Zongzhe Bi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define NO_SOCKET 5
#define BUFFER_SIZE 1024

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

char* encoder(char* text, char* key){
    int size_file = strlen(text);
	 int i = 0;
    int ascii_text, ascii_key, ascii_code;
    char* enc = malloc(sizeof(char)*BUFFER_SIZE);

    // encode the text letter by letter
    for(i = 0; i < size_file; i ++){
      ascii_text = (int)text[i];
      ascii_key = (int)key[i];
      if (ascii_text == 32) ascii_text = 91;
      if (ascii_key == 32) ascii_key = 91;
      ascii_code = ascii_text + ascii_key - 65;
		if (ascii_code > 91) ascii_code -= 26;
      if (ascii_code == 91) ascii_code = 32;
      enc[i] = (char)ascii_code;
    }
    return enc;
}

int main(int argc, char *argv[]){
   int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
   socklen_t sizeOfClientInfo;
   char buffer[BUFFER_SIZE];
   struct sockaddr_in serverAddress, clientAddress;

   pid_t pid;
   int status;
   int num_pkt;

   if (argc < 2) {
      fprintf(stderr,"USAGE: %s port\n", argv[0]);
      exit(1);
   } // Check usage & args

   // Set up the address struct for this process (the server)
   memset((char *)&serverAddress, '\0', sizeof(serverAddress));  // Clear out the address struct
   portNumber = atoi(argv[1]);                                   // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET;                           // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber);                   // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY;                   // Any address is allowed for connection to this process

   // Set up the socket
   listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);                                       // Create the socket
   if (listenSocketFD < 0) error("ERROR opening socket");                                  // Enable the socket to begin listening
   if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
      error("ERROR on binding");

   listen(listenSocketFD, NO_SOCKET);                         // Flip the socket on - it can now receive up to 5 connections

   while(1){
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress);         // Get the size of the address for the client that will connect
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnectionFD < 0) error("ERROR on accept");

      pid = fork();
      if(pid < 0) error("ERROR: Servor child process");
      else if(pid == 0){
         // Get the message from the client and display it
         memset(buffer, '\0', BUFFER_SIZE);
         charsRead = recv(establishedConnectionFD, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
         if (charsRead < 0) error("ERROR reading from socket");
         // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

         if(strcmp(buffer, "SYN")){
           close(establishedConnectionFD);
           error("SERVER: SYN FAIL");
         }
         else{
           charsRead = send(establishedConnectionFD, "ACK: SYN", 8, 0);
           if (charsRead < 0) error("ERROR writing to socket");
         }

         memset((char *)&serverAddress, '\0', sizeof(serverAddress));  // Clear out the address struct
         charsRead = recv(establishedConnectionFD, buffer, BUFFER_SIZE, 0);
         if (charsRead < 0) error("ERROR reading from socket");

         num_pkt = atoi(buffer);
			send(establishedConnectionFD, "ACK: pkt_num", 12, 0);


			int i = 0;
			char content[BUFFER_SIZE];
			char mykey[BUFFER_SIZE];
			char *cipher;

        for(i = 0; i < num_pkt; i ++){
				memset(content, '\0', BUFFER_SIZE);
				memset(mykey, '\0', BUFFER_SIZE);
				// receive text
				memset((char *)&serverAddress, '\0', sizeof(serverAddress));       // Clear out the address struct
				memset(buffer, '\0', sizeof(buffer));
				charsRead = recv(establishedConnectionFD, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
				strcat(content, buffer);
				send(establishedConnectionFD, "ACK: Content Get", 16, 0);

				// receive key
				memset((char *)&serverAddress, '\0', sizeof(serverAddress));       // Clear out the address struct
				memset(buffer, '\0', sizeof(buffer));
				charsRead = recv(establishedConnectionFD, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
				strcat(mykey, buffer);
				send(establishedConnectionFD, "ACK: Key Get", 12, 0);

				memset((char *)&serverAddress, '\0', sizeof(serverAddress));       // Clear out the address struct
				memset(buffer, '\0', sizeof(buffer));
				charsRead = recv(establishedConnectionFD, buffer, BUFFER_SIZE, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
				if(strcmp(buffer, "Req: Need Cipher")) error("SERVER: No Req for cipher\n");

				cipher = encoder(content, mykey);
				send(establishedConnectionFD, cipher, BUFFER_SIZE, 0);
        }
		  free(cipher);
      }
      else{
         close(establishedConnectionFD); // Close the existing socket which is connected to the client
         while(pid > 0) pid = waitpid(-1, &status, WNOHANG);
      }
      waitpid(-1, &status, WNOHANG);
   }
   close(listenSocketFD); // Close the listening socket
   return 0;
}
