/*
 * server.c
 *
 *  Created on: Oct 28, 2013
 *      Author: spencer
 */


/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tcp118.h"
#include "parse.h"
#include "response.h"

#define MAX_FILE_SIZE 1024

// void generateResponseMessage(char *filePath, int size, char *responseMessage);
// void fileToMessage(char *filePath, char *fileMessage);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     /*
     int sockfd, portno; //sockfd:portno -> server socket
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);	//create socket
     if (sockfd < 0)
        error("ERROR opening socket");
     memset((char *) &serv_addr, 0, sizeof(serv_addr));	//reset memory
     //fill in address info
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
              error("ERROR on binding");

     listen(sockfd,10);	//5 simultaneous connection at most


     fd_set active_fd_set, read_fd_set; //set for holding sockets
     int newsockfd; //socket representing client

     // Initialize the set of active sockets
     FD_ZERO(&active_fd_set);
     FD_SET(sockfd, &active_fd_set); //put sock to the set to monitor new connections

     while(1){
        int i = 0;
    	 read_fd_set = active_fd_set;
    	 if(select(FD_SETSIZE,&read_fd_set,NULL,NULL,NULL)<0){error("ERROR select()");}//FAIL with exit status 1 on error

    	 for(i =0; i < FD_SETSIZE; i++){
    		 if(FD_ISSET(i, &read_fd_set)) {
				 if(i == sockfd)//new connection request
				 {
					 newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					 if(newsockfd < 0) { error("ERROR accept()");}
					 FD_SET(newsockfd, &active_fd_set);
				 }
				 else{ // Data on connected socket

					 if (read_socket(i) < 0){
						 close(i);//close connection
						 FD_CLR(i, &active_fd_set);
					 }
				}
    		 }
    	 }
     }
     close(sockfd);
     
     */
     char test1[2] = {0, 1};
     uint16_t cs = 0;
     cs = checksum(test1, 2);
     printf("given:%x, checksum:%x\n",test1[1],cs);
     return 0;

}

int read_socket(int filedes) {
	char buffer[1024];
	memset(buffer, 0, 1024);	//reset memory
	int nread,nwrite;

	nread = read(filedes, buffer, 1024);
	if(nread < 0){
		error("read");
		return -1; //never reached. shuts up compiler.
	}
	else if(nread == 0){
		return -1;
	}
	else {
        /*
		//parse message here
		http_r* request;
        http_w* response;
		request = parseRequest(buffer);
        printRequest(request);
		char* filePath = request->URI;
		printf("Here is the file path: %s\n", filePath);
		response = generateResponseMessage(request);
		//reply to client
        printf("in server: body_size=%d, msg_size=%d\n",response->body_len, response->msg_len);
		nwrite = write(filedes,response->message,response->msg_len);
        freeResponse(response);
        // freeRequest(request);
		if (nwrite < 0) error("ERROR writing to socket");
		return 0;
        */
        int i = 0;
        char *file_s = "0123456789112345678921234567893123456789";
        int file_len = strlen(file_s);
        char ** packets = strToPackets(file_s);
        char packet_buffer[PACKET_SIZE+1];
        memset(packet_buffer,'\0', sizeof(char)*(PACKET_SIZE+1));

        printf("filelen=%02d\n", file_len);
        if(strncmp(buffer, "!c", 2) == 0){
           nwrite = write(filedes, "!s", 2);
            if (nwrite < 0) {
                error("ERROR writing to socket_1");
                return -1;
            } 
            for(i = 0; packets[i] != 0; i++){
                //print the file size and starting byte# of packetl
                sprintf(packet_buffer, "%02d%02d%s",file_len%100, (i*MAX_BODY_SIZE)%100, packets[i]);
                printf("packet len =%d...%s...\n",strlen(packet_buffer), packet_buffer);
                nwrite = write(filedes, packet_buffer, PACKET_SIZE);
                printf("after write");
                if (nwrite < 0) {
                error("ERROR writing to socket_1");
                return -1;
                }

                nread = read(filedes, buffer, 1024);
                while(strncmp(buffer, "#c", 2) != 0){
                    printf("waiting for ACK\n");
                }
            }
            nwrite = write(filedes, "$s", 2);
        }
        
        return 0;
	}
}

