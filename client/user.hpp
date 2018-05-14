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
        int lockShell;
        int lockRequests;
        std::string lastSync;
        std::queue<Request> requestsToSend;
        std::queue<Request> requestsToReceive;
        std::list<File> files;

        void addRequestToSend(Request newRequest);
        void addRequestToReceive(Request newRequest);
        void executeRequest(Socket* socket);
        void processRequest(Socket* socket);
        void login(std::string userid);
        void logout();
        bool createDir();
        std::string getFolderName();
        std::string getFolderPath();
        void updateSyncTime();

        // Files
        std::list<File> getFilesFromFS();
        std::list<File> compareLocalLocal(std::list<File> systemFiles);
        std::list<File> compareLocalServer(Socket* receiverSocket);
        void updateFiles(std::list<File> uploadFiles, std::list<File> downloadFiles);
        void addFile(File newFile);

        void save();
        void load();
};
#endif