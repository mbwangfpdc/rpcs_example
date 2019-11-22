#include <iostream>
#include <stdlib.h>		// atoi()
#include <sys/socket.h>	// socket(), bind(), listen(), accept()
#include <unistd.h>		// close()
#include <netinet/in.h>	// struct sockaddr_in
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "rpcs.h"
using namespace std;

unordered_map<string, string> state;

response handle_list(const request& req) {
    response resp;
    resp.value = "\n";
    for (const auto& statum : state) {
        resp.value += (statum.first + ":" + statum.second + "\n");
    }
    return resp;
}

response handle_get(const request& req) {
    return state.count(req.key) ? response{state[req.key],OK} : response{"",NOT_FOUND};
}

response handle_put(const request& req) {
    state[req.key] = req.value;
    return response{};
}

response handle_append(const request& req) {
    state[req.key] += req.value;
    return response{};
}

response handle_req(const request& req) {
    if (req.op == 'g') {
        return handle_get(req);
    } else if (req.op == 'p') {
        return handle_put(req);
    } else if (req.op == 'a') {
        return handle_append(req);
    } else if (req.op == 'l') {
        return handle_list(req);
    } else {
        return response{"",BAD_OP};
    }
}



request unmarshall_request(const vector<char>& buf) {
    request req;
    const char* head = buf.data();

    size_t key_size = *(size_t*)head;
    head += sizeof(size_t);
    req.key = string(head, key_size);
    head += key_size;

    size_t val_size = *(size_t*)head;
    head += sizeof(size_t);
    req.value = string(head, val_size);
    head += val_size;

    req.op = *head;

    return req;
}

vector<char> marshall_response(const response& resp) {
    vector<char> response;
    response.resize(resp.value.size() + 1);
    copy(resp.value.begin(), resp.value.end(), response.begin());
    response.back() = resp.result;

    return response;
}

int main(int argc, char* argv[]) {
    int port = 0;

    if (argc > 2) {
        cout << "Usage: <EXE> [port_number]" << endl;
        exit(1);
    } else if (argc == 2){
        string s(argv[1]);
        port = stoi(s);
        cout << "Port is " << port << endl;
    }

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int yesval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval));

	struct sockaddr_in addr;
    socklen_t addrSize = sizeof(addr);
    memset(&addr, 0, addrSize);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr*) &addr, addrSize) < 0) cout << "bind: " << errno << endl;
	if (listen(sockfd, 5) < 0) cout << "listen: " << errno << endl;

	while (true) {
        int connFD = accept(sockfd, (struct sockaddr*) &addr, &addrSize);
        vector<char> req_data = receive_data(connFD);
        request req = unmarshall_request(req_data);

        response resp = handle_req(req);

        vector<char> resp_data = marshall_response(resp);
        send_data(connFD, resp_data);
        close(connFD);
	}
}
