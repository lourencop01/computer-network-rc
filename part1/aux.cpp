#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <fstream>
#include <ctime>
#include <iomanip>

#include "headers.h"

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

bool possible_auction_name(string auction_name) {
    if(auction_name.size() > 10) {
        return false;
    }
    for(size_t i = 0; i < auction_name.size(); i++) {
        if(!isalpha(auction_name[i]) && auction_name[i] != ' ') {
            return false;
        }
    }
    return true;
}

bool possible_start_value(string start_value) {
    if(start_value.size() > 6) {
        return false;
    }
    for(size_t i = 0; i < start_value.size(); i++) {
        if(!isdigit(start_value[i])) {
            return false;
        }
    }
    return true;
}

bool possible_time_active(string time_active) {
    if(time_active.size() > 5) {
        return false;
    }
    for(size_t i = 0; i < time_active.size(); i++) {
        if(!isdigit(time_active[i])) {
            return false;
        }
    }
    return true;
}

bool possible_fname(string fname) {
    (void) fname;
    return true;
}

string vector_to_string(vector<string> message) {

    string str = "";

    message.erase(message.begin());

    while (message.back() == "") {
        message.pop_back();
    }

    for(size_t i = 0; i < message.size(); i++) {
        str += message[i];
        str += " ";
    }

    str.pop_back();

    return str;
}

vector<string> string_to_vector(string str) {

    vector<string> vec;
    stringstream ss(str);
    string tok;

    while(getline(ss, tok, ' ')) {
        vec.push_back(tok);
    }

    return vec;

}

ssize_t create_pass_file(User &user) {

    FILE *fp = NULL;

    fp = fopen((user.get_pass_pathname()).c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "%s\n", user.getPassword().c_str());
    fclose(fp);

    return 1;

}

ssize_t create_login_file(User &user) {

    FILE *fp = NULL;

    fp = fopen((user.get_login_pathname()).c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "Logged in\n");
    fclose(fp);

    return 1;

}

ssize_t create_start_aid_file(Auction &auction) {

    FILE *fp = NULL;

    fp = fopen((auction.get_start_aid_pathname()).c_str(), "w");
    if (fp == NULL) {
        return -1;
    }

    // Get the current time
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Convert the current time to a string with the desired format
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
    std::string timestampStr = oss.str();

    fprintf(fp, "%s %s %s %s %s %s %ld\n", auction.get_hosted_by().c_str(), auction.get_name().c_str(), auction.get_asset_fname().c_str(),
                                          to_string(auction.get_start_value()).c_str(), to_string(auction.get_time_active()).c_str(), 
                                          timestampStr.c_str(), time(NULL));
    fclose(fp);

    return 1;

}

ssize_t create_hosted_file(User &user, Auction &auction) {

    FILE *fp = NULL;

    string hosted_pathname = user.get_hosted_pathname() + "/" + auction.get_AID() + ".txt";

    fp = fopen(hosted_pathname.c_str(), "a");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "%s\n", auction.get_AID().c_str());
    fclose(fp);

    return 1;

}

ssize_t delete_login_file(User &user) {

    unlink(user.get_login_pathname().c_str());

    return 1;

}

ssize_t delete_pass_file(User &user) {

    unlink(user.get_pass_pathname().c_str());

    return 1;

}

void load_users(Users &users){
    
    DIR *dir;
    struct dirent *ent;
    string pathname = "USERS/";
    User user("","","","","","","");

    if ((dir = opendir(pathname.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR && ent->d_name[0] != '.') {
                
                string UID = ent->d_name;
                user.setUID(UID);

                user.set_uid_pathname(pathname + UID);
                user.set_pass_pathname(pathname + UID + "/" + UID + "_pass.txt");
                user.set_login_pathname(pathname + UID + "/" + UID + "_login.txt");
                user.set_hosted_pathname(pathname + UID + "/" + "HOSTED");
                user.set_bids_pathname(pathname + UID + "/" + "BIDDED");

                string password = "";
                ifstream pass_file(user.get_pass_pathname());
                ifstream login_file(user.get_login_pathname());
                
                if (pass_file.is_open()) {
                    getline(pass_file, password);
                    pass_file.close();
                    user.setPassword(password);
                } else {
                    user.set_unregistered();
                }

                if (login_file.is_open()) {
                    login_file.close();
                    user.set_logged_in();
                } else {
                    user.set_logged_out();
                }

                users.add_user(user);
            }
        }
        closedir(dir);
    } else {
        perror("");
    }
}
