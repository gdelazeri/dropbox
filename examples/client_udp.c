#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"

int sockfd, n, lSize;
struct sockaddr_in serv_addr, from;
struct hostent *server;

void sendFile(char* filename, char* host)
{
	puts(filename);
	puts(host);
	FILE *fileptr;
	char buffer[BUFFER_SIZE];
	long filelen;
	unsigned int length;
	size_t result;

	// if (argc < 2) {
	// 	fprintf(stderr, "usage %s hostname\n", argv[0]);
	// 	exit(0);
	// }
	
	server = gethostbyname(host);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }	
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);  

	// fileptr = fopen(filename, "rb");
	// fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	// filelen = ftell(fileptr);             // Get the current byte offset in the file
	// rewind(fileptr);                      // Jump back to the beginning of the file
	// fread(buffer, sizeof(char), 4, fileptr); // Read in the entire file
	
	length = sizeof(struct sockaddr_in);
	n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
		printf("ERROR recvfrom");
	printf("Got: %s\n", buffer);

	n = sendto(sockfd, "oi", strlen("oi"), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if (n < 0) 
		printf("ERROR sendto");

	fclose(fileptr); // Close the file
	close(sockfd);
}

int login_server(char* host, int port)
{
	char returnSocket[255];

	server = gethostbyname(host);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

	tDatagram datagram;
	datagram.type = LOGIN_TYPE;
	strcpy(datagram.buffer, "oi");
	
	char buffer[sizeof(datagram)];
    memcpy(buffer, &datagram, sizeof(datagram));

	n = sendto(sockfd, buffer, 256, 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if (n < 0)
		printf("ERROR sendto");
	
	int length = sizeof(struct sockaddr_in);
	n = recvfrom(sockfd, returnSocket, 256, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
		printf("ERROR recvfrom");
	printf("Got an ack: %s\n", returnSocket);
}

int main(int argc, char *argv[])
{	
	login_server(ADDRESS, PORT);

	return 0;
}