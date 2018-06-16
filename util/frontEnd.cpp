#include "frontEnd.hpp"

struct sockaddr_in from;

FrontEnd::FrontEnd(int side)
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

int FrontEnd::login(std::string host, int port)
{
	if (port == 0){
		return 1;
	}

	this->port = port;
	struct hostent *server;

	server = gethostbyname(host.c_str());
	if (server == NULL) {
		fprintf(stderr,"ERROR: no such host\n");
		return 1;
	}

	this->socketAddress.sin_port = htons(this->port);    
	this->socketAddress.sin_family = AF_INET;   
	bzero(&(this->socketAddress.sin_zero), 8);
	this->socketAddress.sin_addr = *((struct in_addr *)server->h_addr);
	
	return 0;
}

int FrontEnd::create(int port)
{
	if (port == 0){
		return 1;
	}

	this->port = port;

	this->socketAddress.sin_port = htons(this->port);    
	this->socketAddress.sin_family = AF_INET;   
	bzero(&(this->socketAddress.sin_zero), 8);
	this->socketAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(this->socketFd, (struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr)) < 0) 
	{
		fprintf(stderr,"ERROR: bind host\n");
		return 1;
	}

	return 0;
}

bool FrontEnd::sendDatagram(tDatagram datagram)
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
		std::cout << "ERROR: sendto";
		return false;
	}

	if (!this->waitAck())
	{
		std::cout << "ERROR: ack miss";
		return false;
	}
	
	return true;
}

tDatagram FrontEnd::receiveDatagram()
{
	tDatagram datagram;
	socklen_t length = sizeof(struct sockaddr_in);
	char* buffer = (char*) calloc(1, BUFFER_SIZE);

	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
	{
		std::cout << "ERROR: recvfrom";
		datagram.type = ERROR;
		return datagram;
	}
	memcpy(&datagram, buffer, sizeof(datagram));
	this->sendAck();

	return datagram;
}

bool FrontEnd::sendAck()
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

bool FrontEnd::waitAck()
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

void FrontEnd::finish()
{
	close(this->socketFd);
}
