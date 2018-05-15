#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

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
#include "dropboxUtil.hpp"

class Socket
{
	protected:
		int side;
		int socketFd;

	public:
		int port;
		struct sockaddr_in socketAddress;
		struct sockaddr_in from;

		Socket(int side);

		int createSocket(int port);
		int login_server(std::string host, int port);

		bool sendDatagram(tDatagram datagram);
		tDatagram receiveDatagram();
		
		std::string get_file(std::string filename, std::string path);
		bool send_file(std::string pathname, std::string modificationTime);
		std::string receive_file(std::string filename);
		
		std::list<File> list_server();
		void send_list_server(UserServer* user);

		std::list<std::string> listDeleted();
		void sendListDeleted(UserServer* user);

		void deleteFile(std::string filename);

		bool sendAck();
		bool waitAck();

		void finish();
		bool close_session();
};
#endif