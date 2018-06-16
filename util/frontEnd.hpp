#ifndef __FRONTEND_HPP__
#define __FRONTEND_HPP__

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
#include "frontEnd.hpp"
#include "dropboxUtil.hpp"

class FrontEnd
{
	protected:
		int side;
		int socketFd;

	public:
		int port;
		struct sockaddr_in socketAddress;
		struct sockaddr_in from;

		FrontEnd(int side);

		int create(int port);
		int login(std::string host, int port);

		bool sendDatagram(tDatagram datagram);
		tDatagram receiveDatagram();
		
		bool sendAck();
		bool waitAck();

		void finish();
};
#endif