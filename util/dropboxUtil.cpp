#include "dropboxUtil.hpp"
#include "file.hpp"

/* Save users server in database.txt */
void saveUsersServer(std::list<UserServer*> users)
{
    std::fstream file;
    file.open("server/database.txt", std::ios::out);
    for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        file << "#user\n";
        file << (*it)->userid << "\n";
        if (!(*it)->files.empty())
        {
            file << "#files\n";
            for (std::list<File>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                file << f->pathname << "\n";
                file << f->last_modified << "\n";
                file << f->access_time << "\n";
                file << f->creation_time << "\n";
            }
        }
        file << "#end\n";
	}
    file.close();
}

/* Load users server from database.txt */
std::list<UserServer*> loadUsersServer()
{
    std::list<UserServer*> usersDB;
    std::fstream file;
    std::string line; 

    file.open("server/database.txt", std::ios::in);
    while (std::getline(file, line))
    {
        if (line == "#user") 
        {
            // Get user id
            std::getline(file, line);
            UserServer* user = new UserServer();
            user->userid = line;

            // Get pathnames
            std::getline(file, line);
            if (line == "#files")
            {
                while (line != "#end") {
                    std::getline(file, line);
                    if (line != "#end"){
                        File newFile;
                        newFile.pathname = line;
                        
                        std::getline(file, line);
                        newFile.last_modified = line;

                        std::getline(file, line);
                        newFile.access_time = line;

                        std::getline(file, line);
                        newFile.creation_time = line;
                        user->files.push_back(newFile);
                    }
                }
            }

            // Add in list
            usersDB.push_back(user);
        }
    }
    file.close();

    return usersDB;
}

/* Create new ports to be used in sockets */
int createNewPort(std::list<int> portsInUse){
	int newPort;
	std::list<int>::iterator findIter;
	
	while(1) {
		newPort = std::rand() % 2000 + SERVER_PORT;
		findIter = std::find(portsInUse.begin(), portsInUse.end(), newPort);
		if (*findIter == (int) portsInUse.size()) {
			portsInUse.push_back(newPort);
			return newPort;
		}
	}
}

/* Parse data ports */
std::pair<int, int> getPorts(char* data) {
	std::string port = std::string(data);
	int p1 = std::stoi(port.substr(0,4));
	int p2 = std::stoi(port.substr(4,8));

	return std::make_pair(p1, p2);
}

/* Get currente date time */
std::string getCurrentTime(){
    time_t     now = time(0);
    struct tm  tstruct;
    char       timeNow[80];
    tstruct = *localtime(&now);
    strftime(timeNow, sizeof(timeNow), "%Y/%m/%d %H:%M:%S", &tstruct);

    return std::string(timeNow);
}

/* DEBUG */
void printUsersServer(std::list<UserServer*> users)
{
    for (std::list<UserServer*>::iterator it = users.begin(); it != users.end(); ++it)
    {
        std::cout << "User: " << (*it)->userid << "\n";
        if (!(*it)->files.empty())
        {
            std::cout << "#files: " << "\n";
            for (std::list<File>::iterator f = (*it)->files.begin(); f != (*it)->files.end(); ++f)
            {
                std::cout << f->pathname << "\t";
                std::cout << f->last_modified << "\t\n";
                std::cout << f->access_time << "\t\n";
                std::cout << f->creation_time << "\t\n";
            }
        }
        std::cout << "\n\n";
	}
}

std::string getByHashString(std::string hashString, int elementIndex)
{
    std::size_t posInit = 0, posEnd;
    int hashes = 1;

    if (elementIndex == 0){
        posEnd = hashString.find("#");
        if (posEnd != std::string::npos)
            return hashString.substr(posInit, posEnd);
    }

    while (posInit != std::string::npos)
    {
        posInit = hashString.find("#", posInit + 1);
        if (posInit != std::string::npos && hashes == elementIndex)
        {
            posEnd = hashString.find("#", posInit + 1);
            if (posEnd != std::string::npos)
                return hashString.substr(posInit+1, posEnd - posInit - 1);
            else
                return hashString.substr(posInit+1, hashString.length() - posInit);
        }
        hashes++;
    }

    return std::string();
}

std::string getIP(){
	struct ifaddrs * ifAddrStruct = NULL, * ifa = NULL;
    void * tmpAddrPtr = NULL;
    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family == AF_INET) {
            char mask[INET_ADDRSTRLEN];
            void* mask_ptr = &((struct sockaddr_in*) ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, mask_ptr, mask, INET_ADDRSTRLEN);
            if (strcmp(mask, "255.0.0.0") != 0) {
                tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                return std::string(addressBuffer);
            }
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);

	return std::string();
}