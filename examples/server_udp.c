#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include "util.h"

int main(int argc, char *argv[])
{
	FILE *fileptr;
	int sockfd, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buf[BUFFER_SIZE];
		
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);
	 
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
		printf("ERROR on binding");
	
	clilen = sizeof(struct sockaddr_in);
	printf("server online\n");
	tDatagram datagram;

	while (1) {
		/* receive from socket */
		bzero(buf, 4);    
		n = sendto(sockfd, "teste", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if (n  < 0) 
			printf("ERROR on sendto");

		printf("enviado");
		n = recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen);
		if (n < 0) 
			printf("ERROR on recvfrom");
		printf("datagram buffer: %s", buf);
		printf("recebido");
		// memcpy(&datagram, buf, sizeof(datagram));

		// char response[10];
		// if (datagram.type == 1)
		// 	strcpy(response, "login");
		// if (datagram.type == 2)
		// 	strcpy(response, "tipo 2");
		
	}
	
	close(sockfd);
	return 0;
}
