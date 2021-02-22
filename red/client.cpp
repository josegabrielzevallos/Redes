#include <thread>
#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <strings.h>

//g++ client.cpp -o cli -std=c++11 -lpthread
// ./cli 51.15.220.108 4021
using namespace std;

bool status = true;

void write_thread(int socket_fd)
{
	while (status)
	{
		string request;
		getline(cin, request);
		write(socket_fd, request.c_str(), request.size());
		if(request == "8")
			status = false;
	}
}

void read_thread(int socket_fd)
{
	while (status)
	{
		char buffer[256];
		bzero(buffer, 256);
		read(socket_fd, buffer, 1);
		if(buffer=="G"){
			read(socket_fd, buffer, 7);
		}
		if(buffer=="P"){
			read(socket_fd, buffer, 8);
		}
		else{
			read(socket_fd, buffer, 18);
		}
		cout << buffer << endl;
	}
}

int main(int argc, char *argv[])
{
	int port_number = atoi(argv[2]);
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	server = gethostbyname(argv[1]);
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	
	connect(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	thread thr(read_thread, socket_fd);
	thread thw(write_thread, socket_fd);
	
	thr.join();
	thw.join();
	
	close(socket_fd);
	
	return 0;
}
