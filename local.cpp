#include<stdio.h>
#include<string.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>    
#include <list>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include<pthread.h>
#include<stdlib.h>

std::ifstream file("local");
using namespace std;

#define DEFAULT_BUFLEN 50
#define DEFAULT_PORT   27015
#define LOCAL_PORT   27016

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
int handle_connection(int client_sock);
string convertToString(char* a, int size);
void updateMap(string name, string ip);

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

void* tcp_handle_conn (void * client_socket)
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
            connect = "Connected to " + localSites[x];
            char cstr[connect.size() + 1];
            connect.copy(cstr, connect.size() + 1);
            cstr[connect.size()] = '\0';

            send(sock , cstr , strlen(cstr), 0);
        }
        else{
            int sockLocal;
            struct sockaddr_in local;
            char messageLocal[DEFAULT_BUFLEN];
            char local_reply[DEFAULT_BUFLEN];

            sockLocal = socket(AF_INET , SOCK_STREAM , 0);
            if (sockLocal == -1)
            {
                printf("Could not create socket");
                exit(0);
            
            }

            local.sin_addr.s_addr = inet_addr("127.0.0.1");
            local.sin_family = AF_INET;
            local.sin_port = htons(LOCAL_PORT);

            if (connect(sockLocal , (struct sockaddr *)&local , sizeof(local)) < 0)
            {
                perror("connect failed. Error");
                exit(0);
            }
            puts("Connected to Local server\n");

            char cstr[x.size() + 1];
            x.copy(cstr, x.size() + 1);
            cstr[x.size()] = '\0';

            if( send(sockLocal , cstr , strlen(cstr), 0) < 0)
            {
                puts("Send failed");
                exit(0);
            }

            memset(local_reply,'\0', DEFAULT_BUFLEN);

            if( recv(sockLocal , local_reply , DEFAULT_BUFLEN , 0)<0)
            {
                puts("recv failed");
                break;
            }

            if(local_reply[0] == 'x'){
                char *message = "Couldn't find IP address.";
                send(sock , message , strlen(message), 0);
            }
            else
            {
                string ip;
                string connect;
                for(int i = 0; i < sizeof(local_reply); i++){

                    if(local_reply[i] == '\0'){
                        break;
                    }
                    connect = connect + local_reply[i];
                    ip = ip + local_reply[i];
                }
                connect = "Connected to " + connect;
                char cstr[connect.size() + 1];
                connect.copy(cstr, connect.size() + 1);
                cstr[connect.size()] = '\0';
                send(sock , cstr , strlen(cstr), 0);

                updateMap(x, ip);

            }

            close(sockLocal);

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

string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        if(a[i] == '\0' || a[i] == '\n'){
            break;
        }
        s = s + a[i];
    }
    return s;
}

void updateMap(string name, string ip){
    localSites.insert({name, ip});
    cout << "DATA BASE UPDATED\n"<< endl;

}
