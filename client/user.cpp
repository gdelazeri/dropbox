#include "user.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void User::login(std::string userid)
{
    this->userid = userid;
    this->logged_in = 1;
    this->createDir();
}

void User::logout()
{
    this->logged_in = 0;
}

bool User::createDir()
{
    std::string dir = "client/sync_dir_" + this->userid;
    if((mkdir(dir.c_str(), 0777)) == 0)
        return true;
    return false;
}

void User::addRequestToSend(Request newRequest)
{
    this->requestsToSend.push(newRequest);
}

void User::addRequestToReceive(Request newRequest)
{
    this->requestsToReceive.push(newRequest);
}

void User::executeRequest(Socket* socket)
{
    if (!this->requestsToSend.empty())
    {
        Request req = this->requestsToSend.front();
    
        if (req.type == UPLOAD_REQUEST){
            socket->send_file(req.argument);
        }
        if (req.type == EXIT_REQUEST){
            socket->close_session();
        }
        
        this->requestsToSend.pop();
    }
}

void User::processResquest(Socket* socket)
{
    if (!this->requestsToReceive.empty())
    {
        Request req = this->requestsToReceive.front();
    
        if (req.type == DOWNLOAD_REQUEST){
            socket->get_file(req.argument);
        }
        if (req.type == EXIT_REQUEST){
            socket->close_session();
        }
        if (req.type == LIST_SERVER_REQUEST){
            std::list<File> filesList;
            
            filesList = socket->list_server();

            std::cout << "filename\tsize\tmodified\t\taccess\t\t\tcreation\n";

            for (std::list<File>::iterator f = filesList.begin(); f != filesList.end(); ++f) {
                std::cout << f->filename << "\t ";
                std::cout << f->size << "\t ";
                std::cout << f->last_modified << "\t ";
                std::cout << f->access_time << "\t ";
                std::cout << f->creation_time << "\t\n ";
            }
        }

        this->requestsToReceive.pop();
    }
}

std::string User::getFolderName()
{
    return "client/sync_dir_" + this->userid;
}

std::list<File> User::getFilesFromFS()
{
    std::list<File> systemFiles;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(this->getFolderName().c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (std::string(ent->d_name) != "." && std::string(ent->d_name) != "..") {
				File newFile;
				newFile.filename = ent->d_name;
				newFile.last_modified = newFile.getTime('M');
				newFile.inode = ent->d_ino;
				systemFiles.push_back(newFile);
			}
		}
		closedir (dir);
	}

    return systemFiles;
}

