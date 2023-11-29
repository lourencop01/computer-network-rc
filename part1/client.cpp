#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <signal.h>

#define PORT "58070"
#define BUFSIZE 128

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

int tcp_client() {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    ssize_t nwritten, nread; //number of bytes written and read
    char *ptr, buffer[BUFSIZE]; //pointer to buffer and buffer to store data

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1); //TCP socket
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket

    check((getaddrinfo("localhost", PORT, &hints, &res)) != 0); //get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); //connect to server

    //TODO : CHECK IF SERVER IS UP

    while(true) {
        
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

    }

    freeaddrinfo(res); // free address info
    close(fd);

}

int udp_client() {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    ssize_t nwritten, nread; //number of bytes written and read
    char *ptr, buffer[BUFSIZE]; //pointer to buffer and buffer to store data
        
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1); //UDP socket

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    check(getaddrinfo("localhost", PORT, &hints, &res) != 0); // get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); // connect to server

    while(true) {

        check((nread = read(0, buffer, BUFSIZE)) == -1); // read from stdin
        ptr = buffer; // pointer to buffer
        ptr[nread-1] = '\0'; // add null terminator to buffer

        check((nwritten = write(fd, ptr, nread)) <= 0); // write to socket and store number of bytes written
        check(nread != nwritten); // check if wrote all bytes
        ptr += nwritten; // move pointer nwritten bytes

        check((nread = read(fd, ptr, nwritten)) == -1); // read from socket TODO: CHANGE TO FORMATS
        check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false);
        check(nread != nwritten); // check if read all bytes

        write(1, "Server UDP: ", 12); 
        write(1, ptr, nread); // print to stdout content from socket
        write(1, "\n", 1); // print newline to stdout

    }

}

int main(int argc, char *argv[]) {

    (void) argc; // unused

    struct sigaction stop; // CTRL_C signal handler

    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT

    if(atoi(argv[1]) == 1) {
        cout << "Client joined TCP" << endl;
        tcp_client();
    } else if(atoi(argv[1]) == 2) {
        cout << "Client joined UDP" << endl;
        udp_client();
    }

    exit(0);

}