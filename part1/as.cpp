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
#include <sys/mman.h>
#include <filesystem>

#include "headers.h"

#define PORT "58070"
#define BUFSIZE 128
#define MAXDATASIZE 1024
#define MAXQUEUE 5

using namespace std;
namespace fs = std::filesystem;

thread timer_thread;

int main() {

    struct sigaction stop; // CTRL_C signal handler
    memset(&stop, 0, sizeof(stop)); // initialize signal handler to 0s
    stop.sa_handler = safe_stop; // set handler to safe_stop function
    check(sigaction(SIGINT, &stop, NULL) == -1, "as_40"); // set signal handler to safe_stop for SIGINT

    thread udpThread(udp_server);

    tcp_server();

    udpThread.join();

    return 0;

}
    
int tcp_server() {

    struct addrinfo hints, *res; // hints: info we want, res: info we get
    int fd, newfd, ret; // file descriptors
    struct sockaddr_in addr; // address of the server
    socklen_t addrlen; // size of the address

    int bytes_to_read = 0;
    int read_bytes = 0;
    FILE *asset_file = NULL;

    char buffer[BUFSIZE], reply[128], data[MAXDATASIZE];
    memset(buffer, '\0', BUFSIZE);
    memset(reply, '\0', 128);
    memset(data, '\0', MAXDATASIZE);

    pid_t pid;

    size_t thread_created = 0;

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

            vector<string> buffer_vec = string_to_vector(buffer);

            if (buffer_vec[0] == "OPA") {

                strcpy(reply, vector_analysis(buffer_vec).c_str());

                if (string_to_vector(reply)[1] == "OK") {

                    string AID = string_to_vector(reply)[2];

                    bytes_to_read = stoi(buffer_vec[7]);
                    read_bytes = 0;

                    asset_file = fopen(("AUCTIONS/" + AID + "/" + buffer_vec[6]).c_str(), "wb");
                    if (asset_file == NULL) {
                        cout << "Error creating asset file" << endl;
                        return -1;
                    }
                     
                    while (bytes_to_read > 0) {
                        memset(buffer, '\0', BUFSIZE);
                        check((read_bytes = read(newfd, buffer, BUFSIZE)) == -1, "as_137");
                        fwrite(buffer, 1, read_bytes, asset_file);
                        bytes_to_read -= read_bytes;
                    }

                    fclose(asset_file);

                    thread_created = 1;
                }

            } else {

                strcpy(reply, vector_analysis(buffer_vec).c_str());

            }

            cout << "TCP sent: " << reply << endl;
            check((write(newfd, reply, BUFSIZE)) <= 0, "as_150");

            if (string_to_vector(reply)[0] == "RSA" && string_to_vector(reply)[1] == "OK") {
                
                    vector<string> reply_vec = string_to_vector(reply);

                    string asset_fname = reply_vec[2];
                    int bytes_to_read = stoi(reply_vec[3]);
                    string asset_to_copy_pathname = "AUCTIONS/" + buffer_vec[1] + "/" + asset_fname;

                    if (bytes_to_read == 0) {
                        cout << "Error with file size" << endl;
                        return -1;
                    }

                    FILE *asset_file = fopen(asset_to_copy_pathname.c_str(), "rb");
                    if (asset_file == NULL) {
                        cout << "Error opening asset file" << endl;
                        return -1;
                    }
    
                    read_bytes = 0;
                    
                    while (bytes_to_read > 0) {
                        read_bytes = fread(data, 1, MAXDATASIZE, asset_file);
                        check((write(newfd, data, read_bytes)) <= 0, "as_178");
                        bytes_to_read -= read_bytes;
                        memset(data, '\0', BUFSIZE);
                    }
    
                    fclose(asset_file);
            }

            memset(buffer, '\0', BUFSIZE);
            memset(reply, '\0', 128);
            memset(data, '\0', MAXDATASIZE);

            close(newfd);

            if (thread_created) { timer_thread.join(); }
            exit(0);
        }
        do {
            ret = close(newfd);
        } while (ret == -1 && errno == EINTR);
    }

    close(fd);
    return 1;
}

int udp_server() {
    
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

        strcpy(ptr, vector_analysis(string_to_vector(buffer)).c_str()); // convert buffer to vector

        cout << "UDP sent: " << ptr << endl;

        check(sendto(fd, ptr, BUFSIZE, 0, (struct sockaddr*)&addr, addrlen) == -1, "as_205"); // send data to socket

    }

    freeaddrinfo(res); // free address info
    close(fd); // close socket

    return 0;
}

string vector_analysis(vector<string> message) {

        if(message[0] == "LIN") { // Todo: What happens if user already logged in
            return "RLI " + login(message[1], message[2]);
        } else if(message[0] == "LOU") { // User requests for logout
            return "RLO "+ logout(message[1], message[2]);
        } else if(message[0] == "UNR") {
            return "RUR " + unregister(message[1], message[2]);
        } else if(message[0] == "LMA") {
            return "RMA " + myauctions(message[1]);
        } else if(message[0] == "LMB") {
            return "RMB " + mybids(message[1]);
        } else if(message[0] == "LST") {
            return "RLS " + list();
        } else if(message[0] == "show_record") {
            //mybids(message[1], message[2]);
        } else if(message[0] == "OPA") {
            return "ROA " + open(message[1], message[2], message[3], message[4], message[5], message[6]);
        } else if(message[0] == "CLS") {
            return "RCL " + close(message[1], message[2], message[3]);
        } else if(message[0] == "SAS") {
            return "RSA " + show_asset(message[1]);
        } else if(message[0] == "BID") {
            return "RBD " + bid(message[1], message[2], message[3], message[4]);
        } else {
            return "ERR";
        }
    return "ERR";
}

string login(string UID, string password) {


    if (!possible_UID(UID) || !possible_password(password)) {
        return "ERR";

    } else if (!user_directory_exists(UID)) {// new user

        string uid_dir_path = "USERS/" + UID;
        string pass_path = uid_dir_path + "/" + UID + "_pass.txt";
        string login_path = uid_dir_path + "/" + UID + "_login.txt";
        string hosted_dir_path = uid_dir_path + "/" + "HOSTED";
        string bids_dir_path = uid_dir_path + "/" + "BIDDED";

        mkdir(uid_dir_path.c_str(), 0700);
        mkdir(hosted_dir_path.c_str(), 0700);
        mkdir(bids_dir_path.c_str(), 0700);

        create_pass_file(UID, password);
        create_login_file(UID);

        return "REG";

    } else if (check_user_login_file(UID) == 0 && check_user_password_file(UID) == "") { // user existed but unregistered
        
        create_login_file(UID);
        create_pass_file(UID, password);

        return "OK";

    } else if (check_user_login_file(UID) == 0 && check_user_password_file(UID) != "") { // user exists, is registered, and logged out

        if (password != check_user_password_file(UID)) {
            return "NOK";
        }

        create_login_file(UID);

        return "OK";

    } else { // user already logged in
        // TODO what to do in this case?
        cout << "User already logged in" << endl;
        return "LOG";
    }

    return "ERR";

}

string logout(string UID, string password) { // logged in and reg, logged out and reg, logged out and unr

    if (!user_directory_exists(UID)) { // user never existed (never registered before)
        return "UNR";

    } else if (check_user_login_file(UID) != 0 && check_user_password_file(UID) == "") { // user logged in but unregistered
        return "ERR";

    } else if (check_user_login_file(UID) == 0 && check_user_password_file(UID) != "") { // user exists, is registered, and logged out
        return "NOK";

    } else { // user exists, is registered, and logged in

        if (check_user_password_file(UID) != password) {
            return "NOK";
        }

        delete_login_file(UID); // delete login file from user

        return "OK";
    }

    return "ERR";

}

string unregister(string UID, string password) { // logged in and reg, logged out and reg, logged out and unr

    if (!user_directory_exists(UID)) { // user never existed (never registered before)
        return "UNR";

    } else if (check_user_login_file(UID) != 0 && check_user_password_file(UID) == "") { // user logged in but unregistered
        return "ERR";

    } else if (check_user_login_file(UID) == 0 && check_user_password_file(UID) != "") { // user exists, is registered, and logged out
        return "NOK";

    } else { // user exists, is registered, and logged in
        
        if (check_user_password_file(UID) != password) {
            return "NOK";
        }

        delete_login_file(UID);
        delete_pass_file(UID);

        return "OK";
    }

    return "ERR";

}

string myauctions(string UID) {

    if (!user_directory_exists(UID) || check_user_login_file(UID) == 0) { // user never existed or user unregistered or user logged out
        return "NLG";
    } else if (user_hosted_directory_empty(UID)) { // user never hosted an auction
        return "NOK";
    } else { // user hosted at least one auction and is logged in
        string message = "OK ";

        string AID = "";

        for (const auto & entry : fs::directory_iterator("USERS/" + UID + "/HOSTED")) {
            AID = entry.path().filename().string();
            if (auction_is_active(AID.erase(3, 4))) {
                message += AID + " 1 ";
            } else {
                message += AID + " 0 ";
            }
        }

        return message;
    }
}

string mybids(string UID) {

    if (!user_directory_exists(UID) || check_user_login_file(UID) == 0) { // user never existed or user unregistered or user logged out
        return "NLG";
    } else if (user_bidded_directory_empty(UID)) { // user never bidded an auction
        return "NOK";
    } else { // user bidded at least one auction and is logged in
        string message = "OK ";

        string AID = "";

        for (const auto & entry : fs::directory_iterator("USERS/" + UID + "/BIDDED")) {
            AID = entry.path().filename().string();
            if (auction_is_active(AID.erase(3, 4))) {
                message += AID + " 1 ";
            } else {
                message += AID + " 0 ";
            }
        }
        
        return message;
    }


}

string list() {

    if (auction_directory_empty()) { // no auctions exist
        return "NOK";
    } else {
        string message = "OK ";

        //TODO é suposto ser so quando tem algum auction ativo? e se tiver varios mas todos inativos

        string AID = "";

        for (const auto & entry : fs::directory_iterator("AUCTIONS")) {
            AID = entry.path().filename().string();
            if (auction_is_active(AID)) {
                message += AID + " 1 ";
            } else {
                message += AID + " 0 ";
            }
        }
        return message;
    }

}

string open(string UID, string password, string name, string start_value, string time_active, string fname) {

    string AID = "";

    if (!user_directory_exists(UID) || check_user_login_file(UID) == 0) { // user never existed (never registered before) or logged out
        return "NLG";

    } else { // user exists, is registered, and logged in
        
        if (check_user_password_file(UID) != password) {
            return "ERR";
        }

        if ((AID = count_directories("AUCTIONS")) == "-1") {
            return "NOK";
        
        } else {

            string auction_pathname = "AUCTIONS/" + AID;
            string auction_bids_pathname = auction_pathname + "/BIDS";
            
            mkdir(auction_pathname.c_str(), 0700); // create auction directory named with its AID under AUCTIONS folder
            mkdir(auction_bids_pathname.c_str(), 0700); // create a BIDS folder under the auction directory
            
            long int start_time_1970 = create_start_aid_file(AID, name, start_value, time_active, fname, UID); // create START_AID.txt file
            // TODO copy asset_fname's file to the auction directory

            timer_thread = thread(monitor_auction_end, AID, stoi(time_active), start_time_1970); // create thread to monitor auction end
            
            create_hosted_file(UID, AID); // create HOSTED file
            return "OK " + AID;
        }
        
        return "ERR";
    }

    return "ERR";

}

string close(string UID, string password, string AID) {

    string end_pathname = "AUCTIONS/" + AID + "/" + AID + "_end.txt";

    if (!user_directory_exists(UID) || check_user_login_file(UID) == 0) { // user never existed (never registered before) or logged out
        return "NLG";
    } else if (!auction_directory_exists(AID)) { // Auction does not exist
        return "EAU";
    } else if (string_to_vector(auction_start_line(AID))[0] != UID) { // Auction was not hosted by user logged in
        return "EOW";
    } else if (check_file_size(end_pathname.c_str()) != 0) { // Auction is already closed
        return "END";
    } else { // user exists, is registered, and logged in and auction is created and ongoing
        
        vector<string> start_file_strings = string_to_vector(auction_start_line(AID));

        int start_time_1970 = stoi(start_file_strings[7]);

        if (check_user_password_file(UID) != password) {
            return "ERR";
        }

        create_end_aid_file(AID, start_time_1970); // create END_AID.txt file
        
        return "OK";
    }

    return "ERR";

}

string show_asset(string AID) {

    string auction_folder_pathname = "AUCTIONS/" + AID + "/";

    if (!auction_directory_exists(AID)) { // Auction does not exist
        return "NOK";

    } else { // Auction folder exists

        vector<string> start_file_strings = string_to_vector(auction_start_line(AID)); // UID name asset_fname start_value timeactive start_datetime(2) start_fulltime

        int file_size = 0;

        if ((file_size = check_file_size((auction_folder_pathname + start_file_strings[2]).c_str())) == 0) {
            return "NOK";
        }

        return ("OK " + start_file_strings[2] + " " + to_string(file_size));

    }

    return "ERR";
}

string bid(string UID, string password, string AID, string value) {

    if (!user_directory_exists(UID) || check_user_login_file(UID) == 0) { // user never existed (never registered before) or logged out
        return "NLG";
    } else if (!auction_is_active(AID)) {
        return "NOK";
    } else if (stoi(value) < maximum_bid(AID)) {
        return "REF";
    } else if (string_to_vector(auction_start_line(AID))[0] == UID) {
        return "ILG";
    } else {
            
            if (check_user_password_file(UID) != password) {
                return "ERR";
            }
    
            create_bid_file_auction(UID, AID, value);
            create_bid_file_user(UID, AID);
    
            return "OK";
    }

        //TODO what if the bid is lower than the start value?
        
    return "ERR";
}




