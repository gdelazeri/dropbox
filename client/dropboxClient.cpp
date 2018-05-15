#include <sys/types.h>
#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>
#include <map>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "user.hpp"

User* user = new User();

void say(std::string message) 
{
	if (user->userid.empty())
		std::cout << CLIENT_NAME  << message << "\n";
	else
		std::cout << "[" << user->userid << "@dropbox] "  << message << "\n";
}

/* Thread to synchronize files between Client and Server  */
void syncThread(Socket* receiverSocket)
{
	while(user->logged_in)
	{
		user->getSyncDir(receiverSocket);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

/* Thread to call method to execute requests and send files */
void sendThread(Socket* senderSocket)
{
	while(user->logged_in)
	{
		user->executeRequest(senderSocket);
	}
	senderSocket->finish();
	delete senderSocket;
}

/* Thread to call method to process requests and receive files */
void receiveThread(Socket* receiverSocket)
{
	while(user->logged_in)
	{
		user->processRequest(receiverSocket);
	}
	receiverSocket->finish();
	delete receiverSocket;
}

/* Thread that interact with user and process commands */
void shellThread(Socket* receiverSocket)
{
	say("Type your command:");
	while(user->logged_in)
	{
		if (user->lockShell == 0)
		{
			std::string line;
			std::string command;
			std::string arg;
			std::size_t pos;

			std::cout << ">> ";
			std::getline(std::cin, line);

			if((pos = line.find(" ")) != std::string::npos) {
				command = line.substr(0, pos);
				line = line.substr(pos+1, std::string::npos);
				if(line != "")
					arg = line;
				else
					arg = "";
				line = "";
			} else {
				command = line;
				line = "";
			}

			if(command == "upload"){
				user->addRequestToSend(Request(UPLOAD_REQUEST, arg));
			}
			else if(command == "download"){
				user->addRequestToReceive(Request(DOWNLOAD_REQUEST, arg));
			}
			else if(command == "list_server"){
				user->lockShell = 1;
				user->addRequestToReceive(Request(LIST_SERVER_REQUEST, arg));
			}
			else if(command == "list_client"){
				user->listClient();
			}
			else if(command == "get_sync_dir"){
				user->getSyncDir(receiverSocket);
			}
			else if(command == "exit"){
				user->addRequestToSend(Request(EXIT_REQUEST, arg));
				user->addRequestToReceive(Request(EXIT_REQUEST, arg));
				user->logout();
			}
		}
	}
	say("FIM");
}

int main(int argc, char* argv[])
{

	tDatagram datagram;

	if(argc != 4)
	{
		std::cout << "Usage:\n\t ./dropboxClient <user> <address> <port>\n";
		exit(1);
	}
	user->login(argv[1]);
    say("User: " + user->userid);
	user->load();

	// Create main communication
	Socket* mainSocket = new Socket(SOCK_CLIENT);
	mainSocket->login_server(argv[2], atoi(argv[3]));

	// Send user
	datagram.type = LOGIN;
	strcpy(datagram.data, user->userid.c_str());
	mainSocket->sendDatagram(datagram);

	// Receive ports
	datagram = mainSocket->receiveDatagram();
	if (datagram.type != NEW_PORTS) {
		std::cout << datagram.data << std::endl;
		exit(0);
	}
	std::pair<int, int> ports = getPorts(datagram.data);

	// Create sender and receiver communication
	Socket* senderSocket = new Socket(SOCK_CLIENT);
	Socket* receiverSocket = new Socket(SOCK_CLIENT);
	senderSocket->login_server(argv[2], ports.first);
	receiverSocket->login_server(argv[2], ports.second);
	
	// Create threads
	std::thread receiver(receiveThread, receiverSocket);
	std::thread sender(sendThread, senderSocket);
	std::thread sync(syncThread, receiverSocket);
	std::thread shell(shellThread, receiverSocket);

	receiver.detach();
	sender.detach();
	sync.detach();
	shell.join();

	user->save();
	mainSocket->finish();

	return 0;
}
