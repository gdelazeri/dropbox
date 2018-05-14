#include "userServer.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

std::string UserServer::getFolderName()
{
    return "sync_dir_" + this->userid;
}

std::string UserServer::getFolderPath()
{
    return "server/sync_dir_" + this->userid;
}

bool UserServer::createDir()
{
    std::string dir = this->getFolderPath();
    if((mkdir(dir.c_str(), 0777)) == 0)
        return true;
    return false;
}


// Files
void UserServer::addFile(std::string pathname, std::string modificationTime)
{
    for (std::list<File>::iterator it = this->files.begin(); it != this->files.end(); ++it){
    	if (it->pathname == pathname) {
			it->last_modified = modificationTime;
            return;
		}
	}

	File newFile;
	newFile.pathname = pathname;
	newFile.last_modified = modificationTime;
	this->files.push_back(newFile);
    return;
}

std::string UserServer::getFileModificationTime(std::string pathname)
{
    for (std::list<File>::iterator it = this->files.begin(); it != this->files.end(); ++it){
    	if (it->pathname == pathname) {
            return it->last_modified;
		}
	}

    return std::string();
}
