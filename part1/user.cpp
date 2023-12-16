#include <stdlib.h>
#include <stdio.h>
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
#include <fstream>
#include <iomanip>

#include "headers.h"

#define PORT "58096"
#define BUFSIZE 128
#define MAXDATASIZE 1024

using namespace std;

int main(int argc, char *argv[]) {

    char asip[32] = "localhost";
    char port[6] = "58096";

    if (argc == 2) {
        cout << "Invalid number of arguments" << endl;
        exit(1);
    } else if (argc == 3) {
        if (strcmp(argv[1], "-n") == 0) {
            memset(asip, '\0', 32);
            strcpy(asip, argv[2]);
        } else if (strcmp(argv[1], "-p") == 0) {
            memset(port, '\0', 6);
            strcpy(port, argv[2]);
        } else {
            cout << "Invalid arguments" << endl;
            exit(1);
        }
    } else if (argc == 5) {
        if (strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-p") == 0) {
            memset(asip, '\0', 32);
            memset(port, '\0', 6);
            strcpy(asip, argv[2]);
            strcpy(port, argv[4]);
        } else if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-n") == 0) {
            memset(asip, '\0', 32);
            memset(port, '\0', 6);
            strcpy(asip, argv[4]);
            strcpy(port, argv[2]);
        } else {
            cout << "Invalid arguments" << endl;
            exit(1);
        }
    } else if (argc > 5) {
        cout << "Invalid number of arguments" << endl;
        exit(1);
    }
    
    User user("", "", "", "", "", "", "");
    
    vector<string> message;

    char buffer[BUFSIZE]; // buffer to store data

    struct sigaction stop; // CTRL_C signal handler
    
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1, "us_38"); // set signal handler to safe_stop for SIGINT

    while(true) {

        check((read(0, buffer, BUFSIZE)) == -1, "us_42"); // read from stdin
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
    struct addrinfo hints, *res;
    int fd, file_size = 0, bytes_read = 0;
    char buffer[BUFSIZE], data[MAXDATASIZE];
    FILE *file_fd = NULL;

    memset(buffer, '\0', BUFSIZE);
    memset(data, '\0', MAXDATASIZE);

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1, "us_85");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    check((getaddrinfo(asip, PORT, &hints, &res)) != 0, "us_90");
    check((connect(fd, res->ai_addr, res->ai_addrlen)) == -1, "us_92");

    if (message.substr(0, 3) == "OPA") {
        vector<string> msg_str = string_to_vector(message);
        file_fd = fopen(msg_str[6].c_str(), "rb");  // Open file in binary mode

        if (file_fd == NULL) {
            cout << "Error opening file or file doesn't exist." << endl;
            return -1;
        }

        file_size = check_file_size(msg_str[6].c_str());

        // Append file size to the message
        message += " " + to_string(file_size) + " ";
    }

    strcpy(buffer, message.c_str());

    write(fd, buffer, BUFSIZE);

    while (file_size > 0) {

        bytes_read = fread(data, 1, MAXDATASIZE, file_fd);
        
        // Write the bytes read from the file to the socket
        ssize_t bytes_written = write(fd, data, bytes_read);
        check(bytes_written != bytes_read, "us_117");
        
        file_size -= bytes_read;
        
        memset(data, '\0', MAXDATASIZE);
        memset(buffer, '\0', BUFSIZE);

    }

    fclose(file_fd);

    // Read the server's reply
    memset(buffer, '\0', BUFSIZE);
    check((read(fd, buffer, BUFSIZE)) == -1, "us_130");
    
    cout << buffer;

    vector<string> buffer_vec = string_to_vector(buffer);

    if (buffer_vec[0] == "RSA" && buffer_vec[1] == "OK") {

        int bytes_to_read = stoi(buffer_vec[3]);
        string file_copy_path = "./" + buffer_vec[2];
        ssize_t bytes_read = 0;
        
        FILE *file_copy_fd = fopen(file_copy_path.c_str(), "wb");
        if (file_copy_fd == NULL) {
            cout << "Error opening file." << endl;
            return -1;
        }

        while (bytes_to_read > 0) {
            memset(data, '\0', MAXDATASIZE);
            bytes_read = read(fd, data, MAXDATASIZE);
            check(bytes_read == -1, "us_148");
            bytes_to_read -= bytes_read;
            fwrite(data, 1, bytes_read, file_copy_fd);
        }
        fclose(file_copy_fd);
    }

    freeaddrinfo(res);
    close(fd);

    return 1;
}


int udp_message(char *asip, char *port, string message) {

    struct addrinfo hints, *res; //hints: info we want, res: info we get
    int fd; //fd: file descriptor
    char buffer[BUFSIZE]; //pointer to buffer and buffer to store data
    int bytes_read = 0, bytes_written = 0;
    socklen_t addrlen;
    struct sockaddr_in addr;

    strcpy(buffer, message.c_str());
        
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1, "us_147"); //UDP socket

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    check(getaddrinfo(asip, PORT, &hints, &res) != 0, "us_153"); // get address info

    bytes_written = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);

    memset(buffer, '\0', BUFSIZE); // initialize buffer to 0s

    bytes_read = recvfrom(fd, buffer, 6002, 0, (struct sockaddr*)&addr, &addrlen);
    check(/*TODO: APANHAR ERRO DO SERVIDOR SE ELE FECHAR*/false, "us_162");

    cout << buffer;

    freeaddrinfo(res); // free address info
    close(fd);

    (void) bytes_written;
    (void) bytes_read;

    return 1;
}

vector<string> string_analysis(char* str, User &user) {
    
    vector<string> message; // vector to store the final message
    
    istringstream iss(str); // istringstream to read each token
    string token; // string to store each token

    while (iss >> token) {
        message.push_back(token); 
    }

    if (message.size() == 1) {

        if (message[0] == "logout") { // LOGOUT
            message = {"udp", "LOU", user.getUID(), user.getPassword()};
            user.set_logged_out();

        } else if (message[0] == "unregister") { // UNREGISTER
            message = {"udp", "UNR", user.getUID(), user.getPassword()};
            user.set_unregistered();
            user.set_logged_out();

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

        if (message[0] == "login" && possible_UID(message[1]) && possible_password(message[2]) && message.size() == 3) { // LOGIN
            user.login(message[1], message[2]);
            message = {"udp", "LIN", user.getUID(), user.getPassword()};

        } else if (message[0] == "open" && possible_auction_name(message[1]) && possible_fname(message[2]) && //OPEN
            message.size() == 5 && possible_start_value(message[3]) && possible_time_active(message[4])) { 
            message = {"tcp", "OPA", user.getUID(), user.getPassword(), message[1], message[3], message[4], message[2]};

        } else if ((message[0] == "bid" || message[0] == "b") && possible_AID(message[1]) && possible_start_value(message[2]) && message.size() == 3) {
            message = {"tcp", "BID", user.getUID(), user.getPassword(), message[1], message[2]};
        }
    }

    return message;
}
