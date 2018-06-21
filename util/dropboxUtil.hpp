#ifndef __DROPBOXUTIL_HPP__
#define __DROPBOXUTIL_HPP__

#include "userServer.hpp"
#include <list>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

/* Constants */
#define SERVER_PORT 4000
#define SERVER_NAME "[server@dropbox] "
#define CLIENT_NAME "[client@dropbox] "

/* Socket side */
#define SOCK_CLIENT 0
#define SOCK_SERVER 1

/* Datagram types */
#define ERROR 'E'
#define CLOSE 'C'
#define LOGIN 'L'
#define NEW_PORTS 'P'
#define GET_FILE_TYPE 'G'
#define BEGIN_FILE_TYPE 'I'
#define FILE_TYPE 'F'
#define END_DATA 'N'
#define ACK 'A'
#define LIST_SERVER 'S'
#define LIST_DELETED 'Y'
#define FILE_INFO 'O'
#define MODIFICATION_TIME 'T'
#define DELETE_TYPE 'D'
#define DELETED_FILE 'X'
#define BACKUP 'B'
#define LIVE_SIGNAL 'V'
#define ELECTION_TIME 'H'
#define NEW_PRIMARY 'J'

/* Sizes */
#define BUFFER_SIZE 6000
#define MAX_DATA_SIZE 5999

/* Requests types */
#define UPLOAD_REQUEST 1
#define EXIT_REQUEST 2
#define DOWNLOAD_REQUEST 3
#define LIST_SERVER_REQUEST 4
#define DOWNLOAD_SYNC_REQUEST 5
#define UPLOAD_SYNC_REQUEST 6
#define DELETE_REQUEST 7

/* Struct that defines the Datagram */
typedef struct tDatagram
{
  char type;
  char data[MAX_DATA_SIZE];
} tDatagram;

/* Util functions */
void saveUsersServer(std::list<UserServer*> users);
std::list<UserServer*> loadUsersServer();
void printUsersServer(std::list<UserServer*> users);
int createNewPort(std::list<int> portsInUse);
std::pair<int, int> getPorts(char* data);
std::string getCurrentTime();
std::string getByHashString(std::string hashString, int elementIndex);
std::string getIP();

#endif