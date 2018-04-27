#ifndef __CLIENTCOMM_HPP__
#define __CLIENTCOMM_HPP__

// #include "../util/communication.hpp"
#include <string>
#include <netinet/in.h>
#include "helper.hpp"

class Socket // : public Communication
{
	protected:
		int side;
		int port;
		int socketFd;
	public:
		Socket(int side);
		int login_server(std::string host, int port);
		bool sendMessage(std::string message);
		bool sendDatagram(tDatagram datagram);
		char* receiveMessage();
		tDatagram receiveDatagram();
		// bool connectServer(std::string serverIp, int serverPort);
};

#endif
