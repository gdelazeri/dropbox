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
#include <string>
#include "socket.hpp"
#include "device.hpp"

std::list<int> portsInUse;
std::list<UserServer*> users;
std::list<std::pair<std::string, int>> addresses;
std::list<std::pair<std::string, int>> serversAddresses;

int primary = 0;
int commPort = 0;
bool electionTime = false;

std::string primaryIP;
int primaryPort = 0;

/* Utils */
void say(std::string message){
	std::cout << SERVER_NAME << message << "\n";
}
				
void sendThread(Socket* socket, Device* device)
{
	tDatagram datagram;
	while(device->connected)
	{
		datagram = socket->frontEnd->receiveDatagram();
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
				for (std::list<std::pair<std::string, int>>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
					if (it->first == device->address && it->second == device->port)
						addresses.erase(it++);
				}
				break;
		}
	}

	socket->frontEnd->finish();
	portsInUse.remove(socket->port);
	delete socket;
}

void receiveThread(Socket* socket, Device* device)
{
	tDatagram datagram;
	
	while(device->connected)
	{
		datagram = socket->frontEnd->receiveDatagram();
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
				for (std::list<std::pair<std::string, int>>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
					if (it->first == device->address && it->second == device->port)
						addresses.erase(it++);
				}
				break;
		}
	}

	socket->frontEnd->finish();
	portsInUse.remove(socket->port);
	delete socket;
	say("Logout: " + device->user->userid);
}

void liveSignalThread(){
	printf("LiveSignalThread, press any button to continue...\n");
	getchar();
	// Create socket for communication between servers
	Socket *serversComm = new Socket(SOCK_SERVER);
	serversComm->createSocket(commPort);

	// Send backup IP and PORT to primary server
	if (primary == 0) {
		printf("B: Sending my IP and PORT to primary\n");
		tDatagram datagram;
		datagram.type = BACKUP;
		strcpy(datagram.data, (getIP() + "#" + std::to_string(commPort)).c_str());
		serversComm->frontEnd->sendDatagramToAddress(datagram, primaryIP, primaryPort);
	}

	// Wait for live signal from primay server
	while (primary == 0) {	
		// printf("B: Waiting for primary live signal\n");
		tDatagram datagram;
		datagram = serversComm->frontEnd->receiveDatagramWithTimeout(3);
		
		if (datagram.type == ERROR) {
			// Check if the election is occuring
			if (!electionTime) {
				electionTime = true;
				std::cout << "Whoops! Primary server is down! Starting election..\n";

				datagram.type = ELECTION_TIME;
				std::string commPortTemp = std::to_string(commPort);
				strcpy(datagram.data, (char *) commPortTemp.c_str());
				
				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
			} else {
				primary = 1;
				std::cout << "I'm the new primary server!\n";
				
				// Send the server data (new primary) to all other backup servers
				datagram.type = NEW_PRIMARY;
				strcpy(datagram.data, (getIP() + "#" + std::to_string(commPort)).c_str());

				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
				electionTime = false;
			}	
		} else if (datagram.type == ELECTION_TIME) {	
			int	serverPort = atoi(datagram.data);
			
			if (serverPort < commPort) {
				datagram.type = ELECTION_TIME;
				std::string commPortTemp = std::to_string(commPort);
				strcpy(datagram.data, (char *) commPortTemp.c_str());

				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
			}
		} else if (datagram.type == NEW_PRIMARY) {
			// Change primary server IP and Port for the new primary server data
			primaryIP = getByHashString(std::string(datagram.data), 0, "#");
			primaryPort = atoi(getByHashString(std::string(datagram.data), 1, "#").c_str());

			std::cout << "We have a new primary server, opperating in address " << primaryIP << " and port " << primaryPort << ".\n";

			for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
				if (it->first == primaryIP && it->second == primaryPort)
					serversAddresses.erase(it++);
			}				

			electionTime = false;
		} else {
			// printf("B: And primary is alive!\n");
			std::string datagramData = std::string(datagram.data);
			int numberOfServers = std::count(datagramData.begin(), datagramData.end(), '#');
			serversAddresses.clear();
			for (int i = 0; i < numberOfServers; i++) {
				std::string backupAdd = getByHashString(datagramData, i, "#");
				std::string backupServerHost = getByHashString(backupAdd, 0, "*");
				int backupServerPort = atoi(getByHashString(backupAdd, 1, "*").c_str());
				if (backupServerPort != commPort)
					serversAddresses.push_back(std::make_pair(backupServerHost, backupServerPort));
			}
		}
	}

	while (primary == 1) {
		// printf("P: Waiting for new backup address\n");
		// Wait for new backup address
		tDatagram datagram;
		datagram = serversComm->frontEnd->receiveDatagramWithTimeout(2);
		if (datagram.type == BACKUP) {
			std::string backupHost = getByHashString(std::string(datagram.data), 0, "#");
			int backupPort = atoi(getByHashString(std::string(datagram.data), 1, "#").c_str());
			std::cout << "Backup Address: " << backupHost << ", " << backupPort << std::endl;
			bool found = false;
			for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
				// std::cout << it->second << std::endl;
				if (it->first == backupHost && it->second == backupPort)
					found = true;
			}
			if (!found)
				serversAddresses.push_back(std::make_pair(backupHost, backupPort));
		}

		// printf("P: Sending live signal to all backup servers\n");
		// Send live signal to all backup servers
		for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
			tDatagram datagram;
			datagram.type = LIVE_SIGNAL;
			std::string addresses;
			for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
				addresses = addresses + it->first + "*" + std::to_string(it->second) + "#";
			}
			strcpy(datagram.data, addresses.c_str());
			serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
		}
	}

}

void notifyClientsThread(){
	//while(!primary);

	// Socket *frontEndSocket = new Socket(SOCK_CLIENT);
	// for (std::list<std::pair<std::string, int>>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
	// 	frontEndSocket->login_server(it->first, it->second);
	// 	tDatagram datagram;

	// 	frontEndSocket->frontEnd->finish();
	// }
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

	if(argc < 3)
	{
		std::cout << "Usage:\n\t ./dropboxServer port type(--primary/--backup) <primaryIP> <portIP>\n";
		exit(1);
	}

	commPort = atoi(argv[1]);
	if (strcmp(argv[2], "--primary") == 0) {
		primary = 1;	
	} else if (argc == 5){
		primary = 0;
		primaryIP = argv[3];
		primaryPort = atoi(argv[4]);
	} else {
		std::cout << "Usage:\n\t ./dropboxServer port type(--primary/--backup) <primaryIP> <portIP>\n";
		exit(1);
	}

	std::thread servers(liveSignalThread);
	servers.detach();

	while (primary == 0);

    Socket *mainSocket = new Socket(SOCK_SERVER);
	mainSocket->createSocket(SERVER_PORT);
	say("Server Online");

	while(true)
	{
		UserServer* user = new UserServer();
		datagram = mainSocket->frontEnd->receiveDatagram();

		if (datagram.type == LOGIN)
		{
			std::string loginInfo = std::string(datagram.data);
			user = searchUser(getByHashString(loginInfo, 0, "#"));
			say("Login: " + user->userid);

			saveUsersServer(users);

			Device* newDevice = new Device(user, getByHashString(loginInfo, 1, "#"), atoi(getByHashString(loginInfo, 2, "#").c_str()));

			if (newDevice->connect()) {
				addresses.push_back(std::make_pair(newDevice->address, newDevice->port));

				Socket* receiverSocket = new Socket(SOCK_SERVER);
				Socket* senderSocket = new Socket(SOCK_SERVER);

				int portReceiver = createNewPort(portsInUse);
				int portSender = createNewPort(portsInUse);
				receiverSocket->createSocket(portReceiver);
				senderSocket->createSocket(portSender);
				std::cout << "Ports: " << portReceiver << portSender << std::endl;

				std::string ports = std::to_string(portReceiver)+std::to_string(portSender);
				datagram.type = NEW_PORTS;
				strcpy(datagram.data, (char *) ports.c_str());
				mainSocket->frontEnd->sendDatagram(datagram);

				std::thread rcv(receiveThread, receiverSocket, newDevice);
				std::thread snd(sendThread, senderSocket, newDevice);

				// getchar();
				// Socket *socketNotify = new Socket(SOCK_CLIENT);
				// socketNotify->login_server(user->address, user->port);
				// tDatagram datagram;
				// strcpy(datagram.data, "aopsdkopsakdopsakdopsa");
				// socketNotify->frontEnd->sendDatagram(datagram);
				// getchar();
				
				rcv.detach();	
				snd.detach();
			}
			else {
				datagram.type = ERROR;
				strcpy(datagram.data, "Too many devices for this user. Try again later.");
				mainSocket->frontEnd->sendDatagram(datagram);
			}
		}
	}

	return 0;
}
