#include "user.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

void User::login(std::string userid)
{
    this->userid = userid;
    this->logged_in = 1;
    this->createDir();
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
            socket->send_file(req.argument, std::string());
        }
        if (req.type == UPLOAD_SYNC_REQUEST){
            std::cout << "upload sync:" << req.argument << std::endl;
            socket->send_file(this->getFolderPath() + "/" + req.argument, req.argument2);
            this->updateSyncTime();
        }
        if (req.type == EXIT_REQUEST){
            socket->close_session();
        }
        
        this->requestsToSend.pop();
    }
}

void User::processRequest(Socket* socket)
{
    if (!this->requestsToReceive.empty())
    {
        Request req = this->requestsToReceive.front();
    
        if (req.type == DOWNLOAD_REQUEST){
            socket->get_file(req.argument, std::string());
        }
        if (req.type == DOWNLOAD_SYNC_REQUEST){
            File downloadedFile;
            downloadedFile.filename = req.argument;
            downloadedFile.last_modified = socket->get_file(req.argument, this->getFolderPath() + "/");
            this->addFile(downloadedFile);
            this->updateSyncTime();
        }
        if (req.type == EXIT_REQUEST){
            socket->close_session();
        }
        if (req.type == LIST_SERVER_REQUEST){
            std::list<File> filesList;
            
            filesList = socket->list_server();

            std::cout << "filename\tsize\tmodified\t\taccess\t\t\tcreation\n";
            for (std::list<File>::iterator f = filesList.begin(); f != filesList.end(); ++f) {
                std::cout << f->filename << "\t ";
                std::cout << f->size << "\t ";
                std::cout << f->last_modified << "\t ";
                std::cout << f->access_time << "\t ";
                std::cout << f->creation_time << "\t\n";
            }
        }

        this->requestsToReceive.pop();
    }
}

std::string User::getFolderName()
{
    return "sync_dir_" + this->userid;
}

std::string User::getFolderPath()
{
    return "client/sync_dir_" + this->userid;
}

std::list<File> User::getFilesFromFS()
{
    std::list<File> systemFiles;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(this->getFolderPath().c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (std::string(ent->d_name) != "." && std::string(ent->d_name) != "..") {
				File newFile;
				newFile.pathname = this->getFolderPath() + "/" + std::string(ent->d_name);
				newFile.filename = std::string(ent->d_name);
				newFile.last_modified = newFile.getTime('M');
				newFile.inode = newFile.getInode(this->getFolderPath() + "/");
				systemFiles.push_back(newFile);
			}
		}
		closedir (dir);
	}

    return systemFiles;
}

std::list<File> User::compareLocalLocal(std::list<File> systemFiles)
{
    std::list<File> uploadFiles;

    for (std::list<File>::iterator fsFile = systemFiles.begin(); fsFile != systemFiles.end(); ++fsFile)
    {
        bool found = false;
        for (std::list<File>::iterator userFile = this->files.begin(); userFile != this->files.end(); ++userFile)
        {
            if (fsFile->inode == userFile->inode)
            {
                found = true;
                if (fsFile->last_modified > userFile->last_modified && fsFile->last_modified > this->lastSync) {
                    uploadFiles.push_back(*fsFile);
                }
            }
        }
        if (!found) {
            uploadFiles.push_back(*fsFile);
        }
    }

    return uploadFiles;
}

std::list<File> User::compareLocalServer(Socket* receiverSocket)
{
    std::list<File> serverFiles, downloadFiles;
    serverFiles = receiverSocket->list_server();

    for (std::list<File>::iterator servFile = serverFiles.begin(); servFile != serverFiles.end(); ++servFile)
    {
        bool found = false;
        for (std::list<File>::iterator userFile = this->files.begin(); userFile != this->files.end(); ++userFile)
        {
            if (servFile->filename == userFile->filename)
            {
                found = true;
                if (servFile->last_modified > userFile->last_modified) {
                    downloadFiles.push_back(*servFile);
                }
            }
        }
        if (!found) {
            downloadFiles.push_back(*servFile);
        }
    }

    return downloadFiles;
}

void User::updateFiles(std::list<File> uploadFiles, std::list<File> downloadFiles)
{
    std::list<File> newFiles = uploadFiles;
    newFiles.splice(newFiles.end(), downloadFiles);

    for (std::list<File>::iterator newFile = newFiles.begin(); newFile != newFiles.end(); ++newFile)
    {
        bool found = false;
        for (std::list<File>::iterator userFile = this->files.begin(); userFile != this->files.end(); ++userFile)
        {
            if (newFile->filename == userFile->filename)
            {
                found = true;
                userFile->last_modified = newFile->last_modified;
            }
        }

        if (!found) {
            File fileToAdd;
            fileToAdd.filename = newFile->filename;
            fileToAdd.last_modified = newFile->last_modified;
            fileToAdd.inode = newFile->inode;
            this->files.push_back(fileToAdd);
        }
    }
}

void User::addFile(File newFile)
{
    bool found = false;
    for (std::list<File>::iterator userFile = this->files.begin(); userFile != this->files.end(); ++userFile)
    {
        if (newFile.filename == userFile->filename)
        {
            found = true;
            userFile->last_modified = newFile.last_modified;
            userFile->inode = newFile.getInode(this->getFolderPath() + "/");
        }
    }

    if (!found) {
        newFile.inode = newFile.getInode(this->getFolderPath() + "/");
        this->files.push_back(newFile);
    }
}

void User::save()
{
    std::fstream file;
    file.open("client/" + this->userid + ".txt", std::ios::out);
    file << "#user\n";
    file << this->userid << "\n";
    file << this->lastSync << "\n";
    if (!this->files.empty())
    {
        file << "#files\n";
        for (std::list<File>::iterator f = this->files.begin(); f != this->files.end(); ++f)
        {
            file << f->filename << "\n";
            file << f->last_modified << "\n";
            file << f->inode << "\n";
        }
    }
    file << "#end\n";
    file.close();
}

void User::load()
{
    std::fstream file;
    std::string line;
    std::list<File> list;
    this->files = std::list<File>();

    File db;
    db.pathname = "client/" + this->userid + ".txt";
    if (!db.exists())
        return;

    file.open(db.pathname, std::ios::in);
    while (std::getline(file, line))
    {
        if (line == "#user") 
        {
            // Get user id
            std::getline(file, line);
            if (line != this->userid)
                return;

            std::getline(file, line);
            this->lastSync = line;

            // Get pathnames
            std::getline(file, line);
            if (line == "#files")
            {
                while (line != "#end") {
                    std::getline(file, line);
                    if (line != "#end"){
                        File newFile;
                        newFile.filename = line;
                        
                        std::getline(file, line);
                        newFile.last_modified = line;

                        std::getline(file, line);
                        newFile.inode = std::stoi(line);

                        this->files.push_back(newFile);
                    }
                }
            }
        }

    }

    // std::cout << "list: " << list.size() << std::endl;
    file.close();
}

void User::updateSyncTime(){
    time_t     now = time(0);
    struct tm  tstruct;
    char       timeNow[80];
    tstruct = *localtime(&now);
    strftime(timeNow, sizeof(timeNow), "%Y/%m/%d %H:%M:%S", &tstruct);

    this->lastSync = timeNow;
}
