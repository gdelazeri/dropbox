// #include "foldermanager.hpp"
// #include "clientcomm.hpp"
// #include "clientuser.hpp"
// #include "device.hpp"
// #include "file.hpp"

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

/* Utils */
void say(std::string message) 
{
	if (user->userid.empty())
		std::cout << CLIENT_NAME  << message << "\n";
	else
		std::cout << "[" << user->userid << "@dropbox] "  << message << "\n";
}

void syncThread(Socket* receiverSocket)
{
	while(user->logged_in)
	{
		std::list<File> localFiles, serverFiles, upload, download, deleteFiles;
		upload = std::list<File>();
		download = std::list<File>();

		std::list<File> systemFiles = user->getFilesFromFS();
		localFiles = user->filesToUpload(systemFiles);
		serverFiles = user->filesToDownload(receiverSocket);
		deleteFiles = user->filesToDelete(systemFiles);

		// Merge to upload
		for (auto upFile = localFiles.begin(); upFile != localFiles.end(); upFile++) {
			bool found = false;
			for (auto downFile = serverFiles.begin(); downFile != serverFiles.end(); downFile++) {
				if (upFile->filename == downFile->filename) {
					found = true;
					if (upFile->last_modified > downFile->last_modified)
						upload.push_back(*upFile);
				}
			}
			if (!found)
				upload.push_back(*upFile);
		}
			
		// Merge to download
		for (auto downFile = serverFiles.begin(); downFile != serverFiles.end(); downFile++) {
			bool found = false;
			for (auto upFile = localFiles.begin(); upFile != localFiles.end(); upFile++) {
				if (upFile->filename == downFile->filename) {
					found = true;
					if (upFile->last_modified <= downFile->last_modified)
						download.push_back(*downFile);
				}
			}
			if (!found)
				download.push_back(*downFile);
		}

		for (auto it = upload.begin(); it != upload.end(); ++it)
			user->addRequestToSend(Request(UPLOAD_SYNC_REQUEST, it->filename, it->last_modified));

		for (auto it = download.begin(); it != download.end(); ++it)
			user->addRequestToReceive(Request(DOWNLOAD_SYNC_REQUEST, it->filename));
		
		for (auto it = deleteFiles.begin(); it != deleteFiles.end(); ++it)
			user->addRequestToSend(Request(DELETE_REQUEST, it->filename));

		user->save();
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

void sendThread(Socket* socket)
{
	while(user->logged_in)
	{
		user->executeRequest(socket);
	}
	socket->finish();
	delete socket;
}

void receiveThread(Socket* socket)
{
	while(user->logged_in)
	{
		user->processRequest(socket);
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
		if (user->lockShell == 0)
		{
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
				user->lockShell = 1;
				user->addRequestToReceive(Request(LIST_SERVER_REQUEST, argument));
			}
			if(command == "list_client"){
				std::list<File> localFiles;
				localFiles = user->files;

				std::cout << "filename\tsize\t\tmodified\t\taccess\t\t\tcreation\n";
				for (std::list<File>::iterator f = localFiles.begin(); f != localFiles.end(); ++f) {
					std::cout << f->filename << "\t ";
					std::cout << f->size << "\t ";
					std::cout << f->last_modified << "\t ";
					std::cout << f->access_time << "\t ";
					std::cout << f->creation_time << "\t\n";
				}
			}
			if(command == "exit"){
				user->addRequestToSend(Request(EXIT_REQUEST, argument));
				user->addRequestToReceive(Request(EXIT_REQUEST, argument));
				user->logout();
			}
		}
	}
	say("FIM");
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
	std::thread shell(shellThread);

	receiver.detach();
	sender.detach();
	sync.detach();
	shell.join();

	user->save();
	mainSocket->finish();
	return 0;
}
