#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <vector>
#include <sstream>

#include "headers.h"

#define BUFSIZE 128

using namespace std;

int main(int argc, char *argv[]) {

    (void) argc; // unused
    (void) argv; // unused
    char asip[32] = "localhost";
    char port[6] = "58070";
    
    User user("", "", "", "", "", "", "");
    
    vector<string> message;

    char buffer[BUFSIZE]; // buffer to store data

    struct sigaction stop; // CTRL_C signal handler
    
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT

    while(true) {

        check((read(0, buffer, BUFSIZE)) == -1); // read from stdin
        buffer[strlen(buffer)-1] = '\0'; // add null terminator to buffer

        message = string_analysis(buffer, user);

        if(message[0] == "udp") {
            udp_message(asip, port, vector_to_string(message));

        } else if(message[0] == "tcp") {
            tcp_message(asip, port, vector_to_string(message));

        } else if (message[0] == "exit") {
            if (user.is_logged_in()) {
                cout << "You are still logged in. Please logout before exiting." << endl;
            } else {
                cout << "Exiting..." << endl;
                break;
            }

        } else {
            cout << "Invalid command" << endl;
        }

        memset(buffer, '\0', BUFSIZE); // initialize buffer to '\0'

    }
    
    exit(0);

}

int tcp_message(char *asip, char *port, string message) {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    char buffer[BUFSIZE]; //pointer to buffer and buffer to store data

    strcpy(buffer, message.c_str());

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1); //TCP socket
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket

    check((getaddrinfo(asip, port, &hints, &res)) != 0); //get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); //connect to server

    //TODO : CHECK IF SERVER IS UP

    check((write(fd, buffer, message.size())) != (ssize_t)message.size()); // write to socket and store number of bytes written

    memset(buffer, '\0', BUFSIZE); // initialize buffer to 0s

    // QUESTION: DOES THE SERVER REPLY IN 1 GO OR IN CHUNKS?
    check((read(fd, buffer, BUFSIZE)) == -1); // read from socket TODO: CHANGE TO FORMATS
    check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false);

    write(1, "Server TCP: ", 12); 
    write(1, buffer, strlen(buffer)); // print to stdout content from socket
    write(1, "\n", 1); // print newline to stdout

    freeaddrinfo(res); // free address info
    close(fd);

    return 1;
}

int udp_message(char *asip, char *port, string message) {

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

    memset(buffer, '\0', BUFSIZE); // initialize buffer to 0s

    check(read(fd, buffer, BUFSIZE) == -1); // read from socket TODO: CHANGE TO FORMATS
    check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false);

    cout << "Server UDP replied: " << buffer << endl;

    freeaddrinfo(res); // free address info
    close(fd);

    return 1;
}

vector<string> string_analysis(char* str, User &user) {
    
    vector<string> message; // vector to store the final message
    
    istringstream iss(str); // istringstream to read each token
    string token; // string to store each token

    while (iss >> token) {
        message.push_back(token); 
    }

    if (message.empty()) {
        return message;

    } else if (message.size() == 1) {

        if (message[0] == "logout") { // LOGOUT
            message = {"udp", "LOU", user.getUID(), user.getPassword()};
            user.erase();

        } else if (message[0] == "unregister") { // UNREGISTER
            message = {"udp", "UNR", user.getUID(), user.getPassword()};

        } else if (message[0] == "exit") { // EXIT
            message = {"exit"};

        } else if (message[0] == "myauctions" || message[0] == "ma") { // MYAUCTIONS
            message = {"udp", "LMA", user.getUID()};

        } else if (message[0] == "mybids" || message[0] == "mb") { // MYBIDS
            message = {"udp", "LMB", user.getUID()};

        } else if (message[0] == "list" || message[0] == "l") { // LIST
            message = {"udp", "LST"};
        }

    } else if (message.size() == 2 && possible_AID(message[1])) { 

        if (message[0] == "show_asset" || message[0] == "sa") { // SHOW_ASSET
            message = {"tcp", "SAS", message[1]};

        } else if (message[0] == "close") { // CLOSE
            message = {"tcp", "CLS", user.getUID(), user.getPassword(), message[1]};

        } else if (message[0] == "show_record" || message[0] == "sr") { // SHOW_RECORD
            message = {"udp", "SRC", message[1]};
        }

    } else if (message.size() >= 3) {

        if (message[0] == "login" && possible_UID(message[1]) && possible_password(message[2])) { // LOGIN
            user.login(message[1], message[2]);
            message = {"udp", "LIN", user.getUID(), user.getPassword()};

        } else if (message[0] == "open" && possible_auction_name(message[1]) && possible_fname(message[2]) && //OPEN
            message.size() == 5 && possible_start_value(message[3]) && possible_time_active(message[4])) { 
            message = {"tcp", "OPA", user.getUID(), user.getPassword(), message[1], message[3], message[4]};

        } else if ((message[0] == "bid" || message[0] == "b") && possible_AID(message[1]) && possible_start_value(message[2])) { // BID
            message = {"tcp", "BID", user.getUID(), user.getPassword(), message[1], message[2]};
        }
    }
    
    return message;
}
