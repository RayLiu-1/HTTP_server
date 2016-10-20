
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define BUFSIZE 4096
#define QUESIZE 4// maximum number of client connections
int ListenPort;
char DocumentRoot[200];
char WebPage[10][100];
char ContentType[20][2][100];
int KeepaliveTime ;

void *connection_handler(void *);
int set_config();

int main(int argc, char *argv[])
{
	if (set_config()!=0)
	{
		perror("set config error");
	}
	int lsfd, cnfd, *sock;
	struct sockaddr_in server,client;
	//Create socket
	lsfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lsfd == -1) {
		perror("Create sock failed");
		return 1;
	}
	puts("Socket created");

	//set socket address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(ListenPort);
	if (bind(lsfd, (struct sockaddr *)&server, sizeof(server)) < 0){
		perror("Bind failed");
		return 1;
	}
	puts("Socket bind");

	//Listen
	listen(lsfd, QUESIZE);
	puts("Listenning...");
	int csize = sizeof(client);
	while (cnfd = accept(lsfd, (struct sockaddr *)&client, (socklen_t*) &csize)) {
		puts("connection appected");
		int* pfd = (int*)malloc(sizeof(int));
		*pfd = cnfd;
		pthread_t new_thread;
		if (pthread_create(&new_thread, NULL, connection_handler, (void*)pfd) < 0)
		{
			perror("Create thread failed");
			return 1;
		}
	}
	perror("accept failed");
	return 0;
}

void *connection_handler(void *sockfd) {
	struct timeval timeout;
	timeout.tv_sec = KeepaliveTime;
	timeout.tv_usec = 0;
	int pipeline = 0;
	int n = 0;
	int cnfd = *(int*)sockfd;
	char buf[BUFSIZE];
	char sendbuf[BUFSIZE];
	int connection = 0;
	do{
		//printf("%d\n", timeout.tv_sec);
		if (setsockopt(cnfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			printf("unable to set socket");
		}
		//Send the message back to client
		n = recv(cnfd, buf, BUFSIZE, 0);
		//pch = strtok(buf)
		if (n == -1) {
			puts("Time out");
			fflush(stdout);
			break;
		}
		if (n == 0) {
			puts("Client disconnected");
			fflush(stdout);
			break;
		}
		char* pch;
		pch = strtok(buf," ");
		char filepath[200];
		char filename[100];
		char HTTP[100];
		char request[100];
		strcpy(request, pch);
		pch = strtok(NULL, " \n\r");
		strcpy(filename, pch);
		pch = strtok(NULL, " \n\r");
		if(strcmp(pch,"HTTP/1.1")==0)
			strcpy(HTTP, "HTTP/1.1");
		else if(strcmp(pch, "HTTP/1.0") == 0)
			strcpy(HTTP, "HTTP/1.0");
		while (pch != NULL) {
			pch = strtok(NULL, ": \n\r");
			if (strcmp(pch, "Connection") == 0) {
				pch = strtok(NULL, ": \r\n");
				if (pch != NULL) {
					if (strcmp(pch, "keep-alive") == 0) {
						connection = 1;
						break;
					}
				}
			}
		}
		
		if (strcmp(request, "GET") == 0 ) {
			if (strlen(filename) == 0 || filename[0] != '/') {
				strcpy(sendbuf, HTTP);
				strcpy(sendbuf, " 400 Bad Request\n");
				char error_message[400] = "<html><body>400 Bad Request Reason: Invalid URL:";
				strcat(error_message, filename);
				strcat(error_message, "\n</body></html>\n");
				char connection[40] = "Connection: Close\r\n\r\n";
				char length[40] = ""; 
				char type[40] = "Content-Type: text/html\n";
				sprintf(length, "Content-Length: %d\n", strlen(error_message));
				strcat(sendbuf, type);
				strcat(sendbuf, length);
				strcat(sendbuf, connection);
				strcat(sendbuf, error_message);
				write(cnfd, sendbuf, strlen(sendbuf)+1);
				puts(sendbuf);
				continue;
			}
			else if (strlen(filename) != 0 && filename[strlen(filename) - 1] == '/')
			{
				strcpy(filepath, DocumentRoot);
				strcat(filepath, "/");
				strcat(filepath, WebPage[0]);
				FILE* fp = fopen(filepath, "r");
				if (!fp) {
					perror("Open file failed");
				}
				strcpy(sendbuf, "");
				strcat(sendbuf, HTTP);
				strcat(sendbuf, " 200 OK\n");
				char type[40] = "Content-Type: text/html\n";
				strcat(sendbuf, type);
				char length[40] = "";
				fseek(fp, 0, SEEK_END);
				sprintf(length, "Content-Length: %d\n", (int) ftell(fp));
				rewind(fp);
				strcat(sendbuf, length);
				if (connection == 1) {
					strcat(sendbuf, "Connection: Keep-alive");
					connection == 1
				}
				strcat(sendbuf, "\r\n\r\n");
				write(cnfd, sendbuf, strlen(sendbuf) + 1);
				puts(sendbuf);
				memset(sendbuf, 0, BUFSIZE);
				while (fgets(sendbuf, BUFSIZE, (FILE*)fp)) {
					write(cnfd, sendbuf, strlen(cnfd)+1);
					printf("%d%s"sendbuf);
				}
			}
			else{
				/*strcpy(filepath, DocumentRoot);
				strcat(filepath, pch);*/
				if (strcmp(pch, "HTTP/1.1") == 0) {
					strcpy(sendbuf, "HTTP/1.1 404 Not Found \r\n\r\n<!DOCTYPE html>\n<html><body>404 Not Found Reason URL does not exist :");
				}
				else {
					strcpy(sendbuf, "HTTP/1.0 404 Not Found \r\n\r\n<!DOCTYPE html>\n<html><body>404 Not Found Reason URL does not exist :");

				}
				strcat(sendbuf, filename);
				strcat(sendbuf, "< / body>< / html>\r");
				write(cnfd, sendbuf, strlen(buf) + 1);
			}
			if (strcmp(HTTP, "HTTP/1.1") != 0 && strcmp(HTTP, "HTTP/1.0") != 0) {
				strcpy(buf,"HTTP/1.1 400 Bad Request\n<other-headers>\n<html><body>400 Bad Request Reason: Invalid HTTP-Version: <<req version>></body></html>\r");
				write(cnfd, buf, strlen(buf)+1);
			}
			else {
				if (strcmp(pch, "HTTP/1.1") == 0) {
					strcpy(sendbuf, "HTTP/1.1 404 Not Found \r\n\r\n<!DOCTYPE html>\n<html><body>404 Not Found Reason URL does not exist :");
				}
				else {
					strcpy(sendbuf, "HTTP/1.0 404 Not Found \r\n\r\n<!DOCTYPE html>\n<html><body>404 Not Found Reason URL does not exist :");

				}
				strcat(sendbuf, filename);
				strcat(sendbuf, "< / body>< / html>\r");
				write(cnfd, sendbuf, strlen(buf)+1);
			}
		}
	} while (connection==1); 
	
	free(sockfd);
	return 0;
}

int set_config()
{
	FILE *fp;
	fp = fopen("ws.conf", "r");
	if (fp == NULL) {
		perror("failed file opening");
		return 1;
	}
	char readBuf[BUFSIZE];

	int ntype = 0;
	int nIndex = 0;
	while (fgets(readBuf, BUFSIZE,(FILE*) fp)) {
		
		if (readBuf[0] == '#')
			continue;
		else {
			char * pch;
			pch = strtok(readBuf," ");
			if (strcmp(pch, "Listen") == 0) {
				pch = strtok(NULL, " ");
				ListenPort = atoi(pch);
				printf("Set listen port:%d\n", ListenPort);
			}
			else if (strcmp(pch, "DocumentRoot")==0) {
				pch = strtok(NULL, " ");
				strcpy(DocumentRoot, pch);
				if (strlen(DocumentRoot) != 0)
					DocumentRoot[strlen(DocumentRoot) - 1] = '\0';
			}
			else if (strcmp(pch, "DirectoryIndex") == 0) {
				while (pch != NULL) {
					pch = strtok(NULL, " ");
					if(pch!=NULL)
						strcpy(WebPage[nIndex++], pch);
				}
			}
			else if (pch[0] == '.') {
				strcpy(ContentType[ntype++][0], pch+1);
				pch = strtok(NULL, " ");
				strcpy(ContentType[ntype++][1], pch);
			}
			else if (strcmp(pch, "KeepaliveTime") == 0) {
				pch = strtok(NULL, " ");
				KeepaliveTime = atoi(pch);
			}
		}
	}
	return 0;
}
