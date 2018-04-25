#include "socket.hpp"
#include "helper.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>

#define BUFFER_SIZE 256

struct sockaddr_in socketAddress;
struct sockaddr_in from;

Socket::Socket(int side)
{
	if (side != SOCK_CLIENT && side != SOCK_SERVER)
	{
		fprintf(stderr, "ERROR: invalid socket side\n");
		exit(1);
	}

	this->side = side;

	if ((this->socketFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "ERROR: open socket\n");
		exit(1);
	}
}

int Socket::login_server(std::string host, int port)
{
	this->port = port;
	struct hostent *server;

	socketAddress.sin_port = htons(this->port);    
	socketAddress.sin_family = AF_INET;   
	bzero(&(socketAddress.sin_zero), 8);
	
	if (this->side == SOCK_CLIENT)
	{
		server = gethostbyname(host.c_str());
		if (server == NULL) {
			fprintf(stderr,"ERROR: no such host\n");
			return 1;
		}
		socketAddress.sin_addr = *((struct in_addr *)server->h_addr);
	}
	else
	{
		socketAddress.sin_addr.s_addr = INADDR_ANY;
		if (bind(this->socketFd, (struct sockaddr *) &socketAddress, sizeof(struct sockaddr)) < 0) 
		{
			fprintf(stderr,"ERROR: bind host\n");
			return 1;
		}
	}

	return 0;
}

bool Socket::sendMessage(std::string message)
{
	int n;

	if (this->side == SOCK_SERVER)
		n = sendto(this->socketFd, message.c_str(), strlen(message.c_str()), 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
	else
		n = sendto(this->socketFd, message.c_str(), strlen(message.c_str()), 0, (const struct sockaddr *) &socketAddress, sizeof(struct sockaddr_in));

	if (n < 0)
	{
		std::cout << "ERROR sendto";
		return false;
	}
	return true;
}

std::string Socket::receiveMessage()
{
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);
	
	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
	{
		std::cout << "ERROR recvfrom";
		return std::string("");
	}
	return std::string(buffer);
}