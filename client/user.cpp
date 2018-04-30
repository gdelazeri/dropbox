#include "user.hpp"
#include <iostream>

void User::addRequestToSend(Request newRequest)
{
    this->requestsToSend.push(newRequest);
    std::cout << "addRequestToSend\n";
}

void User::addRequestToReceive(Request newRequest)
{
    this->requestsToReceive.push(newRequest);
}

void User::executeRequest(Socket* socket)
{
    if (!this->requestsToSend.empty())
    {
        std::cout << "executeRequest\n";
        Request req = this->requestsToSend.front();
        std::cout << req.type << "\n";
        std::cout << req.argument << "\n";
    
        if (req.type == 1){
            socket->send_file(req.argument);
        }
        this->requestsToSend.pop();
    }
}

void User::processResquest(Socket* socket)
{
    Request req = this->requestsToSend.front();
    this->requestsToReceive.pop();
}

void User::login()
{
    this->isConnected = true;
}

void User::logout()
{
    this->isConnected = false;
}