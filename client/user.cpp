#include "user.hpp"
#include <iostream>

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

        this->requestsToReceive.pop();
    }
}

void User::login()
{
    this->logged_in = 1;
}

void User::logout()
{
    this->logged_in = 0;
}