#ifndef __USER_HPP__
#define __USER_HPP__

#include <string>
#include <queue>
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
        std::list<File*> files;

        void addRequestToSend(Request newRequest);
        void addRequestToReceive(Request newRequest);
        void executeRequest(Socket* socket);
        void processResquest(Socket* socket);
        void login(std::string userid);
        void logout();
        bool createDir();
        std::string getFolderName();

        std::list<File> getFilesFromFS();
        
};
#endif