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
#include <thread>
#include <mutex>


#include "headers.h"

#define PORT "58070"
#define BUFSIZE 128
#define MAXQUEUE 5

using namespace std;

mutex dataMutex;

int main() {

    Users users;
    load_users(users); // load users from USERS folder
    users.print_all_users();

    Auctions auctions;
    //load_auctions(auctions); // load auctions from AUCTIONS folder TODO
    //auctions.print_all_auctions();

    struct sigaction stop; // CTRL_C signal handler
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1, "as_40"); // set signal handler to safe_stop for SIGINT

    std::thread udpThread(udp_server, std::ref(users), std::ref(auctions));

    tcp_server(users, auctions);

    udpThread.join();

    return 0;

}
    
int tcp_server(Users &users, Auctions &auctions) {

    struct addrinfo hints, *res; // hints: info we want, res: info we get
    int fd, newfd, ret; // file descriptors
    struct sockaddr_in addr; // address of the server
    socklen_t addrlen; // size of the address

    int bytes_to_read = 0;
    int read_bytes = 0;
    FILE *asset_file = NULL;

    char buffer[BUFSIZE], reply[128];
    memset(buffer, '\0', BUFSIZE);
    memset(reply, '\0', 128);

    pid_t pid;

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    check(sigaction(SIGCHLD, &act, NULL) == -1, "as_74");

    check((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1, "as_76");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    check(getaddrinfo(NULL, PORT, &hints, &res) != 0, "as_83");
    check(::bind(fd, res->ai_addr, res->ai_addrlen) == -1, "as_84");
    check(listen(fd, 5) == -1, "as_85");

    freeaddrinfo(res);

    while (true) {
        addrlen = sizeof(addr);

        do {
            newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
        } while (newfd == -1 && errno == EINTR);
        check(newfd == -1, "as_95");

        check((pid = fork()) == -1, "as_97");

        if (pid == 0) {
            close(fd);

            read_bytes = read(newfd, buffer, BUFSIZE);
            buffer[read_bytes] = '\0';

            cout << "TCP received: " << buffer << endl;

            if (string_to_vector(buffer)[0] == "OPA") {

                vector<string> buffer_vec = string_to_vector(buffer);

                strcpy(reply, vector_analysis(buffer_vec, users, auctions).c_str());

                if (string_to_vector(reply)[1] == "OK") {

                    string AID = string_to_vector(reply)[2];

                    bytes_to_read = stoi(buffer_vec[7]);
                    read_bytes = 0;

                    asset_file = fopen(("AUCTIONS/" + AID + "/" + buffer_vec[6]).c_str(), "wb");
                    if (asset_file == NULL) {
                        cout << "Error creating asset file" << endl;
                        return -1;
                    }

                    char ini_data[BUFSIZE];
                    memset(ini_data, '\0', BUFSIZE);
                    removeFirstNWords(buffer, ini_data, BUFSIZE, 8);
                    cout << "ini_data: " << ini_data << endl;
                    fwrite(ini_data, 1, strlen(ini_data), asset_file);
                    bytes_to_read -= strlen(ini_data);
                     
                    while (bytes_to_read > 0) {
                        memset(buffer, '\0', BUFSIZE);
                        check((read_bytes = read(newfd, buffer, BUFSIZE)) == -1, "as_137");
                        fwrite(buffer, 1, read_bytes, asset_file);
                        bytes_to_read -= read_bytes;
                    }

                    fclose(asset_file);
                }

            } else {

                strcpy(reply, vector_analysis(string_to_vector(buffer), users, auctions).c_str());

            }
            cout << "TCP sent: " << reply << endl;
            check((write(newfd, reply, BUFSIZE)) <= 0, "as_150");

            memset(buffer, '\0', BUFSIZE);
            memset(reply, '\0', 128);

            close(newfd);
            exit(0);
        }
        do {
            ret = close(newfd);
        } while (ret == -1 && errno == EINTR);
    }

    close(fd);
    return 1;
}

int udp_server(Users &users, Auctions &auctions) {
    
    struct addrinfo hints, *res;
    int fd; // file descriptor
    ssize_t n; // number of bytes read
    struct sockaddr_in addr; // address of the server
    socklen_t addrlen; // size of the address
    char *ptr, buffer[BUFSIZE]; // buffer to store data

    ptr = buffer;
    
    check((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1, "as_179"); // create socket with UDP protocol

    memset(&hints, 0, sizeof(hints)); // initialize hints to 0s
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE; // use local IP address

    check(getaddrinfo(NULL, PORT, &hints, &res) != 0, "as_186"); // get address info

    check(::bind(fd, res->ai_addr, res->ai_addrlen) == -1, "as_188"); // bind socket to address

    while(true) {

        memset(buffer, '\0', BUFSIZE); // initialize buffer to '\0'
        ptr = buffer; // reset pointer to buffer

        addrlen = sizeof(addr); // set address length

        check((n = recvfrom(fd, buffer, BUFSIZE, 0, (struct sockaddr*)&addr, &addrlen)) == -1, "as_197"); // receive data from socket
        ptr += n; // update pointer to end of buffer
        cout << "UDP received: " << buffer << endl;

        strcpy(ptr, vector_analysis(string_to_vector(buffer), users, auctions).c_str()); // convert buffer to vector

        cout << "UDP sent: " << ptr << endl;

        check(sendto(fd, ptr, BUFSIZE, 0, (struct sockaddr*)&addr, addrlen) == -1, "as_205"); // send data to socket

    }

    freeaddrinfo(res); // free address info
    close(fd); // close socket

    return 0;
}

string vector_analysis(vector<string> message, Users &users, Auctions &auctions) {

        if(message[0] == "LIN") { // Todo: What happens if user already logged in
            return "RLI " + login(message[1], message[2], users);
        } else if(message[0] == "LOU") { // User requests for logout
            return "RLO "+ logout(message[1], message[2], users);
        } else if(message[0] == "UNR") {
            return "RUR " + unregister(message[1], message[2], users);
        } else if(message[0] == "LMA") {
            return "RMA " + myauctions(message[1], users);
        } else if(message[0] == "LMB") {
            return "RMB " + mybids(message[1], users);
        } else if(message[0] == "LST") {
            return "RLS " + list(auctions);
        } else if(message[0] == "show_record") {
            //mybids(message[1], message[2]);
        } else if(message[0] == "OPA") {
            return "ROA " + open(message[1], message[2], message[3], message[4], message[5], message[6], auctions, users);
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

string myauctions(string UID, Users &users) {
    cout << UID << endl;
    User *user = users.get_user(UID);
    vector<Auction> *hosted = user->get_hosted();

    if (user == nullptr) {
        return "ERR";
    }
    else if (user->is_logged_out()) {
        return "NLG";
    } else if (hosted->size() == 0) {
        return "NOK";
    } else {
        string message = "OK ";
        for (Auction auction : *hosted) {
            message += auction.get_AID() + " " + to_string(auction.get_status()) + " ";
        }
        return message;
    }
}

string mybids(string UID, Users &users) {
    
    User *user = users.get_user(UID);
    vector<Auction> *bidded = user->get_bidded();

    if (user == nullptr) {
        return "ERR";
    }
    else if (user->is_logged_out()) {
        return "NLG";
    } else if (bidded->size() == 0) {
        return "NOK";
    } else {
        string message = "OK ";
        for (Auction auction : *bidded) {
            message += auction.get_AID() + " " + to_string(auction.get_status()) + " ";
        }
        return message;
    }

}

string list(Auctions &auctions) {

    vector<Auction> *all_auctions = auctions.get_all_auctions();

    if (all_auctions->size() == 0) {
        return "NOK";
    } else {
        string message = "OK ";
        for (Auction auction : *all_auctions) {
            message += auction.get_AID() + " " + to_string(auction.get_status()) + " ";
        }
        return message;
    }

}

string open(string UID, string password, string name, string start_value, string time_active, string fname, Auctions &auctions, Users &users) {

    User *user = users.get_user(UID);

    int AID = -1;

    if (user == nullptr || user->is_logged_out() || user->is_unregistered()) { // user never existed (never registered before) or logged out
        return "NLG";

    } else { // user exists, is registered, and logged in
        
        if (user->getPassword() != password) {
            return "ERR";
        }

        if ((AID = auctions.add_auction(name, fname, stoi(start_value), stoi(time_active), UID)) == -1) {
            return "NOK";
        } else {
            
            string AID_string = auctions.intToThreeDigitString(AID);
            Auction *auction = auctions.get_auction(AID_string);
            
            mkdir((auction->get_auction_pathname()).c_str(), 0700); // create auction directory named with its AID under AUCTIONS folder
            mkdir((auction->get_bids_pathname()).c_str(), 0700); // create a BIDS folder under the auction directory
            
            create_start_aid_file(*auction); // create START_AID.txt file
            // TODO copy asset_fname's file to the auction directory
            
            user->add_hosted(*auction); // add auction to user's hosted vector
            create_hosted_file(*user, *auction); // create HOSTED file
            
            return "OK " + AID_string;
        }
        
        return "WEIRD2";
    }

    return "WEIRD";

}

