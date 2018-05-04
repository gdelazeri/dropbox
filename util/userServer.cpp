#include "userServer.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

void UserServer::login()
{
    this->logged_in = 1;
}

void UserServer::logout()
{
    this->logged_in = 0;
}

std::string UserServer::getFolderName()
{
    return "sync_dir_" + this->userid;
}

std::string UserServer::getFolderPath()
{
    return "server/sync_dir_" + this->userid;
}

bool UserServer::createDir(std::string side)
{
    std::string dir = side + "/" + this->getFolderName();
    if((mkdir(dir.c_str(), 0777)) == 0)
        return true;
    return false;
}
