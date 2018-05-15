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
				std::string modificationTime = device->user->getFileModificationTime(pathname);
				socket->send_file(pathname, modificationTime);
				break;
			}

			case LIST_SERVER:
				socket->send_list_server(device->user);
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
				std::string modificationTime = socket->receive_file(pathname);
				device->user->addFile(pathname, modificationTime);
				saveUsersServer(users);
				break;
			}

			case DELETE_TYPE: {
				std::string pathname = device->user->getFolderPath() + "/" + std::string(datagram.data);
				device->user->removeFile(pathname);
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
	say("Logout user: " + device->user->userid);
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
			say("New login: " + user->userid);
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
