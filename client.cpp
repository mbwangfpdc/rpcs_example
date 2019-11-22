#include <iostream>
#include <stdlib.h>		// atoi()
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <unistd.h>		// close()
#include <netinet/in.h>	// struct sockaddr_in
#include <netdb.h>      // gethostbyname(), struct hostent
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

#include "rpcs.h"
using namespace std;
int port;

response unmarshall_response(const vector<char>& buf) {
    response resp;
    resp.value.resize(buf.size() - 1);
    copy(buf.begin(), prev(buf.end()), resp.value.begin());
    resp.result = buf.back();
    return resp;
}

vector<char> marshall_request(const request& req) {
    vector<char> marshalled;
    marshalled.resize(2 * sizeof(size_t) + req.key.size() + req.value.size() + 1);
    char* head = marshalled.data();

    *head = req.key.size();
    head += sizeof(size_t);
    memcpy(head, req.key.data(), req.key.size());
    head += req.key.size();

    *head = req.value.size();
    head += sizeof(size_t);
    memcpy(head, req.value.data(), req.value.size());
    head += req.value.size();

    *head = req.op;

    return marshalled;
}

response make_request(const request& req) {
    vector<char> marshalled = marshall_request(req);
    int sockfd = connect_as_client(port);
    send_data(sockfd, marshalled);
    vector<char> response_buf = receive_data(sockfd);
    close(sockfd);
    return unmarshall_response(response_buf);
}

string get(const string& key) {
    return make_request(request{key, "", 'g'}).value;
}
void put(const string& key, const string& val) {
    make_request(request{key, val, 'p'});
}
void append(const string& key, const string& val) {
    make_request(request{key, val, 'a'});
}

// Note: Thank you @bgreeves for pretty much all of this code
int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: <EXE> [port]" << endl;
        exit(1);
    } else {
        string s(argv[1]);
        port = stoi(s);
        cout << "Port is " << port << endl;
        cout << "USAGE: [g|p|a] <key> [value]" << endl;
        cout << "g: get the value of a key" << endl;
        cout << "p: put a value to a key" << endl;
        cout << "a: append a value to a key" << endl;

    }

    string s;
    while (getline(cin, s)) {
        istringstream iss(s);
        string op;
        string key;
        string val;
        iss >> op >> key >> val;
        if (op == "g" || op == "get") {
            cout << "Value of key " << key << " is " << get(key) << endl;
        } else if (op == "p" || op == "put") {
            cout << "Putting value " << val << " to key " << key << endl;
            put(key, val);
        } else if (op == "a" || op == "append") {
            cout << "Appending value " << val << " to key " << key << endl;
            append(key, val);
        } else {
            cout << "unknown operation type" << endl;
        }
    }
}


