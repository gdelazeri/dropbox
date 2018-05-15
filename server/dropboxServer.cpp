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
#include "device.hpp"

std::list<int> portsInUse;
std::list<UserServer*> users;

/* Utils */
void say(std::string message){
	std::cout << SERVER_NAME << message << "\n";
}
				
void sendThread(Socket* socket, Device* device)
{
	tDatagram datagram;
	while(device->connected)
	{
		datagram = socket->receiveDatagram();
		switch (datagram.type)
		{
			case GET_FILE_TYPE: {
				std::string pathname = device->user->getFolderPath() + "/" + std::string(datagram.data);
				std::string modificationTime = device->user->getFileTime(pathname, 'M');
				std::string accessTime = device->user->getFileTime(pathname, 'A');
				std::string creationTime = device->user->getFileTime(pathname, 'C');
				socket->send_file(pathname, modificationTime, accessTime, creationTime);
				break;
			}

			case LIST_SERVER:
				socket->send_list_server(device->user);
				break;

			case LIST_DELETED:
				socket->sendListDeleted(device->user);
				break;

			case CLOSE:
				device->disconnect();
				break;
		}
	}

	socket->finish();
	portsInUse.remove(socket->port);
	delete socket;
}

void receiveThread(Socket* socket, Device* device)
{
	tDatagram datagram;
	
	while(device->connected)
	{
		datagram = socket->receiveDatagram();
		switch (datagram.type)
		{
			case BEGIN_FILE_TYPE: {
				std::string pathname = device->user->getFolderPath() + "/" + std::string(datagram.data);
				std::string times = socket->receive_file(pathname);
				int posEnd = times.find("#"), pos;
				std::string modificationTime = times.substr(0, posEnd);
				pos = posEnd+1;
				posEnd = times.find("#", posEnd+1);
				std::string accessTime = times.substr(pos, posEnd - pos);
				std::string creationTime = times.substr(posEnd+1, times.length() - posEnd);
				device->user->addFile(pathname, modificationTime, accessTime, creationTime);
				saveUsersServer(users);
				break;
			}

			case DELETE_TYPE: {
				std::string pathname = device->user->getFolderPath() + "/" + std::string(datagram.data);
				device->user->removeFile(pathname);
				device->user->deleted.push_back(std::make_pair(std::string(datagram.data), 2));
				saveUsersServer(users);
				break;
			}

			case CLOSE:
				device->disconnect();
				break;
		}
	}

	socket->finish();
	portsInUse.remove(socket->port);
	delete socket;
	say("Logout: " + device->user->userid);
}

UserServer* searchUser(std::string userid)
{
	for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it){
    	if ((*it)->userid == userid)
		{
			(*it)->createDir();
			return (*it);
		}
	}
	UserServer* newUserServer = new UserServer();
	newUserServer->userid = userid;
	newUserServer->createDir();
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
	mainSocket->createSocket(SERVER_PORT);
	say("Server Online");

	while(true) 
	{
		UserServer* user = new UserServer();
		datagram = mainSocket->receiveDatagram();

		if (datagram.type == LOGIN)
		{
			user = searchUser(std::string(datagram.data));
			say("Login: " + user->userid);
			saveUsersServer(users);
			Device* newDevice = new Device(user);

			if (newDevice->connect()) {
				Socket* receiverSocket = new Socket(SOCK_SERVER);
				Socket* senderSocket = new Socket(SOCK_SERVER);

				int portReceiver = createNewPort(portsInUse);
				int portSender = createNewPort(portsInUse);
				receiverSocket->createSocket(portReceiver);
				senderSocket->createSocket(portSender);

				std::string ports = std::to_string(portReceiver)+std::to_string(portSender);
				datagram.type = NEW_PORTS;
				strcpy(datagram.data, (char *) ports.c_str());
				mainSocket->sendDatagram(datagram);

				std::thread rcv(receiveThread, receiverSocket, newDevice);
				std::thread snd(sendThread, senderSocket, newDevice);
				
				rcv.detach();	
				snd.detach();
			}
			else {
				datagram.type = ERROR;
				strcpy(datagram.data, "Too many devices for this user. Try again later.");
				mainSocket->sendDatagram(datagram);
			}
		}
	}

	return 0;
}
