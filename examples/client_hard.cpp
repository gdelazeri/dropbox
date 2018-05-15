struct sockaddr_in sock;
	struct sockaddr_in fr;
	struct hostent *server;
	int s;
	std::cout << "if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)\n";
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "ERROR: open socket\n");
		exit(1);
	}
	sock.sin_port = htons(5000);    
	sock.sin_family = AF_INET;   
	bzero(&(sock.sin_zero), 8);
	server = gethostbyname(argv[2]);
	if (server == NULL) {
		fprintf(stderr,"ERROR: no such host\n");
		return 1;
	}
	sock.sin_addr = *((struct in_addr *)server->h_addr);

	std::string message = "kkkkkkkkkkkkkkkkk";
	int n = sendto(s, message.c_str(), strlen(message.c_str()), 0, (const struct sockaddr *) &sock, sizeof(struct sockaddr_in));