#ifndef __USER_HPP__
#define __USER_HPP__

#include <string>
#include <queue>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "request.hpp"
#include "socket.hpp"
#include "file.hpp"

class User
{
	public:
        std::string userid;
        int logged_in;
        std::queue<Request> requestsToSend;
        std::queue<Request> requestsToReceive;
        // pthread_mutex_t mutex;
        int lock;

        void addRequestToSend(Request newRequest);
        void addRequestToReceive(Request newRequest);
        void executeRequest(Socket* socket);
        void processResquest(Socket* socket);
        void login(std::string userid);
        void logout();
        bool createDir();

        void lockShell();
        void unlockShell();
};
#endif