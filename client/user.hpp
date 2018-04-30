#include <string>
#include <queue>
#include "request.hpp"
#include "socket.hpp"

class User
{
	public:
        std::string name;
        bool isConnected;
        std::queue<Request> requestsToSend;
        std::queue<Request> requestsToReceive;

        void addRequestToSend(Request newRequest);
        void addRequestToReceive(Request newRequest);
        void executeRequest(Socket* socket);
        void processResquest(Socket* socket);
        void login();
        void logout();
        
};
