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
#include "request.hpp"
#include "device.hpp"
#include <queue>
#include <mutex>

std::list<int> portsInUse;
std::list<UserServer*> users;
std::list<Device*> devices;
std::list<std::pair<std::string, std::string>> addresses;
std::list<std::pair<std::string, int>> serversAddresses;
std::queue<Request> replicationRequests;

std::mutex primaryMutex;
std::mutex replicationMutex;
std::mutex sendingReplicationMutex;

int primary = 0;
int commPort = 0;
bool electionTime = false;

std::string primaryIP;
int primaryPort = 0;

/* Utils */
void say(std::string message){
	std::cout << SERVER_NAME << message << "\n";
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
				socket->send_file(pathname, modificationTime, accessTime, creationTime, std::string());
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
				// for (std::list<std::pair<std::string, int>>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
				// 	if (it->first == device->address && it->second == device->port)
				// 		addresses.erase(it++);
				// }
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
		
				replicationMutex.lock();
				replicationRequests.push(Request(UPLOAD_REPLICATION, device->user->userid, pathname, modificationTime, accessTime, creationTime));
				replicationMutex.unlock();
		
				saveUsersServer(users);
				break;
			}

			case DELETE_TYPE: {
				std::string pathname = device->user->getFolderPath() + "/" + std::string(datagram.data);
				device->user->removeFile(pathname);
				device->user->deleted.push_back(std::make_pair(std::string(datagram.data), 2));
				saveUsersServer(users);
				// TODO: Adiciona na lista de arquivos a serem deletados o nome do file, id do user
				break;
			}

			case CLOSE:
				device->disconnect();
				// for (std::list<std::pair<std::string, int>>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
				// 	if (it->first == device->address && it->second == device->port)
				// 		addresses.erase(it++);
				// }
				break;
		}
	}

	socket->frontEnd->finish();
	portsInUse.remove(socket->port);
	delete socket;
	say("Logout: " + device->user->userid);
}

void liveSignalThread(){
	bool inElection = false;

	// Create socket for communication between servers
	Socket *serversComm = new Socket(SOCK_SERVER);
	serversComm->createSocket(commPort);

	// Send backup IP and PORT to primary server
	if (primary == 0) {
		tDatagram datagram;
		datagram.type = BACKUP;
		strcpy(datagram.data, (getIP() + "#" + std::to_string(commPort)).c_str());
		serversComm->frontEnd->sendDatagramToAddress(datagram, primaryIP, primaryPort);
	}

	// Wait for live signal from primay server
	while (primary == 0) {	
		tDatagram datagram;
		datagram = serversComm->frontEnd->receiveDatagramWithTimeout(4);
		
		if (datagram.type == ERROR) {
			// Check if the election is occuring
			if (!electionTime) {
				electionTime = true;
				inElection = true;
				std::cout << "Whoops! Primary server is down! Starting election..\n";

				datagram.type = ELECTION_TIME;
				std::string commPortTemp = std::to_string(commPort);
				strcpy(datagram.data, (char *) commPortTemp.c_str());
				
				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
			} else if (inElection) {
				primaryMutex.lock();
				primary = 1;
				primaryMutex.unlock();
				std::cout << "I'm the new primary server!\n";
				
				// Send the server data (new primary) to all other backup servers
				datagram.type = NEW_PRIMARY;
				strcpy(datagram.data, (getIP() + "#" + std::to_string(commPort)).c_str());

				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
				electionTime = false;
			}	
		} else if (datagram.type == ELECTION_TIME) {	
			electionTime = true;
			inElection = true;
			int	serverPort = atoi(datagram.data);
			if (serverPort < commPort) {
				datagram.type = ELECTION_TIME;
				std::string commPortTemp = std::to_string(commPort);
				strcpy(datagram.data, (char *) commPortTemp.c_str());

				for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it)
					serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
			} else {
				inElection = false;
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
		} else if (datagram.type == BEGIN_FILE_TYPE_REPLICATION) {
			serversComm->frontEnd->sendAck();
			std::string pathname = std::string(datagram.data);
			std::string times = serversComm->receive_file(pathname);
			std::string modificationTime = getByHashString(times, 0, "#");
			std::string accessTime = getByHashString(times, 1, "#");
			std::string creationTime = getByHashString(times, 2, "#");
			std::string userid = getByHashString(getByHashString(pathname, 2, "_"), 0, "/");
    		for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it) {
				if ((*it)->userid == userid)
					(*it)->addFile(pathname, modificationTime, accessTime, creationTime);
			}
		} else if (datagram.type == LOGIN) {
			serversComm->frontEnd->sendAck();
			std::string datagramData = std::string(datagram.data);
			UserServer* user = new UserServer();
			user = searchUser(getByHashString(datagramData, 0, "#"));
			say("Login: " + user->userid);
			saveUsersServer(users);
			Device* newDevice = new Device(user, getByHashString(datagramData, 1, "#"), atoi(getByHashString(datagramData, 2, "#").c_str()));
			newDevice->connect();
			devices.push_back(newDevice);
			// addresses.push_back(std::make_pair(user->userid, newDevice->address + "#" + newDevice->port));
		} else {
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
		// Wait for new backup address
		tDatagram datagram;
		datagram = serversComm->frontEnd->receiveDatagramWithTimeout(2);
		if (datagram.type == BACKUP) {
			std::string backupHost = getByHashString(std::string(datagram.data), 0, "#");
			int backupPort = atoi(getByHashString(std::string(datagram.data), 1, "#").c_str());
			std::cout << "Backup Address: " << backupHost << ", " << backupPort << std::endl;
			bool found = false;
			for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
				if (it->first == backupHost && it->second == backupPort)
					found = true;
			}
			if (!found)
				serversAddresses.push_back(std::make_pair(backupHost, backupPort));
		}

		// Send live signal to all backup servers
		for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
			tDatagram datagram;
			datagram.type = LIVE_SIGNAL;
			std::string addressesStr;
			for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
				addressesStr = addressesStr + it->first + "*" + std::to_string(it->second) + "#";
			}
			strcpy(datagram.data, addressesStr.c_str());

			sendingReplicationMutex.lock();
			serversComm->frontEnd->sendDatagramToAddress(datagram, it->first, it->second);
			sendingReplicationMutex.unlock();
		}
	}
}

void replicationThread() {
	Socket *replicationSocket = new Socket(SOCK_CLIENT);
	
	while(1) {
		replicationMutex.lock();
		if (!replicationRequests.empty())
		{
			Request req = replicationRequests.front();
			switch (req.type) {
				case UPLOAD_REPLICATION:
					for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
						replicationSocket->login_server(it->first, it->second);
						sendingReplicationMutex.lock();
						replicationSocket->send_file(req.argument2, req.argument3, req.argument4, req.argument5, req.argument);
						sendingReplicationMutex.unlock();
					}
					break;

				case LOGIN_REPLICATION:
					for (std::list<std::pair<std::string, int>>::iterator it = serversAddresses.begin(); it != serversAddresses.end(); ++it) {
						replicationSocket->login_server(it->first, it->second);
						tDatagram datagram;
						datagram.type = LOGIN;
						strcpy(datagram.data, (req.argument + "#" + req.argument2 + "#" + req.argument3).c_str());
						sendingReplicationMutex.lock();
						replicationSocket->frontEnd->sendDatagram(datagram);
						sendingReplicationMutex.unlock();
					}
					break;
			}

        	replicationRequests.pop();
		}
		replicationMutex.unlock();
	}
	delete replicationSocket;
}

void notifyClients(){
	Socket *frontEndSocket = new Socket(SOCK_SERVER);
	for (std::list<Device*>::iterator it = devices.begin(); it != devices.end(); ++it) {
		Socket* receiverSocket = new Socket(SOCK_SERVER);
		Socket* senderSocket = new Socket(SOCK_SERVER);

		int portReceiver = createNewPort(portsInUse);
		int portSender = createNewPort(portsInUse);
		receiverSocket->createSocket(portReceiver);
		senderSocket->createSocket(portSender);
		// Device* newDevice = new Device((*it)->user, (*it)->address, (*it)->port);

		std::thread rcv(receiveThread, receiverSocket, (*it));
		std::thread snd(sendThread, senderSocket, (*it));
		rcv.detach();	
		snd.detach();

		tDatagram datagram;
		datagram.type = NEW_PRIMARY;
		strcpy(datagram.data, (getIP() + "#" + std::to_string(portReceiver) + "#" + std::to_string(portSender)).c_str());
		frontEndSocket->frontEnd->sendDatagramToAddress(datagram, (*it)->address, (*it)->port);
	}
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

	while (1){
		primaryMutex.lock();
		if (primary == 1){
			primaryMutex.unlock();
			break;
		}
		primaryMutex.unlock();
	}

    Socket *mainSocket = new Socket(SOCK_SERVER);
	mainSocket->createSocket(SERVER_PORT);
	say("Server Online");

	std::thread replication(replicationThread);
	replication.detach();
	
	notifyClients();
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
				devices.push_back(newDevice);
				// addresses.push_back(std::make_pair(newDevice->user->userid, newDevice->address + "#" + newDevice->port));
				replicationRequests.push(Request(LOGIN_REPLICATION, user->userid, newDevice->address, newDevice->port));

				Socket* receiverSocket = new Socket(SOCK_SERVER);
				Socket* senderSocket = new Socket(SOCK_SERVER);

				int portReceiver = createNewPort(portsInUse);
				int portSender = createNewPort(portsInUse);
				receiverSocket->createSocket(portReceiver);
				senderSocket->createSocket(portSender);

				std::string ports = std::to_string(portReceiver)+std::to_string(portSender);
				datagram.type = NEW_PORTS;
				strcpy(datagram.data, (char *) ports.c_str());
				mainSocket->frontEnd->sendDatagram(datagram);

				std::thread rcv(receiveThread, receiverSocket, newDevice);
				std::thread snd(sendThread, senderSocket, newDevice);
				
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
