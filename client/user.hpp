#ifndef __USER_HPP__
#define __USER_HPP__

#include <string>
#include <queue>
#include <mutex>
#include "request.hpp"
#include "socket.hpp"
#include "file.hpp"


class User
{
	private:
        std::mutex sendAccess;
        std::mutex rcvAccess;
        std::mutex syncDir;

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

        // Files Handlers
        std::list<File> getFilesFromFS();
        std::list<File> filesToUpload(std::list<File> systemFiles);
        std::list<File> filesToDownload(Socket* receiverSocket);
        std::list<File> filesToDelete(std::list<File> systemFiles);
        void addFile(File newFile);
        void removeFile(std::string filename);
        void deleteFilesFromServer(std::list<std::string> deletedFiles);

        // Primitives
        void getSyncDir(Socket* receiverSocket);
        void listClient();

        void save();
        void load();
};
#endif