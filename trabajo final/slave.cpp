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
#include "auxiliary_function.h"

using namespace std;

int LengthSize = 4;

//Variables globales
multimap<string, string> NodeDB;

multimap<string, string> RelationDB;

vector<int> ClientSockets;

vector<thread> ThreadsRecibiendo;

void PrintInfoServer(){
    cout << "Nodos" << endl;
    cout << "Nombre\tValor" << endl;
    for(multimap<string, string>::iterator it = NodeDB.begin(); it != NodeDB.end(); it++){
        cout << it->first << "\t" << it->second << endl;
    }
    cout << endl;
    cout << "Relaciones" << endl;
    cout << "Nombre 1 \tNombre 2" << endl;
    for(multimap<string, string>::iterator it = RelationDB.begin(); it != RelationDB.end(); it++){
        cout << it->first << "\t" << it->second << endl;
    }
}




void PrintNode(){
  cout << "Nodos" << endl;
  cout << "Nombre\tValor" << endl;
  for(multimap<string, string>::iterator it = NodeDB.begin(); it != NodeDB.end(); it++){
      cout << it->first << "\t" << it->second << endl;
  }
}

void PrintNodeRelations(){
  cout << "Relaciones" << endl;
  cout << "Nombre 1 \tNombre 2" << endl;
  for(multimap<string, string>::iterator it = RelationDB.begin(); it != RelationDB.end(); it++){
      cout << it->first << "\t" << it->second << endl;
  }
}

void SendMssg(int SocketClient, string mensaje){
    int sizetotal = mensaje.length();
    string size = size_to_string(sizetotal, LengthSize);
    int n = send(SocketClient, size.c_str(), size.length(), 0);
    if(n < 0){
        perror("Error sending to socket");
    }
    //
    n = send(SocketClient, mensaje.c_str(), mensaje.length(), 0);
    if(n < 0){
        perror("Error sending to socket");
    }
}


void ProccessMessage(int SocketClient, string mensaje)
{
    //Command Insert received
    if(mensaje[0] == '1')
    {
        //cout << mensaje << endl;
        //Code Node received
        if(mensaje[1] == '1')
        {
            cout<<"[Slave said: Inserting new node.]"<<endl;
            //Splitting data into name and value
            int TamNombre = stoi(mensaje.substr(2, LengthSize));
            string Nombre = mensaje.substr(2 + LengthSize, TamNombre);
            int TamValue = stoi(mensaje.substr(2 + LengthSize + TamNombre, LengthSize));
            string Value = mensaje.substr(2 + LengthSize + TamNombre + LengthSize, TamValue);
            multimap<string, string>::iterator it = NodeDB.find(Nombre);

            if (it == NodeDB.end())
            {
                string respuesta = "O";
                NodeDB.insert(pair<string, string>(Nombre, Value));
                SendMssg(SocketClient, respuesta);
            }

            else
            {
                SendMssg(SocketClient, "E");
            }

            PrintNode();
        }

        else if(mensaje[1] == '2')
        {
            cout<<"[Slave said: Inserting new relationship.]"<<endl;
            int TamNombre1 = stoi(mensaje.substr(2, LengthSize));
            string Nombre1 = mensaje.substr(2 + LengthSize, TamNombre1);
            int TamNombre2 = stoi(mensaje.substr(2 + LengthSize + TamNombre1, LengthSize));
            string Nombre2 = mensaje.substr(2 + LengthSize + TamNombre1 + LengthSize, TamNombre2);
            multimap<string, string>::iterator it1 = NodeDB.find(Nombre1);
            if(it1 == NodeDB.end())
            {//encontre algo
                string respuesta = "E";
                SendMssg(SocketClient, respuesta);//borrado con exito
            }
            /*else if(it2 == NodeDB.end()){
                string respuesta = "E";
                SendMssg(SocketClient, respuesta);//borrado con exito
            }*/
            else
            {
                //busco por rango el nombre 1
                //dentro de los resultados veo si esta nombre 2, si esta no inserto, si no esta inserto
                pair<multimap<string,string>::iterator, multimap<string, string>::iterator> range = RelationDB.equal_range(Nombre1);
                bool ExistRelation = false;
                for(multimap<string, string>::iterator it = range.first; it != range.second; it++)
                {
                    if(it->second == Nombre2)
                    {
                        ExistRelation = true;
                    }
                }
                if(!ExistRelation)
                {
                    RelationDB.insert(pair<string, string>(Nombre1, Nombre2));
                    string respuesta = "O";
                    SendMssg(SocketClient, respuesta);//borrado con exito
                }
                else
                {
                    string respuesta = "E";
                    SendMssg(SocketClient, respuesta);//borrado con exito
                }
            }
            PrintNodeRelations();
        }
        //mandar respuesta
        //SendMssg(SocketClient, mensaje);//por ahora lo mando de regreso
    }
    else if(mensaje[0] == '2')
    {
        cout<<"[Slave said: Updating node.]"<<endl;
        //procesar comando
        //cout << mensaje << endl;
        if(mensaje[1] == '1'){
            int TamNombre = stoi(mensaje.substr(2, LengthSize));
            string Nombre = mensaje.substr(2 + LengthSize, TamNombre);
            int TamValue = stoi(mensaje.substr(2 + LengthSize + TamNombre, LengthSize));
            string Value = mensaje.substr(2 + LengthSize + TamNombre + LengthSize, TamValue);
            multimap<string, string>::iterator it = NodeDB.find(Nombre);
            if(it != NodeDB.end()){//encontre algo
                string respuesta = "O";
                it->second = Value;
                SendMssg(SocketClient, respuesta);//borrado con exito
            }
            else{
                SendMssg(SocketClient, "E");//el nodo no existia
            }
            PrintNode();
        }
        else if(mensaje[1] == '2')
        {
            //Working on it
          //PrintNodeRelations();
        }
        //mandar respuesta
    }
    else if(mensaje[0] == '3')
    {
        cout<<"[Slave said: Erasing node.]"<<endl;
        //procesar comando
        //cout << mensaje << endl;
        if(mensaje[1] == '1')
        {
            int TamNombre = stoi(mensaje.substr(2, LengthSize));
            string Nombre = mensaje.substr(2 + LengthSize, TamNombre);
            multimap<string, string>::iterator it = NodeDB.find(Nombre);
            if(it != NodeDB.end()){//encontre algo
                string respuesta = "O";
                NodeDB.erase(it);
                SendMssg(SocketClient, respuesta);//borrado con exito
            }
            else{
                SendMssg(SocketClient, "E");//el nodo no existia
            }
            PrintNode();
        }
        else if(mensaje[1] == '2')
        {
            int TamNombre1 = stoi(mensaje.substr(2, LengthSize));
            string Nombre1 = mensaje.substr(2 + LengthSize, TamNombre1);
            int TamNombre2 = stoi(mensaje.substr(2 + LengthSize + TamNombre1, LengthSize));
            string Nombre2 = mensaje.substr(2 + LengthSize + TamNombre1 + LengthSize, TamNombre2);
            multimap<string, string>::iterator it1 = RelationDB.find(Nombre1);
            multimap<string, string>::iterator it2 = NodeDB.find(Nombre2);//debo ver que existan los nodso
            if(it1 == NodeDB.end()){//encontre algo
                string respuesta = "E";
                SendMssg(SocketClient, respuesta);//borrado con exito
            }
            else if(it2 == NodeDB.end()){
                string respuesta = "E";
                SendMssg(SocketClient, respuesta);//borrado con exito
            }
            else{
                //busco por rango el nombre 1
                //dentro de los resultados veo si esta nombre 2, si esta no inserto, si no esta inserto
                pair<multimap<string,string>::iterator, multimap<string, string>::iterator> range = RelationDB.equal_range(Nombre1);
                bool ExistRelation = false;
                multimap<string, string>::iterator iterase;
                for(multimap<string, string>::iterator it = range.first; it != range.second; it++){
                    if(it->second == Nombre2){
                        ExistRelation = true;
                        iterase = it;
                    }
                }
                if(ExistRelation){
                    RelationDB.erase(iterase);
                    string respuesta = "O";
                    SendMssg(SocketClient, respuesta);//borrado con exito
                }
                else{
                    string respuesta = "E";
                    SendMssg(SocketClient, respuesta);//borrado con exito
                }
            }
            PrintNodeRelations();
        }
        //mandar respuesta
    }
    //PrintInfoServer();
}


void Rcv_Client(int SocketClient){
    //recibo mensajes hasta hartarme, proceso la consulta y la devuelvo, este si es de enfoque tradicionarl
    int n;
    char * buffersize = new char [LengthSize];

    while(true){
        n = recv(SocketClient, buffersize, LengthSize, 0);
        if (n < 0){
            perror("Error size from socket");
        }

        if(n>0){
            int size = atoi(buffersize);
            char * buffermensaje = new char [size];

            n = recv(SocketClient, buffermensaje, size, 0);
            if (n < 0){
                perror("Error reading message from socket");
            }

            if(n>0){
                string smensaje (buffermensaje, size);
                //cout << "Mensaje recibido del cliente: " << smensaje << endl;
                ProccessMessage(SocketClient, smensaje);

            }
            delete [] buffermensaje;
        }

    }
    delete [] buffersize;
}

////debo escuchar clientes, puede haber varios gestores
void Listening(int SocketI){//aun que solo tendre un unico gestor, no se si necesite esto
    cout<<"[Slave status: Connected.]"<<endl;
    while(true){
        int in;
        in = accept(SocketI, NULL, NULL);//no conozco al participante activo
        //se supone que in, es un nuevo socket, el socket
        if (in < 0){
            perror("accept failed");
            //close(SocketI);
            exit(EXIT_FAILURE);
        }
        else{
            ClientSockets.push_back(in);
            //thread ThreadRcv_Client;
            //ThreadRcv_Client = thread(Rcv_Client, in);
            ThreadsRecibiendo.push_back(thread(Rcv_Client, in));

            //ThreadsEnviandoCliente.push_back(ThreadEnviando);
            //solo deberia haber un enviando
        }
    }
}


//inicialiazacion del socket con el numero de puerto como parametro
int init(int slave_port)
{
  int SocketI = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//creando la interfaz
  if(SocketI == -1) {
      perror("can not create socket");
      exit(EXIT_FAILURE);
  }

  struct sockaddr_in SSocketAddr;

  memset(&SSocketAddr, 0, sizeof(struct sockaddr_in));
  SSocketAddr.sin_family = AF_INET;
  SSocketAddr.sin_port = htons(slave_port);
  SSocketAddr.sin_addr.s_addr = INADDR_ANY;

  int status = bind(SocketI, (const struct sockaddr *)&SSocketAddr, sizeof(struct sockaddr_in));
  if(status == -1){
      perror("bind failed");
      close(SocketI);
      exit(EXIT_FAILURE);
  }

  status = listen(SocketI, 10);
  if(status == -1) {
      perror("listen failed");
      close(SocketI);
      exit(EXIT_FAILURE);
  }
  return SocketI;
}


int main(){
    int slave_port;
    cout<<"SLAVE"<<endl;
    cout<<"Port Number: ";
    cin >> slave_port;
    int SocketI = init(slave_port);

    thread EsperandoClients(Listening, SocketI);

    for(int i = 0; i < ThreadsRecibiendo.size(); i++)
    {
        ThreadsRecibiendo[i].join();
    }

    EsperandoClients.join();

    return 0;
}