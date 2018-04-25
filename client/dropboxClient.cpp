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

#include "helper.hpp"
#include "socket.hpp"

// #include <csignal>
// #include <condition_variable>
// #include <mutex>

/* Utils */
void say(std::string message) {
	std::cout << CLIENT_NAME << message << "\n";
}

int main(int argc, char* argv[])
{
	// signal(SIGINT, signalHandler);
	if(argc != 4)
	{
		std::cout << "Usage:\n\t ./dropboxClient <user> <address> <port>\n";
		exit(1);
	}
	std::string userName = argv[1];
    say("User logged: " + userName);

	Socket* mainSocket = new Socket(SOCK_CLIENT);
	mainSocket->login_server(argv[2], atoi(argv[3]));
	mainSocket->sendMessage(userName);
	std::string ack = mainSocket->receiveMessage();
    say("ack: " + ack);

	return 0;

	Socket* senderSocket = new Socket(SOCK_CLIENT);
	Socket* receiverSocket = new Socket(SOCK_CLIENT);

	senderSocket->login_server(argv[2], atoi(argv[3]));
	receiverSocket->login_server(argv[2], atoi(argv[3]));

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
