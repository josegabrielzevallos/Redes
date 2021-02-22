#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <algorithm>
#include <time.h>
#include <map>
#include <iterator>
#include <queue>

#include "auxiliary_function.h"

using namespace std;

class Client
{
public:	
    int fixed_command_size;
    int number_servers;
    vector<int> sockets_servers;

    Client();
    void connect_with_slaves();
	void send_keep_alive();
	void parse_input(vector<string> words_input_text);
    int hash_function(string text);
    void send_command(int client_socket, string command_to_send);
    string receive_command(int client_socket);
    void operation_insert(vector<string> list_words_input);
    void operation_update(vector<string> list_words_input);
    void operation_delete(vector<string> list_words_input);
};

Client::Client()
{
    fixed_command_size = 4;
    number_servers = 2;
}

void Client::connect_with_slaves()
{
    for(int i = 0; i < number_servers; i++)
    {
        int SocketI = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(SocketI == -1) 
        {
            perror("can not create socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in SSocketAddr;

        memset(&SSocketAddr, 0, sizeof(struct sockaddr_in));
        SSocketAddr.sin_family = AF_INET;
        cout<<"i "<<i<<endl;
        SSocketAddr.sin_port = htons(1100 + i);
        //SSocketAddr.sin_addr.s_addr = INADDR_ANY;

        int In = inet_pton(AF_INET, "127.0.0.1", &SSocketAddr.sin_addr);

        if (In < 0) {
            perror("first parameter is not a valid address family");
            close(SocketI);
            exit(EXIT_FAILURE);
        }
        else if (In == 0) {
            perror("char string (second parameter does not contain valid ipaddress");
            close(SocketI);
            exit(EXIT_FAILURE);
        }

        int status = connect(SocketI, (const struct sockaddr *)&SSocketAddr, sizeof(struct sockaddr_in));
        if(status == -1){
            perror("connect failed");
            close(SocketI);
            exit(EXIT_FAILURE);
        }
        sockets_servers.push_back(SocketI);
    }

}

int Client::hash_function(string text)
{
    return int(text[0]) % number_servers;
}

void Client::parse_input(vector<string> words_input_text)
{
    if (words_input_text[0][0] == 'i')
    {
        operation_insert(words_input_text);
    }
    else if (words_input_text[0][0] == 'u')
    {
        operation_update(words_input_text);
    }
    else if (words_input_text[0][0] == 'd')
    {
        operation_delete(words_input_text);
        //cout<<"words_input_text: Delete"<<endl;
    }
    else if (words_input_text[0][0] == 'q')
    {
        cout<<"Comando: Query"<<endl;
    }
}

void Client::operation_insert(vector<string> list_words_input)
{
  int fixed_command_size = 4;

    if (list_words_input[1][0] == 'n')
    {
        int socket_dest = hash_function(list_words_input[2]);
        string to_send = "11";
        to_send += size_to_string(list_words_input[2].size(), fixed_command_size);
        to_send += list_words_input[2];
        to_send += size_to_string(list_words_input[3].size(), fixed_command_size);
        to_send += list_words_input[3];

        //cout<<"Sending: "<<to_send<<endl;
        
        send_command(sockets_servers[socket_dest], to_send);
        
        string server_reply = receive_command(sockets_servers[socket_dest]);

        if (server_reply[0] == 'O')
        {
            cout<<"[Server "<<socket_dest<<" said: Node added.]"<<endl;
        }
        else
        {
            cout<<"Error."<<endl;
        }
    }

    else if (list_words_input[1][0] == 'r')
    {
        int socket_dest_A = hash_function(list_words_input[2]);
        int socket_dest_B = hash_function(list_words_input[3]);
        
        string temp_1 = size_to_string(list_words_input[2].length(), fixed_command_size);
        string temp_2 = size_to_string(list_words_input[3].length(), fixed_command_size);

        string to_send = "12";
        to_send += temp_1;
        to_send += list_words_input[2];        
        to_send += temp_2;
        to_send += list_words_input[3];

        //cout<<"Sending: "<<to_send<<endl;
        
        send_command(sockets_servers[socket_dest_A], to_send);

        string server_reply = receive_command(sockets_servers[socket_dest_A]);

        to_send = "12";
        to_send += temp_2;
        to_send += list_words_input[3];
        to_send += temp_1;
        to_send += list_words_input[2];

        send_command(sockets_servers[socket_dest_B], to_send);
        
        server_reply = receive_command(sockets_servers[socket_dest_B]);
        
        if (server_reply[0] == 'O')
        {
            cout<<"[Slave "<<socket_dest_A<<" said: Relationship added.]"<<endl;
        }
        else
        {
            cout<<"Error."<<endl;
        }
    }
}

void Client::send_command(int client_socket, string command_to_send)
{
    string command_size = size_to_string(command_to_send.size(), fixed_command_size);

    int n = send(client_socket, command_size.c_str(), command_size.size(), 0);

    if(n < 0)
    {
        perror("Error sending to socket");
    }

    n = send(client_socket, command_to_send.c_str(), command_to_send.size(), 0);

    if(n < 0)
    {
        perror("Error sending to socket");
    }
}

string Client::receive_command(int client_socket)
{
    int n;
    char *first_command_size = new char[fixed_command_size];

    bool waiting_for_answer = true;

    while (waiting_for_answer)
    {
        n = recv(client_socket, first_command_size, fixed_command_size, 0);

        if (n>0)
        {
            int command_size = atoi(first_command_size);
            char *reading_message = new char[command_size];

            n = recv(client_socket, reading_message, command_size, 0);

            if (n<0)
            {
                perror("Error reading message from socket");
            }

            if (n>0)
            {
                string received_message(reading_message, command_size);
                //cout<<"[LOG - Server said: "<<received_message<<"]"<<endl;                
                waiting_for_answer = false;
                return received_message;
            }
            delete [] reading_message;
        }
    }
    delete [] first_command_size;
    return "";
}

void Client::operation_update(vector<string> list_words_input)
{
    if(list_words_input[1][0] == 'n')
    {
        int socket_dest = hash_function(list_words_input[2]);

        string to_send = "21";
        to_send += size_to_string(list_words_input[2].size(), fixed_command_size);
        to_send += list_words_input[2];
        to_send += size_to_string(list_words_input[3].size(), fixed_command_size);
        to_send += list_words_input[3];

        send_command(sockets_servers[socket_dest], to_send);

        string server_reply = receive_command(sockets_servers[socket_dest]);
    }

    else if(list_words_input[1] == "R")
    {
        //Working on it
    }       
}

void Client::operation_delete(vector<string> list_words_input)
{
    if (list_words_input[1][0] == 'n')
    {
        int socket_dest = hash_function(list_words_input[2]);
        
        string to_send = "31";
        to_send += size_to_string(list_words_input[2].length(), fixed_command_size);
        to_send += list_words_input[2];
        
        send_command(sockets_servers[socket_dest], to_send);
    
        string server_reply = receive_command(sockets_servers[socket_dest]);

        if (server_reply[0] == 'O')
        {
            cout<<"[Slave "<<socket_dest<<" said: Node deleted.]"<<endl;
        }
        else
        {
            cout<<"Error."<<endl;
        }
    }

    else if (list_words_input[1][0] == 'r')
    {
        int socket_dest_A = hash_function(list_words_input[2]);
        int socket_dest_B = hash_function(list_words_input[3]);
        
        string temp_1 = size_to_string(list_words_input[2].length(), fixed_command_size);
        string temp_2 = size_to_string(list_words_input[3].length(), fixed_command_size);

        string to_send = "32";
        to_send += temp_1 + list_words_input[2];
        to_send += temp_2 + list_words_input[3];
        
        send_command(sockets_servers[socket_dest_A], to_send);
        
        string server_reply = receive_command(sockets_servers[socket_dest_A]);
        //string respuesta = RecibiendoCliente(SocketsServers[socket_dest_A]);

        to_send = "32";
        to_send += temp_2 + list_words_input[3];
        to_send += temp_1 + list_words_input[2];

        send_command(sockets_servers[socket_dest_B], to_send);

        server_reply = receive_command(sockets_servers[socket_dest_B]);
        
        //respuesta = RecibiendoCliente(SocketsServers[socket_dest_B]);

        if (server_reply[0] == 'O')
        {
            cout<<"[Slave "<<socket_dest_A<<" said: Relationship deleted.]"<<endl;
        }
        else
        {
            cout<<"Error."<<endl;
        }

    }
}


#endif