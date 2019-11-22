#include <iostream>
#include <stdlib.h>		// atoi()
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <unistd.h>		// close()
#include <netinet/in.h>	// struct sockaddr_in
#include <netdb.h>      // gethostbyname(), struct hostent
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "rpcs.h"
using namespace std;
const char NOT_FOUND = '?';
const char OK = '+';
const char BAD_OP = 'X';

vector<char> receive_data(int connFD) {
    size_t message_size;
    if (recv(connFD, &message_size, sizeof(message_size), MSG_WAITALL) < 0) {
        cout << "recv failed" << endl;
        exit(1);
    }
    vector<char> result(message_size);
    if (recv(connFD, result.data(), result.size(), MSG_WAITALL) < 0) {
        cout << "recv failed" << endl;
        exit(1);
    }
    return result;
}

void send_data(int connFD, const vector<char>& to_send) {
    size_t data_size = to_send.size();
    vector<char> buf(sizeof(data_size));
    memcpy(buf.data(), &data_size, sizeof(data_size));

    buf.resize(buf.size() + to_send.size());
    copy(to_send.begin(), to_send.end(), buf.begin() + sizeof(data_size));
    send(connFD, buf.data(), buf.size(), 0);
}

int connect_as_client(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct hostent* he = gethostbyname("localhost");
    if(!he) {
        cout << "ERROR no such host" << endl;
        exit(-1);
    }
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    if(connect(sockfd, (sockaddr*) &addr, sizeof(addr)) < 0) {
        cout << "ERROR connecting to server" << endl;
        exit(-1);
    }

    return sockfd;
}

