// #include "../util/communication.hpp"
#include <string>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "helper.hpp"

class Socket // : public Communication
{
	protected:
		int side;
		int port;
		int socketFd;
	public:
		struct sockaddr_in socketAddress;
		struct sockaddr_in from;
		Socket(int side);
		struct sockaddr_in createSocket(std::string host, int port);
		bool sendDatagram2(tDatagram datagram, struct sockaddr_in sockAddr);
		int login_server(std::string host, int port);
		bool sendMessage(std::string message);
		bool sendDatagram(tDatagram datagram);
		char* receiveMessage();
		tDatagram receiveDatagram();
		tDatagram receiveDatagram2();
		void send_file(std::string filename);
		void receive_file(std::string filename);
};
