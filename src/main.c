/*
 ============================================================================
 Name        : main.c
 Author      : Andres
 Version     :
 Copyright   : Your copyright notice
 Description : Recibe y envia texto al segundo proceso
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <fcgi_stdio.h>

#define SERIAL_PORT "/dev/ttyGS0"
#define SOCKET_PATH "/tmp/fastcgi_chat.socket"

void error(const char *);

int main(void) {
	char *input = NULL;
	char *data = NULL;
	int content_length;

	int sockfd, servlen,n;
	struct sockaddr_un  serv_addr;
	char buf[82];

	/* Abrimos el socket */
	bzero((char *)&serv_addr,sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, SOCKET_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);


	if ((sockfd = socket(AF_UNIX, SOCK_STREAM,0)) < 0)
		error("Creating socket");
	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
		error("Connecting");

	fcntl(sockfd, F_SETFL, O_NONBLOCK); // Setiamos como non blocking al socket


	while (FCGI_Accept() >= 0) {

		content_length = atoi( getenv("CONTENT_LENGTH") );

		input = (char *) malloc( (content_length + 1)*sizeof(char));
		FCGI_fread(input, sizeof(char), content_length, stdin);
		input[content_length] = 0;


		data = strtok(input,"=");
		if( strcmp( data, "msg") == 0)
		{
			FCGI_printf("Content-type: text/html\n\n");

			data = strtok(NULL, "=");
			if( data != NULL && data[0] != '\n')
			{
				write(sockfd, data, strlen(data));
				FCGI_printf("=> %s<br>",data);
			}
			if( (n=read(sockfd, buf, 80)) > 0 )
			{
				FCGI_printf("<= %s<br>", buf);
				memset(buf, 0, 82);
			}
			else if ( errno != EAGAIN ) {
				error("read socket");
			}
		}
		free(input);
	}// while (FCGI_Accept() >= 0)

	close(sockfd);
	return 0;
}


void error(const char *msg)
{
    perror(msg);
    exit(0);
}
