#ifndef __USER_HPP__
#define __USER_HPP__

#include <string>
#include <queue>
#include <mutex>
#include "request.hpp"
#include "socket.hpp"
#include "file.hpp"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

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
        std::list<File> filesToUpload(std::list<File> systemFiles);
        std::list<File> filesToDownload(Socket* receiverSocket);
        std::list<File> filesToDelete(std::list<File> systemFiles);
        void updateFiles(std::list<File> uploadFiles, std::list<File> downloadFiles);
        void addFile(File newFile);
        void removeFile(std::string filename);

        void save();
        void load();
};
#endif