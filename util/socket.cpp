#include "socket.hpp"

#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <fstream>

struct sockaddr_in socketAddress;
struct sockaddr_in from;

#include <time.h>

void now(){
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    std::cout << buf << "\n";
}
    

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
	if (port == 0){
		port = rand() % 2000;
	}

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

bool Socket::sendDatagram(tDatagram datagram)
{
	std::cout << "sendDatagram\n";
	std::cout << "port: " << this->port << '\n';
	std::cout << "sin_port: " << socketAddress.sin_port << '\n';
	
	int n;
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	memcpy(buffer, &datagram, sizeof(datagram));
	if (this->side == SOCK_SERVER) {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
	}
	else {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &socketAddress, sizeof(struct sockaddr_in));
	}

	if (n < 0)
	{
		std::cout << "ERROR sendto";
		return false;
	}
	return true;
}

char* Socket::receiveMessage()
{
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
	{
		std::cout << "ERROR recvfrom";
		return buffer;
	}
	return buffer;
}

tDatagram Socket::receiveDatagram()
{
	std::cout << "receiveDatagram\n";
	std::cout << "port: " << this->port << '\n';
	std::cout << "sin_port: " << socketAddress.sin_port << '\n';

	tDatagram datagram;
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
	{
		std::cout << "ERROR recvfrom";
		datagram.type = ERROR;
		return datagram;
	}
	memcpy(&datagram, buffer, sizeof(datagram));
	return datagram;
}

void Socket::send_file(std::string filename)
{
	tDatagram datagram;
	datagram.type = FILE_TYPE;

	char* fileBuffer = (char*) calloc(1, MAX_DATA_SIZE);

	std::fstream file;
	file.open(filename.c_str(), std::ios::binary | std::ios::in);
	
	file.seekg (0, file.end);
	int fileSize = file.tellg();
	
	file.seekg (0, file.beg);

	if (fileSize <= MAX_DATA_SIZE)
	{
		bzero(fileBuffer, MAX_DATA_SIZE);
		file.read(fileBuffer, fileSize);
		strcpy(datagram.data, (char *) fileBuffer);
		this->sendDatagram(datagram);
	}
	else
	{
		int bytesSent = 0;
		int bytesToRead = 0;
		while(bytesSent < fileSize)
		{
			bytesToRead = (fileSize - bytesSent < MAX_DATA_SIZE) ?  (fileSize - bytesSent) : MAX_DATA_SIZE;
			bzero(fileBuffer, MAX_DATA_SIZE);
			file.read(fileBuffer, bytesToRead);
			strcpy(datagram.data, (char *) fileBuffer);
			this->sendDatagram(datagram);
			bytesSent += bytesToRead;
		}
	}
}