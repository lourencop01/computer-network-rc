#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sstream>

#define BUFSIZE 128

using namespace std;

extern int errno;

class User{
    public:
        
        string UID;
        string password;
        
        User(string UID, string password) {
            this->UID = UID;
            this->password = password;
        }

        string getUID() {
            return this->UID;
        }

        string getPassword() {
            return this->password;
        }

        void setUID(string UID) {
            this->UID = UID;
        }

        void setPassword(string password) {
            this->password = password;
        }
};

User user("", "");

/*
* Checks if condition is true. In that case, exits with error code 1.
*/
void check(bool condition) { if(condition) exit(1); }

void safe_stop(int signal) {
    (void) signal;
    cout << "\nExiting..." << endl;
    exit(0);
}

bool possible_UID(string UID) {
    if(UID.size() != 6) {
        return false;
    }
    for(size_t i = 0; i < UID.size(); i++) {
        if(!isdigit(UID[i])) {
            return false;
        }
    }
    return true;
}

bool possible_password(string password) {
    if(password.size() != 8) {
        return false;
    }
    for(size_t i = 0; i < password.size(); i++) {
        if(!isdigit(password[i]) && !isalpha(password[i])) {
            return false;
        }
    }
    return true;
}

bool possible_AID(string AID) {
    if(AID.size() != 3) {
        return false;
    }
    for(size_t i = 0; i < AID.size(); i++) {
        if(!isdigit(AID[i])) {
            return false;
        }
    }
    return true;
}

string create_string(vector<string> message) {

    string str = "";

    message.erase(message.begin());

    if(message.back() == "") {
        message.pop_back();
    }

    for(size_t i = 0; i < message.size(); i++) {
        str += message[i];
        str += " ";
    }

    str.pop_back();

    return str;
}

vector<string> string_analysis(char* str) {
    
    vector<string> message; // vector to store the final message
    
    istringstream iss(str); // istringstream to read each token
    string token; // string to store each token

    while (iss >> token) { 
        message.push_back(token); 
    }

    if (message.empty()) {
        // TODO
        return message;
    } else if (message.size() == 1) {
        const string& firstToken = message[0];
        if (firstToken == "logout") {
            message = {"udp", "LOU", user.getUID(), user.getPassword()};
        } else if (firstToken == "unregister") {
            message = {"udp", "UNR", user.getUID(), user.getPassword()};
        } else if (firstToken == "exit") {
            // TODO
        } else if (firstToken == "myauctions" || firstToken == "ma") {
            message = {"udp", "LMA", user.getUID()};
        } else if (firstToken == "mybids" || firstToken == "mb") {
            message = {"udp", "LMB", user.getUID()};
        } else if (firstToken == "list" || firstToken == "l") {
            message = {"udp", "LST"};
        }
        // TODO
    } else if (message.size() == 2 && possible_AID(message[1])) {
        const string& command = message[0];
        if (command == "show_asset" || command == "sa") {
            // TODO
        } else if (command == "close") {
            // TODO
        } else if (command == "show_record" || command == "sr") {
            message = {"udp", "SRC", message[1]};
        }
        // TODO
    } else if (message.size() >= 3) {
        const string& command = message[0];
        if (command == "login" && possible_UID(message[1]) && possible_password(message[2])) {
            user.setUID(message[1]);
            user.setPassword(message[2]);
            message = {"udp", "LIN", user.getUID(), user.getPassword()};
        } else if (command == "open" && message[1] == "name" && message[2] == "asset_fname" &&
            message.size() >= 6 && message[3] == "start_value" && message[5] == "timeactive") {
            // TODO
        } else if ((command == "bid" || command == "b") && message[1] == "AID" && message[2] == "value") {
            // TODO
        }
        // TODO
    }
    // TODO
    return message;
}

int tcp_client(char *asip, char *port, string message) {

    (void) message; // unused

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    ssize_t nwritten, nread; //number of bytes written and read
    char *ptr, buffer[BUFSIZE]; //pointer to buffer and buffer to store data

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1); //TCP socket
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket

    check((getaddrinfo(asip, port, &hints, &res)) != 0); //get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); //connect to server

    //TODO : CHECK IF SERVER IS UP
        
    check((nread = read(0, buffer, BUFSIZE)) == -1); // read from stdin
    ptr = buffer; // pointer to buffer
    ptr[nread-1] = '\0'; // add null terminator to buffer

    check((nwritten = write(fd, ptr, nread)) <= 0); // write to socket and store number of bytes written
    check(nread != nwritten); // check if wrote all bytes
    ptr += nwritten; // move pointer nwritten bytes

    // QUESTION: DOES THE SERVER REPLY IN 1 GO OR IN CHUNKS?
    check((nread = read(fd, ptr, nwritten)) == -1); // read from socket TODO: CHANGE TO FORMATS
    check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false);
    check(nread != nwritten); // check if read all bytes

    write(1, "Server TCP: ", 12); 
    write(1, ptr, nread); // print to stdout content from socket
    write(1, "\n", 1); // print newline to stdout

    freeaddrinfo(res); // free address info
    close(fd);

    return 1;
}

int udp_client(char *asip, char *port, string message) {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    char buffer[BUFSIZE]; //pointer to buffer and buffer to store data

    strcpy(buffer, message.c_str());
        
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1); //UDP socket

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    check(getaddrinfo(asip, port, &hints, &res) != 0); // get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); // connect to server

    check(write(fd, buffer, message.size()) != (ssize_t) message.size()); // write to socket

    memset(buffer, 0, BUFSIZE); // initialize buffer to 0s

    check(read(fd, buffer, BUFSIZE) == -1); // read from socket TODO: CHANGE TO FORMATS
    check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false);

    write(1, "Server UDP: ", 12);
    write(1, buffer, strlen(buffer)); // print to stdout content from socket
    write(1, "\n", 1); // print newline to stdout

    freeaddrinfo(res); // free address info
    close(fd);

    return 1;
}

int main(int argc, char *argv[]) {

    (void) argc; // unused
    (void) argv; // unused
    char asip[32] = "localhost";
    char port[6] = "58070";
    
    vector<string> message;

    char buffer[BUFSIZE]; // pointer to buffer and buffer to store data

    struct sigaction stop; // CTRL_C signal handler
    
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT

    while(true) {
        
        check((read(0, buffer, BUFSIZE)) == -1); // read from stdin
        buffer[strlen(buffer)-1] = '\0'; // add null terminator to buffer

        message = string_analysis(buffer);

        if(message[0] == "udp") {
            udp_client(asip, port, create_string(message));
        } else if(message[0] == "tcp") {
            tcp_client(asip, port, create_string(message));
        } else {
            cout << "Invalid command" << endl;
        }

    }
    
    exit(0);

}