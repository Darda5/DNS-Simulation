#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <list>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>

std::ifstream file("root");
using namespace std;

#define DEFAULT_BUFLEN 50
#define DEFAULT_PORT   27016

map<string, string> localSites;


class Site{
    private:
        string ip;
        string name;
    public:
        void setIP(string s){
            ip = s;
        }

        void setName(string s){
            name = s;
        }

        string getName(){
            return name;
        }
        string getIP(){
            return ip;
        }
};

Site split(string s, string del);
string split_v2(string s, string del);
void handle_connection(int client_sock);

#include<stdio.h>
#include<string.h>   
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include<pthread.h>
#include<stdlib.h>

void * tcp_handle_conn (void * client_socket);

int main(int argc , char *argv[])
{
    //////////////////////////////////////////////////////////////////////////
        if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {

            Site b;
            b = split(line, ":");
            localSites.insert({b.getName(), b.getIP()});
        }
        file.close();
}

    cout <<"\n##########################################" << endl;
    map<string, string>::iterator it = localSites.begin();
    while (it != localSites.end())
    {
        cout << "Site: " << it->first << ", IP: " << it->second << std::endl;
        ++it;
    }
    cout <<"\n##########################################" << endl;
    //////////////////////////////////////////////////////////////////////////
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    
   
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
return 1;
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
while (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))
{
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    pthread_t t;
    int *pclient = (int *)malloc (sizeof(int));
    *pclient = client_sock;
    pthread_create(&t, NULL, tcp_handle_conn, pclient);

}

    close(socket_desc);
    close(client_sock);

    return 0;
}

void * tcp_handle_conn (void * client_socket)
{
    int read_size;
    int sock = *((int*)client_socket);
    free(client_socket);
    char client_message[DEFAULT_BUFLEN];

    //Receive a message from client
    while( (read_size = recv(sock, client_message , DEFAULT_BUFLEN , 0)) > 0 )
    {
        bool flag = false;
        client_message[read_size]='\0';
        string s = "";    
        printf("Message received: %s\nBytes received: %d\n\n", client_message, read_size);
        for(int i = 0; i < sizeof(client_message); i++){
            if(client_message[i] == '.'){
                flag = true;
            }
            if(client_message[i] == '\0'){
                break;
            }
            s = s + client_message[i];
        }
        cout << s << endl;
        string x;
        if(flag == true){
            x = split_v2(s, "."); //string za pretragu u mapi
        }
        else{
            x = client_message;
        }
        
        if(localSites.find(x)!=localSites.end()){
        //if the element is found before the end of the map
            string connect;
            connect = localSites[x];
            char cstr[connect.size() + 1];
            connect.copy(cstr, connect.size() + 1);
            cstr[connect.size()] = '\0';

            send(sock , cstr , strlen(cstr), 0);
        }
        else{
            char *message = "x";
            send(sock , message , strlen(message), 0);

        }
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }



}

Site split(string s, string del)
{   
    Site site;
    string x;
    int i = 1;
	int start, end = -1 * del.size();
	do {
		start = end + del.size();
		end = s.find(del, start);
        x = s.substr(start, end - start);
        if( i % 2 != 0){
            site.setName(x);
        }
        else{
            site.setIP(x);
        }
        i++;
	} while (end != -1);

    return site;
}
string split_v2(string s, string del = " "){
    string x;
    int start, end = -1 * del.size();
	start = end + del.size();
	end = s.find(del, start);
    x = s.substr(start, end - start);

    return x;
        
}
