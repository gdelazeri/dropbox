#include "socket.hpp"

// struct sockaddr_in from;

Socket::Socket(int side)
{
	this->frontEnd = new FrontEnd(side);
	this->side = side;
}

int Socket::login_server(std::string host, int port)
{
	return this->frontEnd->login(host, port);
}

int Socket::createSocket(int port)
{
	return this->frontEnd->create(port);
}

// bool Socket::sendAck()
// {
// 	int n;
// 	char* buffer = (char*) calloc(1, BUFFER_SIZE);
// 	buffer[0] = ACK;

// 	if (this->side == SOCK_SERVER)
// 		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
// 	else
// 		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr_in));

// 	if (n < 0)
// 	{
// 		std::cout << "ERROR sendto";
// 		return false;
// 	}
// 	return true;
// }

// bool Socket::waitAck()
// {
// 	socklen_t length = sizeof(struct sockaddr_in);
// 	char* buffer = (char*) calloc(1, BUFFER_SIZE);

// 	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
// 	if (n < 0)
// 	{
// 		std::cout << "ERROR recvfrom";
// 		return false;
// 	}
// 	if (buffer[0] == ACK)
// 		return true;
	
// 	return false;
// }


// bool Socket::sendDatagram(tDatagram datagram)
// {
// 	int n;
// 	char* buffer = (char*) calloc(1, BUFFER_SIZE);

// 	memcpy(buffer, &datagram, sizeof(datagram));
// 	if (this->side == SOCK_SERVER) {
// 		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &from, sizeof(struct sockaddr));
// 	}
// 	else {
// 		n = sendto(this->socketFd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &this->socketAddress, sizeof(struct sockaddr_in));
// 	}
// 	if (n < 0)
// 	{
// 		std::cout << "ERROR: sendto";
// 		return false;
// 	}

// 	if (!this->waitAck())
// 	{
// 		std::cout << "ERROR: ack miss";
// 		return false;
// 	}
	
// 	return true;
// }

// tDatagram Socket::receiveDatagram()
// {
// 	tDatagram datagram;
// 	socklen_t length = sizeof(struct sockaddr_in);
// 	char* buffer = (char*) calloc(1, BUFFER_SIZE);

// 	int n = recvfrom(this->socketFd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &from, &length);
// 	if (n < 0)
// 	{
// 		std::cout << "ERROR: recvfrom";
// 		datagram.type = ERROR;
// 		return datagram;
// 	}
// 	memcpy(&datagram, buffer, sizeof(datagram));
// 	this->frontEnd->sendAck();

// 	return datagram;
// }

bool Socket::send_file(std::string pathname, std::string modificationTime, std::string accessTime, std::string creationTime)
{
	File fileHelper;
	fileHelper.pathname = pathname;

	if (!fileHelper.exists()){
		std::cout << "ERROR: This file not exists!";
		return false;
	}

	tDatagram datagram;
	std::fstream file;
	int bytesSent = 0, bytesToRead = 0, fileSize = fileHelper.getSize();
	char buffer[MAX_DATA_SIZE];

	file.open(pathname.c_str(), std::ios::binary | std::ios::in);

	// Send filename
	datagram.type = BEGIN_FILE_TYPE;
	strcpy(datagram.data, fileHelper.getFilename().c_str());

	this->frontEnd->sendDatagram(datagram);

	// Send file
	datagram.type = FILE_TYPE;
	while(bytesSent < fileSize)
	{
		bzero(buffer, MAX_DATA_SIZE);
		bytesToRead = (fileSize - bytesSent < MAX_DATA_SIZE-1) ?  (fileSize - bytesSent) : MAX_DATA_SIZE-1;
		file.read(buffer, bytesToRead);
		strcpy(datagram.data, (char *) buffer);
		this->frontEnd->sendDatagram(datagram);
		bytesSent += bytesToRead;
	}

	// Send end of file
	datagram.type = END_DATA;
	this->frontEnd->sendDatagram(datagram);

	// Send last modification time
	datagram.type = MODIFICATION_TIME;
	if (!modificationTime.empty()) {
		if (accessTime.empty())
			accessTime = fileHelper.getTime('A');
		if (creationTime.empty())
			creationTime = fileHelper.getTime('C');
		strcpy(datagram.data, (modificationTime + "#" + accessTime + "#" + creationTime).c_str());
	}
	else {
		strcpy(datagram.data, (fileHelper.getTime('M') + "#" + fileHelper.getTime('A') + "#" + fileHelper.getTime('C')).c_str());
	}
		

	this->frontEnd->sendDatagram(datagram);

	file.close();

	return true;
}

// Used just by Client Side Socket
std::string Socket::get_file(std::string filename, std::string path)
{
	tDatagram datagram;

	// Request file
	datagram.type = GET_FILE_TYPE;
	strcpy(datagram.data, filename.c_str());
	this->frontEnd->sendDatagram(datagram);

	datagram = this->frontEnd->receiveDatagram();
	if (datagram.type == BEGIN_FILE_TYPE)
		return this->receive_file(path + std::string(datagram.data));

	return std::string();
}


std::string Socket::receive_file(std::string filename)
{
	tDatagram datagram;
	std::fstream file;
	file.open(filename.c_str(), std::ios::binary | std::ios::out);
	datagram = this->frontEnd->receiveDatagram();
	while(datagram.type == FILE_TYPE && datagram.type != END_DATA)
	{
		file.write(datagram.data, strlen(datagram.data));
		datagram = this->frontEnd->receiveDatagram();
	}
	
	file.close();

	// get the modification time from the file
	datagram = this->frontEnd->receiveDatagram();
	if (datagram.type == MODIFICATION_TIME) {
		return std::string(datagram.data);
	}
	return std::string();
}

bool Socket::close_session()
{
	tDatagram datagram;

	datagram.type = CLOSE;
	return this->frontEnd->sendDatagram(datagram);
}

// void Socket::finish()
// {
// 	close(this->socketFd);
// }

std::list<File> Socket::list_server()
{
	tDatagram datagram;
	std::list<File> filesList;
	
	datagram.type = LIST_SERVER;
	this->frontEnd->sendDatagram(datagram);

	datagram = this->frontEnd->receiveDatagram();
	while(datagram.type == FILE_INFO && datagram.type != END_DATA)
	{
		File newFile;

		int pos = 0, posEnd;
		std::string fileInfo = std::string(datagram.data);
		
		posEnd = fileInfo.find("#");
		newFile.filename = fileInfo.substr(pos, posEnd-pos);

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		newFile.size = atoi(fileInfo.substr(pos, posEnd-pos).c_str());

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		newFile.last_modified = fileInfo.substr(pos, posEnd-pos);

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		newFile.access_time = fileInfo.substr(pos, posEnd-pos);

		pos = posEnd+1;
		posEnd = fileInfo.find("#", posEnd+1);
		newFile.creation_time = fileInfo.substr(pos, posEnd-pos);

		filesList.push_back(newFile);

		datagram = this->frontEnd->receiveDatagram();
	}

	return filesList;
}


void Socket::send_list_server(UserServer* user)
{
	for (std::list<File>::iterator f = user->files.begin(); f != user->files.end(); ++f)
	{
		tDatagram datagram;
		std::string fileInfo;
		File* file = new File();
		file->pathname = f->pathname;
		
		fileInfo = file->getFilename();
		fileInfo += "#";
		fileInfo += std::to_string(file->getSize());
		fileInfo += "#";
		fileInfo += std::string(f->last_modified);
		fileInfo += "#";
		fileInfo += std::string(f->access_time);
		fileInfo += "#";
		fileInfo += std::string(f->creation_time);

		datagram.type = FILE_INFO;
		strcpy(datagram.data, fileInfo.c_str());
		this->frontEnd->sendDatagram(datagram);
	}

	tDatagram datagram;
	datagram.type = END_DATA;
	this->frontEnd->sendDatagram(datagram);
}

std::list<std::string> Socket::listDeleted()
{
	tDatagram datagram;
	std::list<std::string> deletedFiles = std::list<std::string>();
	
	datagram.type = LIST_DELETED;
	this->frontEnd->sendDatagram(datagram);

	datagram = this->frontEnd->receiveDatagram();
	while(datagram.type == DELETED_FILE && datagram.type != END_DATA)
	{
		deletedFiles.push_back(std::string(datagram.data));
		datagram = this->frontEnd->receiveDatagram();
	}

	return deletedFiles;
}

void Socket::sendListDeleted(UserServer* user)
{
	if (user->deleted.size() > 0 && user->deleted.size() < 1000) {
		std::list<std::pair<std::string, int>>::iterator f = user->deleted.begin();
		while (f != user->deleted.end())
		{
			tDatagram datagram;
			datagram.type = DELETED_FILE;
			strcpy(datagram.data, f->first.c_str());
			this->frontEnd->sendDatagram(datagram);
			f->second = f->second - 1;
			if (f->second == 0)
				user->deleted.erase(f++);
			else
				f++;
		}
	}

	tDatagram datagram;
	datagram.type = END_DATA;
	this->frontEnd->sendDatagram(datagram);
}

void Socket::deleteFile(std::string filename)
{
	tDatagram datagram;

	datagram.type = DELETE_TYPE;
	strcpy(datagram.data, filename.c_str());
	this->frontEnd->sendDatagram(datagram);
}

std::string Socket::waitNewServer()
{
	tDatagram datagram;
	datagram = this->frontEnd->receiveDatagram();
	
	return std::string(datagram.data);
}

int Socket::connectNewServer(std::string host, int port)
{
	this->frontEnd->finish();
	delete this->frontEnd;

	this->frontEnd = new FrontEnd(SOCK_CLIENT);
	return this->login_server(host, port);
}