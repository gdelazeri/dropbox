#include "userServer.hpp"
#include <iostream>

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
