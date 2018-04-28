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
void say(std::string message) {
	std::cout << CLIENT_NAME << message << "\n";
}

/* Parse data ports */
std::pair<int, int> getPorts(char* data) {
	std::string port = std::string(data);
	int p1 = std::stoi(port.substr(0,4));
	int p2 = std::stoi(port.substr(4,8));

	return std::make_pair(p1, p2);
}

void sendThread(Socket* socket)
{
	while(user->isConnected)
	{
		// std::cout << "sendThread\n";
		user->executeRequest(socket);
	}
}

void receiveThread(Socket* socket)
{
	while(user->isConnected)
	{
		// std::cout << "receiveThread\n";
		// user->processResquest(socket);
	}
}

void shellThread()
{
	std::mutex block;

	while(user->isConnected)
	{
		say("shellThread");
		std::string line;
		std::string command;
		std::string argument;
		std::size_t pos;

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
			std::cout << command << '\n';
			user->addRequestToSend(Request(UPLOAD_REQUEST, argument));
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
	std::string userName = argv[1];
    say("User: " + userName);
	user->isConnected = true;

	// Cria comunicação principal com o servidor
	Socket* mainSocket = new Socket(SOCK_CLIENT);
	mainSocket->login_server(argv[2], atoi(argv[3]));
	mainSocket->sendMessage(userName);

	// Recebe portas
	datagram = mainSocket->receiveDatagram();
	std::pair<int, int> ports = getPorts(datagram.data);
	if (datagram.type != NEW_PORTS)
		return 1;

	Socket* senderSocket = new Socket(SOCK_CLIENT);
	Socket* receiverSocket = new Socket(SOCK_CLIENT);
	senderSocket->login_server(argv[2], ports.first);
	receiverSocket->login_server(argv[2], ports.second);
	say("Sockets created");
	
	std::thread shell(shellThread);
	std::thread sender(sendThread, senderSocket);
	std::thread receiver(receiveThread, receiverSocket);
	say("Threads created");

	shell.join();
	
	// std::thread noti(notifyThread, thisDevice, thisFolder);

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
