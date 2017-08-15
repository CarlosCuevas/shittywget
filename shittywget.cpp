#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;

#define MAXDATASIZE 3000

void *get_in_addr (struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[]) {
    int status, sockfd, numbytes, nDataLength;
    struct addrinfo hints, *servinfo, *p;
    char ipstr[INET6_ADDRSTRLEN];
    char buffer[MAXDATASIZE];
    ofstream outputFile;
    string get = "GET / HTTP/1.1\r\nHost: " + string(argv[1]) + "\r\nConnection: close\r\n\r\n";
    
    if (argc != 2) {
        cerr << "usage: " << argv[0] << " <hostname>" << endl;
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(argv[1], "80", &hints, &servinfo);
    if (status != 0) {
        cerr << argv[0] << " " << gai_strerror(status) << endl;
        return 2;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            cerr << "error opening socket";
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            cerr << "error connecting to socket";
            continue;
        }

        break;
    }

    if (p == NULL) {
        cerr << "failed to connect" << endl;
        return 2;
    }

    send(sockfd, get.c_str(), get.length(), 0);
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), ipstr, sizeof ipstr);

    freeaddrinfo(servinfo);

    outputFile.open("output.html");
    
    while ((nDataLength = recv(sockfd, buffer, MAXDATASIZE-1, 0)) > 0) {
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
            cout << buffer[i];
            outputFile << buffer[i];
            i += 1;
        }
    }

    outputFile.close();
    close(sockfd);

    return 0;
}