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

// struct sockaddr_in this->socketAddress;
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
	if (port == 0){
		port = rand() % 2000;
	}

	this->port = port;
	struct hostent *server;

	this->socketAddress.sin_port = htons(this->port);    
	this->socketAddress.sin_family = AF_INET;   
	bzero(&(this->socketAddress.sin_zero), 8);

	if (this->side == SOCK_CLIENT)
	{
		server = gethostbyname(host.c_str());
		if (server == NULL) {
			fprintf(stderr,"ERROR: no such host\n");
			return 1;
		}
		this->socketAddress.sin_addr = *((struct in_addr *)server->h_addr);
	}
	else
	{
		this->socketAddress.sin_addr.s_addr = INADDR_ANY;
		if (bind(this->socketFd, (struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr)) < 0) 
		{
			fprintf(stderr,"ERROR: bind host\n");
			return 1;
		}
	}
	return 0;
}

struct sockaddr_in Socket::createSocket(std::string host, int port)
{
	struct sockaddr_in sockAddr;
	struct hostent *server;

	this->port = port;

	sockAddr.sin_port = htons(this->port);    
	sockAddr.sin_family = AF_INET;   
	bzero(&(sockAddr.sin_zero), 8);

	if (this->side == SOCK_CLIENT)
	{
		server = gethostbyname(host.c_str());
		if (server == NULL) {
			fprintf(stderr,"ERROR: no such host\n");
			return sockAddr;
		}
		sockAddr.sin_addr = *((struct in_addr *)server->h_addr);
	}
	else
	{
		sockAddr.sin_addr.s_addr = INADDR_ANY;
		if (bind(this->socketFd, (struct sockaddr *) &sockAddr, sizeof(struct sockaddr)) < 0) 
		{
			fprintf(stderr,"ERROR: bind host\n");
			return sockAddr;
		}
	}

	return sockAddr;
}

bool Socket::sendMessage(std::string message)
{
	int n;

	if (this->side == SOCK_SERVER)
		n = sendto(this->socketFd, message.c_str(), strlen(message.c_str()), 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
	else
		n = sendto(this->socketFd, message.c_str(), strlen(message.c_str()), 0, (const struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr_in));

	if (n < 0)
	{
		std::cout << "ERROR sendto";
		return false;
	}
	return true;
}

bool Socket::sendDatagram(tDatagram datagram)
{
	int n;
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	memcpy(buffer, &datagram, sizeof(datagram));
	if (this->side == SOCK_SERVER) {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
	}
	else {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr_in));
	}

	if (n < 0)
	{
		std::cout << "ERROR sendto";
		return false;
	}
	return true;
}

bool Socket::sendDatagram2(tDatagram datagram, struct sockaddr_in sock)
{
	struct sockaddr_in fr;
	int n;
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	memcpy(buffer, &datagram, sizeof(datagram));
	if (this->side == SOCK_SERVER) {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &fr, sizeof(struct sockaddr));
	}
	else {
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &sock, sizeof(struct sockaddr_in));
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
	tDatagram datagram;
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);
	struct sockaddr_in fr;

	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &fr, &length);
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
	std::fstream file;
	int bytesSent = 0;
	int bytesToRead = 0;
	
	char fileBuffer[MAX_DATA_SIZE];
	file.open(filename.c_str(), std::ios::binary | std::ios::in);
	
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	file.seekg(0, file.beg);


	// Send filename
	datagram.type = FILENAME_TYPE;
	strcpy(datagram.data, filename.c_str());
	std::cout << "getchar()\n";
	getchar();
	this->sendDatagram(datagram);
	datagram.type = FILE_TYPE;
	while(bytesSent < fileSize)
	{
		bzero(fileBuffer, MAX_DATA_SIZE);
		bytesToRead = (fileSize - bytesSent < MAX_DATA_SIZE) ?  (fileSize - bytesSent) : MAX_DATA_SIZE;
		file.read(fileBuffer, bytesToRead);
		strcpy(datagram.data, (char *) fileBuffer);
		std::cout << "getchar()\n";
		getchar();
		this->sendDatagram(datagram);
		bytesSent += bytesToRead;
	}
	std::cout << "getchar()\n";
	getchar();
	datagram.type = END_FILE_TYPE;
	this->sendDatagram(datagram);
	file.close();
}

void Socket::receive_file()
{
	tDatagram datagram;
	std::fstream file;
	
	datagram = this->receiveDatagram();
	
	if (datagram.type == FILENAME_TYPE)
		file.open("server/tt.txt", std::ios::binary | std::ios::out);
	else
		return;

	datagram = this->receiveDatagram();
	while(datagram.type == FILE_TYPE && datagram.type != END_FILE_TYPE)
	{
		file.write(datagram.data, strlen(datagram.data));
		std::cout << "data: " << strlen(datagram.data) << '\n';
		datagram = this->receiveDatagram();
	}
	file.close();
}