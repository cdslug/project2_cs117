#include "response.h"



http_w * responseInit()
{
	int i = 0;

	http_w * response = (http_w*)malloc(sizeof(http_w));
	if(!response){error("ERROR malloc response");}

	response->header_lines = (char**)malloc(sizeof(char*) * (NUM_HEADER_ELEMENTS + 1));
	if(!response->header_lines){error("ERROR malloc response->header_lines");}
	memset(response->header_lines, '\0',sizeof(char*) * NUM_HEADER_ELEMENTS + 1);

	response->header_fields = (char**)malloc(sizeof(char*) * (NUM_HEADER_ELEMENTS +1));
	if(!response->header_fields){error("ERROR malloc response->header_fields");}
	response->header_fields[HTTP_VERSION] 	= strdup("");
	response->header_fields[STATUS] 		= strdup(" ");
	response->header_fields[STATUS_STR] 	= strdup(" ");
	response->header_fields[CONNECTION] 	= strdup("\nConnection: ");
	response->header_fields[DATE] 			= strdup("\nDate: ");
	response->header_fields[SERVER] 		= strdup("\nServer: ");

	response->header_fields[CONTENT_TYPE] 	= strdup("\nContent-Type: ");
	response->header_fields[CONTENT_LENGTH] = strdup("\nContent-Length: ");
	response->header_fields[LAST_MODIFIED] 	= strdup("\nLast-Modified: ");
	response->header_fields[BODY] 			= strdup("\n\n");
	
	response->body_len = 0;
	response->msg_len = 0;
	return response;
}
//code found on stackoverflow
//http://stackoverflow.com/questions/7548759/generate-a-date-string-in-http-response-date-format-in-c
//SAMPLE: "Tue, 15 Nov 2010 08:12:31 GMT"
//
//allocate memory in function
//return pointer to cstring
char * dateToStr()
{
  char *buf = malloc(sizeof(char) * (30));
  if(!buf){error("ERROR malloc buf");}
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  strftime(buf, 30, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  // printf("in dateToStr: date=%s\n",buf);
  return buf;
}

//max length of a number is ceiling of log10(2^64-1)+1 = 21
//didn't end up using the method that needed this info
//still seems useful enough to keep
char * numToStr(size_t num)
{
	
	char buf[21];
	memset(buf,'\0',21);
	sprintf(buf,"%d",num);
	return strdup(buf);
}

char * getContentType(const char * URI)
{
	char * contentType = NULL;
	char * ext = strrchr(URI,'.');

	if(ext == NULL)
	{
		//todo check
		contentType = strdup("empty");
	}
	else if(strcmp(ext,".html") == 0)
	{
		contentType = strdup("text/html");
	}
	else if (strcmp(ext, ".css") == 0)
	{
		contentType = strdup("text/css");
	}
	else if (strcmp(ext,".jpg") == 0 || strcmp(ext,".jpeg") == 0)
	{
		contentType = strdup("image/jpeg");
	}
	else if (strcmp(ext,".gif") == 0)
	{
		contentType = strdup("image/gif");
	}
	else
	{
		//todo, I don't know, maybe set a status message
		contentType = strdup("text/html");
	}
	return contentType;
}

char * getFileDate(FILE * fp)
{
	if(fp == NULL)
	{
		return strdup("");
	}
	else
	{
		//todo, this is a dummy for consistancy, otherwise freeing this will crash
		return strdup("TODO");
	}
}
//the convention I used allocates member strings in helper functions
//the helper functions pass back a pointer to allocated memory
//this memory needs to be cleaned up
void getFileInfo(const http_r * request, http_w * response)
{
	// printf("begin gFI\n");

	char * fileBody = NULL;
	size_t fileSize = 0;

	FILE *filePointer = NULL;
	filePointer = fopen(&(request->URI[1]), "r");
	// printf("in gFI: file opened\n");

	if(filePointer == NULL)
	{
		printf("ERROR, could not open file \"%s\"\n",request->URI);
		fileBody = strdup("");
	}
	else
	{	
		int remaining = -1;
		size_t pos = 0;
		 

		//find file size
		//from stackoverflow
		//http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
		fseek(filePointer, 0L, SEEK_END);
		fileSize = ftell(filePointer);
		fseek(filePointer, 0L, SEEK_SET);
		//end stackoverflow
		printf("in gFI: filesize: %d\n",fileSize);
		remaining = fileSize;

		fileBody = malloc(sizeof(char)*(fileSize+1));
		if(!fileBody){error("ERROR malloc fileBody");}
		// memset(fileBody, 0, sizeof(char)*(fileSize+1));
		
		//this should concatenate
		//todo check: this included the EOF
	    pos = fread(fileBody, 1, fileSize, filePointer);
	    if(pos != fileSize) {printf("in gFI, did not read entire file\n");}
	    /*
	    while ( remaining > 0)
	    {
	    	// printf("in gFI: fileSize = %d, pos = %d, remaining = %d\n",fileSize, pos, remaining);
	      	fileBody[pos] = fgetc (filePointer);
	      	
	      		pos++;// = strlen(fileBody);
	      		remaining = fileSize-pos;
	    }
	    */

	    // printf("in gFI: fileSize = %d, pos = %d, remaining = %d\n",fileSize, pos, remaining);
	    // fclose (filePointer);//relocated to bottom so unfound file can be detected	    
	}
	// printf("in gFI: fileBody:\n%s\n***END BODY***\n", fileBody);
	// printf("in gFI: strlen(fileBody) = %d\n", strlen(fileBody));
	
	response->header_lines[CONTENT_LENGTH] 	= numToStr(fileSize);
	response->body_len = fileSize;
	response->header_lines[LAST_MODIFIED] 	= getFileDate(filePointer);
	response->header_lines[BODY] 			= fileBody;
	
	if(filePointer == NULL){
		response->header_lines[CONTENT_TYPE] = getContentType("");
	}
	else
	{
		response->header_lines[CONTENT_TYPE] = getContentType(request->URI);
		fclose(filePointer);
	}
}

char * getStatusStr(int status)
{
	if(status == 200)
	{
		return strdup("OK");
	}
	else if(status == 404)
	{
		return strdup("Not Found");
	}
	else
	{
		return strdup("Unsupport Status");
	}
}

//allocate all memory in functions before setting response pointers
http_w * generateResponseInfo(http_r * request)
{
	// printf("begin gRI\n");
	http_w * response = responseInit();

	int status = 0;

	// printf("in gRI: call gFI\n");
	getFileInfo(request, response);
	// printf("in gRI: BODY=%s\n",response->header_lines[BODY]);
	//assume that if 
	if(strlen(response->header_lines[BODY]) == 0)
	{
		status = 404;
		// printf("in gRI: body unfilled\n");
	}
	else
	{
		//assume correct
		status = 200;
		/*
		printf("in gRI: body filled, status=%s, stat_str=%s\n",
				response->header_lines[STATUS], 
				response->header_lines[STATUS_STR]);
		*/
	}
	// printf("in gRI, sizeof(URI)=%d\n",sizeof(request->URI));
	if(!isprint(request->HTTP_version[strlen(request->HTTP_version)-1]))
	{
		// printf("in gRI, version has non-printable char");
		request->HTTP_version[strlen(request->HTTP_version)-1] = '\0';
	}
	response->header_lines[HTTP_VERSION] 	= strdup(request->HTTP_version);//todo
	// response->header_lines[HTTP_VERSION] 	= strdup("HTTP/1.1");//todo
	response->header_lines[STATUS] 			= numToStr(status);
	response->header_lines[STATUS_STR] 		= getStatusStr(status); 
	response->header_lines[CONNECTION] 		= strdup("close");//todo
	response->header_lines[DATE] 			= dateToStr();
	response->header_lines[SERVER] 			= strdup("CS118/0.0.1");

	// printf("in gRI: response status = %s\n",response->header_lines[STATUS]);
	return response;
}

http_w * generateResponseMessage(http_r *request)
{
	// printf("begin gRM\n");
	http_w * response = NULL;
	int i = 0; 
	int pos = 0;
	int len = 0;
	
	// printf("in gRM: call gRI\n");
	response = generateResponseInfo(request);
	// printf("in gRM: info returned\n");

	for(i = 0; i < NUM_HEADER_ELEMENTS; i++)
	{
		if(i == BODY)
		{
			len += strlen(response->header_fields[i]);
			len += response->body_len;

		}
		else
		{
			len += strlen(response->header_fields[i]);
			len += strlen(response->header_lines[i]);
		}
	}
	response->msg_len = len;
	// printf("in gRM: info length=%d\n",len+1);
	response->message = malloc(sizeof(char) * (len+1));
	if(!response->message){error("ERROR malloc responseMessage");}
	memset(response->message, '\0', (len+1));

	printf("http-version=%s\n",response->header_lines[HTTP_VERSION]);
	pos = 0;
	for(i = 0; i < NUM_HEADER_ELEMENTS; i++)
	{
		// printf("responseMessage[%d]=%s\n",pos,responseMessage);
		sprintf(&(response->message[pos]),
				"%s%s",
				response->header_fields[i],
				response->header_lines[i]);
		pos = strlen(response->message);//error if anythig is after the body
	}
	printf("\nin gRM: ***response->message***\n%s\n***END RESPONSE MESSAGE***\n", response->message);


//free the allocated space in the response variable
// freeResponse(response);

return response;
}

void freeResponse(http_w * response)
{
	int i = 0;
	//would have been more elegant if I used a string array
	for(i = 0; i < NUM_HEADER_ELEMENTS; i++)
	{
		free(response->header_lines[i]);
		free(response->header_fields[i]);
	}
	free(response->message);
	free(response);
}
