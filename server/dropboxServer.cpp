// #include "database.hpp"
// #include "foldermanager.hpp"
// #include "servercomm.hpp"
// #include "serveruser.hpp"
// #include "device.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <iterator>
#include <vector>
#include <thread>
#include <list>
#include <algorithm>
#include <string.h>

#include "socket.hpp"

std::list<int> portsInUse;

/* Utils */
void say(std::string message){
	std::cout << SERVER_NAME << message << "\n";
}

int createNewPort(){
	int newPort;
	std::list<int>::iterator findIter;
	
	while(1) {
		newPort = std::rand() % 2000 + SERVER_PORT;
		findIter = std::find(portsInUse.begin(), portsInUse.end(), newPort);
		if (*findIter == (int) portsInUse.size()) {
			portsInUse.push_back(newPort);
			return newPort;
		}
	}
}

int main(int argc, char* argv[])
{
	tDatagram datagram;

	if(argc != 1)
	{
		std::cout << "Usage:\n\t ./dropboxServer\n";
		exit(1);
	}

    Socket* mainSocket = new Socket(SOCK_SERVER);
	mainSocket->login_server(std::string(), SERVER_PORT);
	say("server online");

	while(1){
		datagram = mainSocket->receiveDatagram();
		if (datagram.type != LOGIN)
			return 1;
		else
			say("login username: " + std::string(datagram.data));
		
		Socket* receiverSocket = new Socket(SOCK_SERVER);
		Socket* senderSocket = new Socket(SOCK_SERVER);

		int portReceiver = createNewPort();
		int portSender = createNewPort();
		std::string ports = std::to_string(portReceiver)+std::to_string(portSender);
		datagram.type = NEW_PORTS;
		strcpy(datagram.data, (char *) ports.c_str());
		mainSocket->sendDatagram(datagram);

		receiverSocket->login_server(std::string(), portReceiver);
		senderSocket->login_server(std::string(), portSender);
		say("waiting for a file");
		receiverSocket->receive_file();
		say("file received");

		// ServerComm* activeComm = server.newConnection();
		// activeComm->receiveMessage();
		// ServerComm* passiveComm = server.newConnection();
		// passiveComm->receiveMessage();

		// userName = passiveComm->receiveMessage();
		// std::cout << "[server]~: user " << userName << " logged in.\n";

		// ServerUser* thisUser;
		// bool found = false;
		// for (std::vector<ServerUser*>::iterator it = users.begin(); it != users.end(); ++it)
		// {
		// 	if((*it)->getName() == userName)
		// 	{
		// 		thisUser = (*it);
		// 		found = true;
		// 	}
		// }
		// if(!found){
		// 	FolderManager* thisFolder = new FolderManager(std::string(
		// 		database->getPath() + "sync_dir_" + userName
		// 	));
		// 	thisUser = new ServerUser(userName, thisFolder);
		// 	users.push_back(thisUser);
		// }
		// FolderManager* thisFolder = thisUser->getFolder();
		// Device* thisDevice = new Device(
		// 	ActiveProcess(activeComm, thisFolder),
		// 	PassiveProcess(passiveComm, thisFolder)
		// );
		// thisUser->newDevice(thisDevice);
		// std::thread init(initThread, thisUser, thisDevice);
		// init.detach();
	}

	return 0;
}
