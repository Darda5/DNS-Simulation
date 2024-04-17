#include <iostream>
#include <ostream>
#include <stdio.h>    
#include <string.h>    
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <fcntl.h>     
#include <unistd.h>    
#include <regex>
#include <fstream>
#include <list>


#define DEFAULT_BUFLEN 50
#define DEFAULT_PORT   27015
using namespace std;

std::ifstream file("test");

bool isValidURL(string url);
string convertToString(char* a, int size);

int main(int argc , char *argv[])
{
    bool test = true;
    int sock;
    struct sockaddr_in server;
    char message[DEFAULT_BUFLEN];
    char server_replay[DEFAULT_BUFLEN];

    list<string> siteList;

    if(test == true){
        if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            siteList.push_back(line);
        }
        file.close();
        }
    }

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
        return 1;
	
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    int word = 0;
while(1)
{


    if(word == 8){
        break;
    }

    string input, site;
    string https = "https://www.";

    list<string>::iterator it = siteList.begin();
    if(test == true){
        advance(it, word);
        std::string line;
        file.close();
        input = *it;
        site = https + input;

    }

    if(test == false){

        printf("Enter site you wish to connect to: ");
        getline (cin,input);
        site = https + input;
    }

    char cstr[input.size() + 1];
    input.copy(cstr, input.size() + 1);
    cstr[input.size()] = '\0';
    
    if(isValidURL(site)){
        //Send some data
        cout << "Connecting to " + site << endl;
        if( send(sock , cstr , strlen(cstr), 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        memset(server_replay,'\0', DEFAULT_BUFLEN);
        memset(message,'\0', DEFAULT_BUFLEN);

        if( recv(sock , server_replay , DEFAULT_BUFLEN , 0)<0)
        {
        puts("recv failed");
        break;
        }

            puts(server_replay);
    }

    if(test == true){
        word ++;
        sleep(2);
    }

}


    close(sock);

    return 0;
}

bool isValidURL(string url)
{

	// Regex to check valid URL
	const regex pattern("((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

	// If the URL
	// is empty return false
	if (url.empty())
	{
		return false;
	}

	// Return true if the URL
	// matched the Regex
	if (regex_match(url, pattern))
	{
		return true;
	}
	else
	{
		return false;
	}
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

