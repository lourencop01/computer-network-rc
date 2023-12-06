#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <string>
#include <vector>
#include <sstream>


#include "headers.h"

#define PORT "58070"
#define BUFSIZE 128
#define MAXQUEUE 5

using namespace std;

int main() {

    pid_t pid;

    Users users;
    load_users(users); // load users from USERS folder
    users.print_all_users();

    struct sigaction stop; // CTRL_C signal handler
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT

    check((pid = fork()) == -1); // create child process

    if(pid == 0) { // child process
        tcp_server(users);
    } else { // parent process
        udp_server(users);
    }

    return 0;

}

int tcp_server(Users &users) {
    (void) users; // unused

    struct addrinfo hints, *res; // hints: info we want, res: info we get
    int fd, newfd, ret; // file descriptors
    ssize_t n, nw; // number of bytes read or written
    struct sockaddr_in addr; // address of the server
    socklen_t addrlen; // size of the address
    char *ptr, buffer[BUFSIZE]; // buffer to store data
    
    pid_t pid; // process id

    struct sigaction act; // signal handler
    memset(&act, 0, sizeof(act)); // initialize signal handler to 0s
    act.sa_handler = SIG_IGN; // set handler to ignore the signal
    check(sigaction(SIGCHLD, &act, NULL) == -1); // set signal handler to ignore SIGCHLD

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1); // create socket with TCP protocol

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // use local IP address

    check(getaddrinfo(NULL, PORT, &hints, &res) != 0); // get address info
    check(::bind(fd, res->ai_addr, res->ai_addrlen) == -1); // bind socket to address
    check(listen(fd, 5) == -1); // listen for connections

    freeaddrinfo(res); // free address info

    while(true) {
        addrlen = sizeof(addr); // set address length
        do { 
            newfd = accept(fd, (struct sockaddr*)&addr, &addrlen); // accept connection 
        } while(newfd == -1 && errno == EINTR); // if interrupted by signal, try again
        check(newfd == -1); // check if accept failed
        check((pid = fork()) == -1); // create child process
        if(pid == 0){ // child process
            close(fd); // close listening socket TODO: WHY??
            while((n = read(newfd, buffer, BUFSIZE)) > 0) { // read from socket
                cout << "TCP received: "<< buffer << endl;
                ptr = buffer;
                while(n > 0) { // while there is data to be written
                    check((nw = write(newfd, ptr, n)) <= 0); // write to socket
                    n -= nw; // update number of bytes to be written
                    ptr += nw; // update pointer to buffer
                }
            }
            close(newfd); // close socket
            exit(0); // exit child process
        }
        do {
            ret = close(newfd); // close socket
        } while(ret == -1 && errno == EINTR); // if interrupted by signal, try again
    }

    close(fd); // close listening socket
    return 1;

}

int udp_server(Users &users) {
    
    struct addrinfo hints, *res;
    int fd; // file descriptor
    ssize_t n; // number of bytes read
    struct sockaddr_in addr; // address of the server
    socklen_t addrlen; // size of the address
    char *ptr, buffer[BUFSIZE]; // buffer to store data

    ptr = buffer;
    
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ); // create socket with UDP protocol

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE; // use local IP address

    check(getaddrinfo(NULL, PORT, &hints, &res) != 0); // get address info

    check(::bind(fd, res->ai_addr, res->ai_addrlen) == -1); // bind socket to address

    while(true) {

        memset(buffer, '\0', BUFSIZE); // initialize buffer to '\0'
        ptr = buffer; // reset pointer to buffer

        addrlen = sizeof(addr); // set address length

        check((n = recvfrom(fd, buffer, BUFSIZE, 0, (struct sockaddr*)&addr, &addrlen)) == -1); // receive data from socket
        ptr += n; // update pointer to end of buffer
        cout << "UDP received: " << buffer << endl;

        strcpy(ptr, vector_analysis(string_to_vector(buffer), users).c_str()); // convert buffer to vector

        cout << "UDP sent: " << ptr << endl;

        check(sendto(fd, ptr, BUFSIZE, 0, (struct sockaddr*)&addr, addrlen) == -1); // send data to socket

    }

    freeaddrinfo(res); // free address info
    close(fd); // close socket

    return 0;
}

string vector_analysis(vector<string> message, Users &users) {

        if(message[0] == "LIN") { // Todo: What happens if user already logged in
            return "RLI " + login(message[1], message[2], users);
        } else if(message[0] == "LOU") { // User requests for logout
            return "RLO "+ logout(message[1], message[2], users);
        } else if(message[0] == "UNR") {
            return "RUR " + unregister(message[1], message[2], users);
        } else if(message[0] == "mb" || "mybids") {
            return "RMB " + mybids(message[1], message[2], users);
        } else if(message[0] == "bid") {
            //bid(message[1], message[2], message[3], message[4]);
        } else if(message[0] == "search") {
            //search(message[1], message[2], message[3]);
        } else if(message[0] == "mybids") {
            //mybids(message[1], message[2]);
        } else if(message[0] == "auctions") {
            //auctions(message[1], message[2]);
        } else if(message[0] == "exit") {
            //exit(0);
        } else {
            return "ERR";
        }
    return "Weird";
}

string login(string UID, string password, Users &users) {

    User *user = users.get_user(UID);

    if (!possible_UID(UID) || !possible_password(password)) {
        return "ERR";

    } else if (user == nullptr) { // new user

        string uid_pathname = "USERS/" + UID;
        string pass_pathname = uid_pathname + "/" + UID + "_pass.txt";
        string login_pathname = uid_pathname + "/" + UID + "_login.txt";
        string hosted_pathname = uid_pathname + "/" + "HOSTED";
        string bids_pathname = uid_pathname + "/" + "BIDDED";

        users.add_user(User(UID, password, uid_pathname, pass_pathname, login_pathname, hosted_pathname, bids_pathname));

        user = users.get_user(UID);

        mkdir((user->get_uid_pathname()).c_str(), 0700);
        mkdir((user->get_hosted_pathname()).c_str(), 0700);
        mkdir((user->get_bids_pathname()).c_str(), 0700);

        create_pass_file(*user);
        create_login_file(*user);

        return "REG";

    } else if (user->is_unregistered()) { // user existed but unregistered
        
        // TODO : add hosted and bidded auctions to user vectors
        user->re_register(password);

        create_pass_file(*user);
        create_login_file(*user);

        return "OK";

    } else if (user->is_logged_out()) { // user exists, is registered, and logged out

        if (user->getPassword() != password) {
            return "NOK";
        }

        user->set_logged_in();

        create_login_file(*user);

        return "OK";

    } else { // user already logged in
        // TODO what to do in this case?
        cout << "User already logged in" << endl;
        return "LOG";
    }

    return "WEIRD";

}

string logout(string UID, string password, Users &users) { // logged in and reg, logged out and reg, logged out and unr

    cout << UID << endl;
    User *user = users.get_user(UID);

    if (!possible_UID(UID) || !possible_password(password)) {
        return "ERR";

    } else if (user == nullptr) { // user never existed (never registered before)
        return "UNR";

    } else if (user->is_logged_in() && user->is_unregistered()) {
        return "IMPOSSIBLE";

    } else if ((user->is_logged_out() && !(user->is_unregistered()))) { // user exists, is registered, and logged out
        return "NOK";

    } else { // user exists, is registered, and logged in
        if (user->getPassword() != password) {
            return "NOK";
        }
        user->set_logged_out();
        // Delete login file from user
        
        delete_login_file(*user);

        return "OK";
    }

    return "WEIRD";

}

string unregister(string UID, string password, Users &users) { // logged in and reg, logged out and reg, logged out and unr

    User *user = users.get_user(UID);

    if (!possible_UID(UID) || !possible_password(password)) {
        return "ERR";

    } else if (user == nullptr) { // user never existed (never registered before)
        return "UNR";

    } else if (user->is_logged_in() && user->is_unregistered()) {
        return "IMPOSSIBLE";

    } else if ((user->is_logged_out() && !(user->is_unregistered()))) { // user exists, is registered, and logged out
        return "NOK";

    } else { // user exists, is registered, and logged in
        if (user->getPassword() != password) {
            return "NOK";
        }
        user->set_unregistered();
        user->set_logged_out();
        // Delete login file from user
        delete_login_file(*user);
        delete_pass_file(*user);

        return "OK";
    }

    return "WEIRD";

}

string mybids(string UID, string password, Users &users) {

}

