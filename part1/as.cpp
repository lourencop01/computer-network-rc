#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define PORT "58001"
#define BUFSIZE 128
#define MAXQUEUE 5

extern int errno;

using namespace std;

/*
* Checks if condition is true. In that case, exits with error code 1.
*/
void check(bool condition) { if(condition) exit(1); }

void safe_stop(int signal) {
    (void) signal;
    cout << "\nExiting..." << endl;
    exit(0);
}

int tcp_server() {
    struct addrinfo hints, *res;
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

    struct sigaction stop; // CTRL_C signal handler
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT
    
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
                ptr = buffer;
                while(n > 0) { // while there is data to be written
                    check((nw = write(newfd, ptr, n)) <= 0); // write to socket
                    n -= nw; // update number of bytes to be written
                    ptr += nw; // update pointer to buffer
                }
                cout << buffer << endl;
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

int udp_server() {

    return 0;
}

int main() {

    tcp_server();

    return 0;

}