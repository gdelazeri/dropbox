#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include "userServer.hpp"
#include <list>
#include <fstream>
#include <iostream>
void saveUsersServer(std::list<UserServer*> users);
std::list<UserServer*> loadUsersServer();
void printUsers(std::list<UserServer*> users);

#define SERVER_PORT 4000
#define SERVER_NAME "[server@dropbox] "
#define CLIENT_NAME "[client@dropbox] "

/* Socket side definition */
#define SOCK_CLIENT 0
#define SOCK_SERVER 1

/* Datagram definition */
#define ERROR 'E'
#define CLOSE 'C'
#define LOGIN 'L'
#define NEW_PORTS 'P'
#define GET_FILE_TYPE 'G'
#define BEGIN_FILE_TYPE 'I'
#define FILE_TYPE 'F'
#define END_FILE_TYPE 'N'
#define ACK 'A'
#define BUFFER_SIZE 6000
#define MAX_DATA_SIZE 5999

#define UPLOAD_REQUEST 1
#define EXIT_REQUEST 2
#define DOWNLOAD_REQUEST 3

typedef struct tDatagram
{
  char type;
  char data[MAX_DATA_SIZE];
} tDatagram;

#endif