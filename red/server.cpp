#include <thread>
#include <string>
#include <iostream>
#include <map>
#include <vector>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <strings.h>
//g++ server.cpp -o ser -std=c++11 -lpthread
// ./ser 4021
using namespace std;

vector <int> sockets_jugadores;
string tablero[3][3];

void llenar_tablero()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			tablero[i][j] = " ";
		}        
	}    
}

string convertir_tablero()
{
	string tab = "T";
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if(j < 2)
				tab += tablero[i][j] + "|";  
			else
			{
				tab += tablero[i][j];
			}            
		}
		tab += "\n"; 
	}
	return tab;
}
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void mandar_tablero()
{
	for (int i = 0; i < sockets_jugadores.size(); i++)
	{
		string tab = convertir_tablero();
		write(sockets_jugadores[i], tab.c_str(), tab.size());
	}    
}

void move(char symb, char x, char y)
{
	string symbol(1, symb);
	if(x == '0' && y == '0')
		tablero[0][0] = symbol;
	if(x == '0' && y == '1')
		tablero[0][1] = symbol;
	if(x == '0' && y == '2')
		tablero[0][2] = symbol;
	if(x == '1' && y == '0')
		tablero[1][0] = symbol;
	if(x == '1' && y == '1')
		tablero[1][1] = symbol;
	if(x == '1' && y == '2')
		tablero[1][2] = symbol;
	if(x == '2' && y == '0')
		tablero[2][0] = symbol;
	if(x == '2' && y == '1')
		tablero[2][1] = symbol;
	if(x == '2' && y == '2')
		tablero[2][2] = symbol;
}

bool validar_victoria()
{
	if(tablero[0][0] != " " && (tablero[0][0] == tablero[0][1]) && (tablero[0][0] == tablero[0][2]))
		return true;
	if(tablero[1][0] != " " && (tablero[1][0] == tablero[1][1]) && (tablero[1][0] == tablero[1][2]))
		return true;
	if(tablero[2][0] != " " && (tablero[2][0] == tablero[2][1]) && (tablero[2][0] == tablero[2][2]))
		return true;
	
	if(tablero[0][0] != " " && (tablero[0][0] == tablero[1][0]) && (tablero[0][0] == tablero[2][0]))
		return true;
	if(tablero[0][1] != " " && (tablero[0][1] == tablero[1][1]) && (tablero[0][1] == tablero[2][1]))
		return true;
	if(tablero[0][2] != " " && (tablero[0][2] == tablero[1][2]) && (tablero[0][2] == tablero[2][2]))
		return true;
	
	if(tablero[0][0] != " " && (tablero[0][0] == tablero[1][1]) && (tablero[0][0] == tablero[2][2]))
		return true;
	if(tablero[0][2] != " " && (tablero[0][2] == tablero[1][1]) && (tablero[0][2] == tablero[2][0]))
		return true;
	
	return false;
}

void attend_client(int socket_fd)
{
	sockets_jugadores.push_back(socket_fd);
	bool status = true;
	while (status)
	{        
		char pos[3];
		read(socket_fd, pos, 3);
		move(pos[0], pos[1], pos[2]);
		mandar_tablero();
		if(validar_victoria()){
			write(socket_fd, "GGANASTE", 8);
			for(vector<int>::iterator it = sockets_jugadores.begin() ; it != sockets_jugadores.end(); ++it){
				if((*it)!=socket_fd)
					write((*it), "PPERDISTE", 9);
			}
		}
	}
	close(socket_fd);
}

int main(int argc, const char** argv) 
{
	if (argc < 2) 
	{
		cout << "Error de servidor: No se especificaron los parametros de inicializacion" << endl;
		exit(1);
	}
	
	llenar_tablero();
	int socket_fd, newsocket_fd, port_number = atoi(argv[1]);
	
	struct sockaddr_in serv_addr, cli_addr;
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);    
	
	socklen_t clilen = sizeof(cli_addr);
	
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) 
		error("Error abriendo el socket del servidor");
	
	if(bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("Error de bind");
	listen(socket_fd, 5);
	
	while (true)
	{
		newsocket_fd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsocket_fd < 0)
			error("Error abriendo el socket del cliente");
		thread th(attend_client, newsocket_fd);
		th.detach();
	}
	
	close(socket_fd);
	return 0; 
}
