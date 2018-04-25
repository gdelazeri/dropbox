// #include "database.hpp"
// #include "foldermanager.hpp"
// #include "servercomm.hpp"
// #include "serveruser.hpp"
// #include "device.hpp"

#include "socket.hpp"
#include "helper.hpp"

#include <iostream>
#include <iterator>
#include <vector>
#include <thread>

int main(int argc, char* argv[])
{

	if(argc != 1)
	{
		std::cout << "Usage:\n\t ./dropboxServer\n";
		exit(1);
	}

	// ServerComm server(atoi(argv[1]));
	// if(!database)
	// {
	// 	if(argc == 3)
	// 		database = new Database(std::string(argv[2]));
	// 	else
	// 		database = new Database("./database");
	// }
    Socket* mainSocket = new Socket(SOCK_SERVER);
	mainSocket->login_server(std::string(), 4000);
	std::cout << "server@dropbox~: server online " << "\n\n";

	// std::cout << "[server]~: dropbox server is up at port " << atoi(argv[1]) <<", database: " << database->getPath() << "\n";
	// while(1){
		std::string username = mainSocket->receiveMessage();
		std::cout << username << "\n";
		mainSocket->sendMessage("KK");
		getchar();
		
		// Socket* recSocket = new Socket(SOCK_SERVER);
		// recSocket->login_server(std::string(), 4000);

        // std::cout << "firstMessage" << firstMessage;
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
	// }

	return 0;
}
