#include <string>
#include <vector>
using namespace std;

struct request {
    string key = "";
    string value = "";
    char op = '\0';
};

extern const char NOT_FOUND;
extern const char OK;
extern const char BAD_OP;
struct response {
    string value = "";
    char result = OK;
};

vector<char> receive_data(int connfd);
void send_data(int connfd, const vector<char>& response_buf);
int connect_as_client(int port);
