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

#define BUFSIZE 128

using namespace std;

extern int errno;

string password = "\0";
string UID = "\0";
string AID = "\0";

/*
* Checks if condition is true. In that case, exits with error code 1.
*/
void check(bool condition) { if(condition) exit(1); }

void safe_stop(int signal) {
    (void) signal;
    cout << "\nExiting..." << endl;
    exit(0);
}

string string_analysis(char *str) {

    char *token, *token2, *token3;

    token = strtok(str, " ");
    token2 = strtok(NULL, " ");
    token3 = strtok(NULL, " ");

    if (token == NULL) {
        return "";
    } else if(token2 == NULL) {
        if(!strcmp(token, "logout") && token2 == NULL) {
            return "LOU " + UID + " " + password;
        } else if(!strcmp(token, "unregister")) {
            return "UNR " + UID + " " + password;
        } else if(!strcmp(token, "exit")) {
            return "";
        } else if(!strcmp(token, "myauctions") || !strcmp(token, "ma")) {
            return "LMA " + UID;
        } else if(!strcmp(token, "mybids") || !strcmp(token, "mb")) {
            return "LMB " + UID;
        } else if(!strcmp(token, "list") || !strcmp(token, "l")) {
            return "LST";
        }
        return "";
    } else if (token3 == NULL && !strcmp(token2, "AID")) {
        strcpy(AID, token2);
        if(!strcmp(token, "show_asset") || !strcmp(token, "sa")) {
            return "";
        } else if(!strcmp(token, "close")) {
            return "";
        } else if(!strcmp(token, "show_record") || !strcmp(token, "sr")) {
            return "SRC " + AID;
        }
        return "";
    } else if (token3 != NULL) {
        if(!strcmp(token, "login") && !strcmp(token2, "UID") && !strcmp(token3, "password")) {
            strcpy(UID, token2); strcpy(password, token3);
            return "LIN " + UID + " " + password;
        } else if(!strcmp(token, "open") && !strcmp(token2, "name") && !strcmp(token3, "asset_fname") 
                    && strcmp(strtok(NULL, " "), "start_value") && strcmp(strtok(NULL, " "), "timeactive")) {
            return "";
        } else if((!strcmp(token, "bid") || !strcmp(token, "b")) && !strcmp(token2, "AID") && !strcmp(token3, "value")) {
            return "";
        }
        return "";
    }
    return "";
}

int tcp_client(char *asip, char *port) {

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

}

int udp_client(char *asip, char *port) {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    ssize_t nwritten, nread; //number of bytes written and read
    char *ptr, buffer[BUFSIZE]; //pointer to buffer and buffer to store data
        
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1); //UDP socket

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    check(getaddrinfo(asip, port, &hints, &res) != 0); // get address info

    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1); // connect to server

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

int main(int argc, char *argv[]) {

    (void) argc; // unused
    (void) argv; // unused
    char asip[32] = "localhost";
    char port[6] = "58070";

    ssize_t nread; // number of bytes read
    char *ptr, buffer[BUFSIZE]; // pointer to buffer and buffer to store data

    struct sigaction stop; // CTRL_C signal handler
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1); // set signal handler to safe_stop for SIGINT

    while(true) {
        
        check((nread = read(0, buffer, BUFSIZE)) == -1); // read from stdin
        ptr = buffer; // pointer to buffer
        ptr[nread-1] = '\0'; // add null terminator to buffer

        strcpy(buffer, string_analysis(ptr));

        cout << buffer << endl;

    }
    
    exit(0);

}