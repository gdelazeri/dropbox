#include "userServer.hpp"
#include <iostream>

void login()
{
    this->logged_in = 1;
}

void logout()
{
    this->logged_in = 0;
}

std::string getFolderName()
{
    return "sync_dir_" + this->userid;
}