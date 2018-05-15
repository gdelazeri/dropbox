#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <string>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "helper.hpp"

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
		bool close_session();
		std::list<File> list_server();
		void send_list_server(UserServer* user);
		void deleteFile(std::string filename);

		bool sendAck();
		bool waitAck();

		void finish();
};
#endif