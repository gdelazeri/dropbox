// #include "foldermanager.hpp"
// #include "clientcomm.hpp"
// #include "clientuser.hpp"
// #include "device.hpp"
// #include "file.hpp"

#include <iostream>
#include <iterator>
#include <thread>
#include <chrono>
#include <map>
#include <fstream>
#include <string.h>
#include <stdio.h>

#include "socket.hpp"

// #include <csignal>
// #include <condition_variable>
// #include <mutex>

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
	
	char buf[50];
	std::fstream fbin;
	fbin.open("client/test.txt", std::ios::binary | std::ios::in);
	std::streamsize length = fbin.gcount();
	fbin.read(buf, 50);
	std::cout << buf  << '\n';
	
    // std::ifstream file ("test.txt", std::ios::binary);
	// file.seekg(0);
    // file.read(buf, 0);
	// std::cout << buf << '\n';

	// sendDatagram->sendDatagram();


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
