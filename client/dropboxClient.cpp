// #include "foldermanager.hpp"
// #include "clientcomm.hpp"
// #include "clientuser.hpp"
// #include "device.hpp"
// #include "file.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>
#include <map>
#include <string.h>
#include <stdio.h>
#include <mutex>

#include "user.hpp"

// #include <csignal>
// #include <condition_variable>
// #include <mutex>

User* user = new User();

/* Utils */
void say(std::string message) 
{
	if (user->userid.empty())
		std::cout << CLIENT_NAME  << message << "\n";
	else
		std::cout << "[" << user->userid << "@dropbox] "  << message << "\n";
}

void sendThread(Socket* socket)
{
	while(user->logged_in)
	{
		// std::cout << "sendThread\n";
		user->executeRequest(socket);
	}
	socket->finish();
	delete socket;
}

void receiveThread(Socket* socket)
{
	while(user->logged_in)
	{
		// std::cout << "receiveThread\n";
		user->processResquest(socket);
	}
	socket->finish();
	delete socket;
}

void shellThread()
{
	std::mutex block;
	say("Type your command:");

	while(user->logged_in)
	{
		while(user->lock) { }

		std::string line;
		std::string command;
		std::string argument;
		std::size_t pos;

		std::cout << ">> ";
		std::getline(std::cin, line);

		if((pos = line.find(" ")) != std::string::npos)
		{
			command = line.substr(0, pos);
			line = line.substr(pos+1, std::string::npos);
			if(line != "")
				argument = line;
			else
				argument = "";
			line = "";
		}
		else
		{
			command = line;
			line = "";
		}

		if(command == "upload"){
			user->addRequestToSend(Request(UPLOAD_REQUEST, argument));
		}
		if(command == "download"){
			user->addRequestToReceive(Request(DOWNLOAD_REQUEST, argument));
		}
		if(command == "list_server"){
			user->addRequestToReceive(Request(LIST_SERVER_REQUEST, argument));
			user->lockShell();
            // std::cout << "user->lockShell();\n";
		}
		if(command == "exit"){
			user->addRequestToSend(Request(EXIT_REQUEST, argument));
			user->addRequestToReceive(Request(EXIT_REQUEST, argument));
			user->logout();
		}
	}
}

int main(int argc, char* argv[])
{
	tDatagram datagram;

	// signal(SIGINT, signalHandler);
	if(argc != 4)
	{
		std::cout << "Usage:\n\t ./dropboxClient <user> <address> <port>\n";
		exit(1);
	}
	user->login(argv[1]);
    say("User: " + user->userid);

	// Create main communication
	Socket* mainSocket = new Socket(SOCK_CLIENT);
	mainSocket->login_server(argv[2], atoi(argv[3]));

	// Send user
	datagram.type = LOGIN;
	strcpy(datagram.data, user->userid.c_str());
	mainSocket->sendDatagram(datagram);

	// Receive ports
	datagram = mainSocket->receiveDatagram();
	if (datagram.type != NEW_PORTS)
		return 1;
	std::pair<int, int> ports = getPorts(datagram.data);

	// Create sender and receiver communication
	Socket* senderSocket = new Socket(SOCK_CLIENT);
	Socket* receiverSocket = new Socket(SOCK_CLIENT);
	senderSocket->login_server(argv[2], ports.first);
	receiverSocket->login_server(argv[2], ports.second);
	
	// Create threads
	std::thread receiver(receiveThread, receiverSocket);
	std::thread sender(sendThread, senderSocket);
	std::thread shell(shellThread);

	receiver.detach();
	sender.detach();
	shell.join();
	
	// std::thread noti(notifyThread, thisDevice, thisFolder);

	mainSocket->finish();

	return 0;
    
	// passiveComm->connectServer(std::string(argv[2]), atoi(argv[3]));
	// passiveComm->sendMessage("PASSIVE");
	// activeComm->connectServer(std::string(argv[2]), atoi(argv[3]));
	// activeComm->sendMessage("ACTIVE");

	// activeComm->sendMessage(userName);

	// FolderManager* thisFolder;
	// if(argc == 5){
	// 	thisFolder = new FolderManager(std::string(argv[4]));
	// }
	// else {
	// 	thisFolder = NULL;
	// }

	// Device* thisDevice = new Device(
	// 	ActiveProcess(activeComm, thisFolder),
	// 	PassiveProcess(passiveComm, thisFolder)
	// );

	// user = new ClientUser(userName, thisFolder, thisDevice);
	// if(argc == 5) {
	// 	user->synchronize();
	// 	thisDevice->pushAction(Action(ACTION_INITILIAZE));
	// }
	// std::thread io(ioThread, thisDevice);
	// std::thread act(activeThread, thisDevice);
	// std::thread pass(passiveThread, thisDevice);
	// std::thread noti(notifyThread, thisDevice, thisFolder);

	// io.join();

	return 0;
}
