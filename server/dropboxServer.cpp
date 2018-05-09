// #include "database.hpp"
// #include "foldermanager.hpp"
// #include "servercomm.hpp"
// #include "serveruser.hpp"
// #include "device.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <thread>
#include <list>
#include <algorithm>
#include <string.h>

#include "socket.hpp"

std::list<int> portsInUse;
std::list<UserServer*> users;

/* Utils */
void say(std::string message){
	std::cout << SERVER_NAME << message << "\n";
}
				
void sendThread(Socket* socket, UserServer* user)
{
	tDatagram datagram;
	say("Send thread");
	
	while(user->logged_in)
	{
		datagram = socket->receiveDatagram();
		switch (datagram.type)
		{
			case GET_FILE_TYPE: {
				std::string pathname = "server/" + user->getFolderName() + "/" + std::string(datagram.data);
				std::string modificationTime = user->getFileModificationTime(pathname);
				socket->send_file(pathname, modificationTime);
				break;
			}

			case LIST_SERVER:
				socket->send_list_server(user);
				break;

			case CLOSE:
				user->logged_in = 0;
				break;
		}
	}

	socket->finish();
	portsInUse.remove(socket->port);
	delete socket;
}

void receiveThread(Socket* socket, UserServer* user)
{
	tDatagram datagram;
	say("Receive thread");
	
	while(user->logged_in)
	{
		datagram = socket->receiveDatagram();
		switch (datagram.type)
		{
			case BEGIN_FILE_TYPE: {
				std::string pathname = user->getFolderPath() + "/" + std::string(datagram.data);

				std::string modificationTime = socket->receive_file(pathname);
				user->addFile(pathname, modificationTime);
				saveUsersServer(users);
				break;
			}
			case CLOSE:
				user->logged_in = 0;
				break;
		}
	}

	socket->finish();
	portsInUse.remove(socket->port);
	delete socket;
}

UserServer* searchUser(std::string userid)
{
	for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it){
    	if ((*it)->userid == userid)
		{
			(*it)->logged_in = 1;
			(*it)->createDir("server");
			return (*it);
		}
	}
	UserServer* newUserServer = new UserServer();
	newUserServer->userid = userid;
	newUserServer->logged_in = 1;
	newUserServer->createDir("server");
	users.push_back(newUserServer);

	return newUserServer;
}

int main(int argc, char* argv[])
{
	tDatagram datagram;
	users = loadUsersServer();

	if(argc != 1)
	{
		std::cout << "Usage:\n\t ./dropboxServer\n";
		exit(1);
	}

    Socket* mainSocket = new Socket(SOCK_SERVER);
	mainSocket->login_server(std::string(), SERVER_PORT);
	say("server online");

	while(true) 
	{
		UserServer* user = new UserServer();
		datagram = mainSocket->receiveDatagram();

		if (datagram.type == LOGIN)
		{
			user = searchUser(std::string(datagram.data));
			say("new login: " + user->userid);
			saveUsersServer(users);
		
			Socket* receiverSocket = new Socket(SOCK_SERVER);
			Socket* senderSocket = new Socket(SOCK_SERVER);

			int portReceiver = createNewPort(portsInUse);
			int portSender = createNewPort(portsInUse);
			receiverSocket->login_server(std::string(), portReceiver);
			senderSocket->login_server(std::string(), portSender);

			std::string ports = std::to_string(portReceiver)+std::to_string(portSender);
			datagram.type = NEW_PORTS;
			strcpy(datagram.data, (char *) ports.c_str());
			mainSocket->sendDatagram(datagram);

			std::thread rcv(receiveThread, receiverSocket, user);
			std::thread snd(sendThread, senderSocket, user);
			
			rcv.detach();	
			snd.detach();
		}
	}

	return 0;
}
