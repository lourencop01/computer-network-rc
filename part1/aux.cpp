#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <iostream>

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
        if(!isalpha(auction_name[i]) && auction_name[i] != ' ') { // TODO: CHECK IF SPACE IS ALLOWED
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
    if(time_active.size() < 5) {
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

    return 0;

}

ssize_t create_login_file(User &user) {

    FILE *fp = NULL;

    fp = fopen((user.get_login_pathname()).c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "Logged in\n");
    fclose(fp);

    return 0;

}
