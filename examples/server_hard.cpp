struct sockaddr_in sock;
		struct sockaddr_in fr;
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
		sock.sin_addr.s_addr = INADDR_ANY;
		std::cout << "if (bind(s, (struct sockaddr *) &sock, sizeof(struct sockaddr)) < 0)\n";
		if (bind(s, (struct sockaddr *) &sock, sizeof(struct sockaddr)) < 0) 
		{
			fprintf(stderr,"ERROR: bind host\n");
			return 1;
		}

		socklen_t length = sizeof(struct sockaddr_in);
		char* buffer = (char*) calloc(1, BUFFER_SIZE);
		std::cout << "recvfrom\n";
		int n = recvfrom(s, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &fr, &length);
		std::cout << "buffer" << buffer << '\n';