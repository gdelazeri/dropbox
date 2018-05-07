#include "user.hpp"


void User::login(std::string userid)
{
    this->userid = userid;
    this->logged_in = 1;
    this->createDir();
    // pthread_mutex_init(&this->mutex, NULL);
    this->lock = 0;
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
            socket->list_server();
			this->unlockShell();
            // std::cout << "this->unlockShell();\n";
        }

        this->requestsToReceive.pop();
    }
}




void User::lockShell()
{
    // pthread_mutex_lock(&this->mutex);
    this->lock = 1;
}

void User::unlockShell()
{
    // pthread_mutex_unlock(&this->mutex);
    this->lock = 0;
}