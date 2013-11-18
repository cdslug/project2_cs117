/*
 * parse.c
 *
 *  Created on: Oct 28, 2013
 *      Author: spencer
 */


#include "parse.h"
#define BUFSIZE 8096


http_r * parseRequest(char* requestMessage) {
	http_r * request = malloc(sizeof(http_r));

	int i;
	if(strncmp(requestMessage,"GET ",4) && strncmp(requestMessage,"get ",4)){
		strncpy(request->method, requestMessage, 4);
	}
	for(i=0; i < BUFSIZE; i++){
		if(requestMessage[i] == ' '){
			strncpy(request->URI,requestMessage+4, i-4);
		}
	}
	
	if(strcmp(request->URI, "/")) {
		strcpy(request->URI, "/index.html");
	}
	strcpy(request->HTTP_version,"HTTP/1.1");
	return request;
}

void freeRequest(http_r* request){
	free(request->HTTP_version);
	free(request->URI);
	free(request->method);
	free(request);
	return;
}
void printRequest(http_r* request) {
	printf("Method: %s\n", request->method);
	printf("URI: %s\n", request->URI);
	printf("HTTP Version: %s\n", request->HTTP_version);

	return;
}

