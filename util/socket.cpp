#include "socket.hpp"
#include "file.hpp"

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
#include <cstddef>

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

bool Socket::sendAck()
{
	int n;
	char* buffer = (char*) calloc(1, BUFFER_SIZE);
	buffer[0] = ACK;

	if (this->side == SOCK_SERVER)
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
	else
		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr_in));

	if (n < 0)
	{
		std::cout << "ERROR sendto";
		return false;
	}
	return true;
}

bool Socket::waitAck()
{
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
	{
		std::cout << "ERROR recvfrom";
		return false;
	}
	if (buffer[0] == ACK)
		return true;
	
	return false;
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

	if (!this->waitAck())
	{
		std::cout << "ERROR ack miss";
		return false;
	}
	
	return true;
}

tDatagram Socket::receiveDatagram()
{
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
	this->sendAck();

	return datagram;
}

bool Socket::send_file(std::string pathname)
{
	File* fileHelper = new File(pathname);

	if (!fileHelper->exists())
		return false;

	tDatagram datagram;
	std::fstream file;
	int bytesSent = 0, bytesToRead = 0, fileSize = fileHelper->size;
	char buffer[MAX_DATA_SIZE];

	file.open(pathname.c_str(), std::ios::binary | std::ios::in);
	
	// Send filename
	datagram.type = BEGIN_FILE_TYPE;
	strcpy(datagram.data, fileHelper->getFilename().c_str());

	this->sendDatagram(datagram);

	// Send file
	datagram.type = FILE_TYPE;
	while(bytesSent < fileSize)
	{
		bzero(buffer, MAX_DATA_SIZE);
		bytesToRead = (fileSize - bytesSent < MAX_DATA_SIZE-1) ?  (fileSize - bytesSent) : MAX_DATA_SIZE-1;
		file.read(buffer, bytesToRead);
		strcpy(datagram.data, (char *) buffer);
		this->sendDatagram(datagram);
		bytesSent += bytesToRead;
	}

	// Send end of file
	datagram.type = END_DATA;
	this->sendDatagram(datagram);

	// Send last modification time
	datagram.type = MODIFICATION_TIME;
	strcpy(datagram.data, fileHelper->getTime('M').c_str());
	this->sendDatagram(datagram);

	file.close();
	delete fileHelper;

	return true;
}

// Used just by Client Side Socket
void Socket::get_file(std::string filename)
{
	tDatagram datagram;

	// Request file
	datagram.type = GET_FILE_TYPE;
	strcpy(datagram.data, filename.c_str());
	this->sendDatagram(datagram);

	datagram = this->receiveDatagram();
	if (datagram.type == BEGIN_FILE_TYPE)
	{
		this->receive_file(std::string(datagram.data));
	}
}


std::string Socket::receive_file(std::string filename)
{
	tDatagram datagram;
	std::fstream file;

	file.open(filename.c_str(), std::ios::binary | std::ios::out);
	datagram = this->receiveDatagram();
	while(datagram.type == FILE_TYPE && datagram.type != END_DATA)
	{
		file.write(datagram.data, strlen(datagram.data));
		datagram = this->receiveDatagram();
	}
	
	file.close();

	// get the modification time from the file
	datagram = this->receiveDatagram();
	if (datagram.type == MODIFICATION_TIME) {
		return std::string(datagram.data);
	}
	return "";
}

bool Socket::close_session()
{
	tDatagram datagram;

	datagram.type = CLOSE;
	return this->sendDatagram(datagram);
}

void Socket::finish()
{
	close(this->socketFd);
}

void Socket::list_server()
{
	tDatagram datagram;
	std::cout << "filename\tsize\tmodified\t\taccess\t\t\tcreation\n";

	datagram.type = LIST_SERVER;
	this->sendDatagram(datagram);

	datagram = this->receiveDatagram();
	while(datagram.type == FILE_INFO && datagram.type != END_DATA)
	{
		int pos = 0, posEnd;
		std::string fileInfo = std::string(datagram.data);
		posEnd = fileInfo.find("#");
		std::cout << fileInfo.substr(pos, posEnd-pos) << "\t ";

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		std::cout << fileInfo.substr(pos, posEnd-pos) << "\t";

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		std::cout << fileInfo.substr(pos, posEnd-pos) << "\t";

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		std::cout << fileInfo.substr(pos, posEnd-pos) << "\t";

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		std::cout << fileInfo.substr(pos, posEnd-pos) << "\t\n";

		datagram = this->receiveDatagram();
	}
}

void Socket::send_list_server(UserServer* user)
{
	for (std::list<File>::iterator f = user->files.begin(); f != user->files.end(); ++f)
	{
		tDatagram datagram;
		std::string fileInfo;
		File* file = new File();
		file->pathname = f->pathname;
		file->last_modified = f->last_modified;
		
		fileInfo = file->getFilename();
		fileInfo += "#";
		fileInfo += std::to_string(file->getSize());
		fileInfo += "#";
		fileInfo += std::string(file->last_modified);
		fileInfo += "#";
		fileInfo += std::string(file->getTime('A'));
		fileInfo += "#";
		fileInfo += std::string(file->getTime('C'));

		datagram.type = FILE_INFO;
		strcpy(datagram.data, fileInfo.c_str());
		this->sendDatagram(datagram);
	}

	tDatagram datagram;
	datagram.type = END_DATA;
	this->sendDatagram(datagram);
}