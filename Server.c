
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 4096
#define QUESIZE 32// maximum number of client connections
int ListenPort;
char DocumentRoot[100];
char DirectoryIndex[100];
char ContentType[20][2][100];
int KeepaliveTime;

void *connection_handler(void *);
int set_config();

int main(int argc, char *argv[])
{
	if (set_config()==0)
	{
		perror("set config error");
	}
	int msock, cnfd, n;
	
	char client[QUESIZE];
	return 0;
}

int set_config()
{
	FILE *fp;
	fp = fopen("ws.conf", "r");
	char readBuf[200];
	while (fgets(readBuf,200,(FILE*) fp)) {
		if (readBuf[0] == '#')
			continue;
		else {
			char * pch;
			pch = strtok(readBuf," ");
			if (strcmp(pch, "ListenPort") == 0) {
				ListenPort = atoi(pch);
				puts(ListenPort);
			}
		}
	}
	return 1;
}