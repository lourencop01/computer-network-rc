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
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include <filesystem>

#include "headers.h"

using namespace std;
namespace fs = std::filesystem;

/*
* Checks if condition is true. In that case, exits with error code 1.
*/
void check(bool condition, string message) { if(condition) { cout << message << endl; exit(1); }}

void safe_stop(int signal) {
    (void) signal;
    cout << "\nExiting..." << endl;
    exit(0);
}

string int_to_three_digit_string(int number) {
    if (number > 999 || number < 0) {
        return "LIMIT EXCEEDED";
    }
    ostringstream oss;
    oss << setw(3) << setfill('0') << number;
    return oss.str();
}

int check_file_size(const char *fname) {
    struct stat filestat; 
    int ret_stat;

    ret_stat = stat(fname, &filestat);

    if ( ret_stat == -1 || filestat.st_size == 0)
        return (0);

    return(filestat.st_size);
}

int check_user_login_file(string UID) {
    if (UID == "") {
        return 0;
    }
    string pathname = "USERS/" + UID + "/" + UID + "_login.txt";
    return check_file_size(pathname.c_str());
}

string check_user_password_file(string UID) {

    if (UID == "") {
        return "";
    }
    
    FILE *fp = NULL;

    string pathname = "USERS/" + UID + "/" + UID + "_pass.txt";

    if (check_file_size(pathname.c_str()) == 0) {
        return "";
    }

    fp = fopen(pathname.c_str(), "r");
    if (fp == NULL) {
        return "";
    }

    char pass[9];
    memset(pass, '\0', 9);
    fscanf(fp, "%s", pass);

    return pass;
}

string check_auction_start_file(string AID) {
    
    FILE *fp = NULL;

    string pathname = "AUCTIONS/" + AID + "/" + AID + "_start.txt";

    if (check_file_size(pathname.c_str()) == 0) {
        return "";
    }

    fp = fopen(pathname.c_str(), "r");
    if (fp == NULL) {
        return "";
    }

    char content[128];
    memset(content, '\0', 128);
    fscanf(fp, "%s", content);

    return content;
}

string check_auction_end_file(string AID) {
    
    FILE *fp = NULL;

    string pathname = "AUCTIONS/" + AID + "/" + AID + "_end.txt";

    if (check_file_size(pathname.c_str()) == 0) {
        return "";
    }

    fp = fopen(pathname.c_str(), "r");
    if (fp == NULL) {
        return "";
    }

    char content[128];
    memset(content, '\0', 128);
    fscanf(fp, "%s", content);

    return content;
}

bool user_directory_exists(string UID) {
    string directoryPath = "USERS/" + UID;
    return (fs::exists(directoryPath) && fs::is_directory(directoryPath) && UID != "");
}

bool user_hosted_directory_empty(string UID) {
    string directoryPath = "USERS/" + UID + "/HOSTED";
    return (fs::exists(directoryPath) && fs::is_directory(directoryPath) && fs::is_empty(directoryPath) && UID != "");
}

bool user_bidded_directory_empty(string UID) {
    string directoryPath = "USERS/" + UID + "/BIDDED";
    return (fs::exists(directoryPath) && fs::is_directory(directoryPath) && fs::is_empty(directoryPath) && UID != "");
}

bool auction_directory_empty() { // TODO WHY DOESNT THIS ONE WORK
    string directoryPath = "AUCTIONS";
    if (fs::exists(directoryPath)){ cout << "exists" << endl; }
    if (fs::is_directory(directoryPath)){ cout << "is dir" << endl; }
    if (fs::is_empty(directoryPath)){ cout << "is empty" << endl; } 
    return (fs::exists(directoryPath) && fs::is_directory(directoryPath) && fs::is_empty(directoryPath));
}

bool auction_directory_exists(string AID) {
    string directoryPath = "AUCTIONS/" + AID;
    return (fs::exists(directoryPath) && fs::is_directory(directoryPath) && AID != "");
}

string count_directories(string directoryPath) {
    int count = 0;
    for (const auto & entry : fs::directory_iterator(directoryPath)) {
        if (fs::is_directory(entry)) {
            count++;
        }
    }

    if (count > 999) {
        return "-1";
    }

    return int_to_three_digit_string(count);
}

// function retrieves the first line of the file
string auction_start_line(string AID) {
    string pathname = "AUCTIONS/" + AID + "/" + AID + "_start.txt";
    ifstream file(pathname);
    string line;
    getline(file, line);
    return line;
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

ssize_t create_pass_file(string UID, string password) {

    FILE *fp = NULL;

    string pathname = "USERS/" + UID + "/" + UID + "_pass.txt";

    fp = fopen(pathname.c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "%s\n", password.c_str());
    fclose(fp);

    return 1;

}

ssize_t create_login_file(string UID) {

    FILE *fp = NULL;

    string pathname = "USERS/" + UID + "/" + UID + "_login.txt";

    fp = fopen(pathname.c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "Logged in\n");
    fclose(fp);

    return 1;

}

long int create_start_aid_file(string AID, string name, string start_value, string time_active, string fname, string UID) {

    FILE *fp = NULL;

    string pathname = "AUCTIONS/" + AID + "/" + AID + "_start.txt";

    fp = fopen(pathname.c_str(), "w");
    if (fp == NULL) {
        return -1;
    }

    // Get the current time
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Convert the current time to a string with the desired format
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
    std::string timestampStr = oss.str();

    long int start_time_1970 = time(NULL);

    fprintf(fp, "%s %s %s %s %s %s %ld\n", UID.c_str(), name.c_str(), fname.c_str(), start_value.c_str(),
                                            time_active.c_str(), timestampStr.c_str(), start_time_1970);

    fclose(fp);

    return start_time_1970;

}

ssize_t create_end_aid_file(string AID, int time_since_1970) {
    
        FILE *fp = NULL;

        string end_aid_pathname = "AUCTIONS/" + AID + "/" + AID + "_end.txt";

        //  check if end file already exists
        if (check_file_size(end_aid_pathname.c_str()) != 0) {
            return -1;
        }
    
        fp = fopen(end_aid_pathname.c_str(), "w");
        if (fp == NULL) {
            return -1;
        }
    
        // Get the current time
        auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
        // Convert the current time to a string with the desired format
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
        std::string timestampStr = oss.str();
        cout << "create end file " << time_since_1970 << endl;
        fprintf(fp, "%s %ld\n", timestampStr.c_str(), time(NULL) - time_since_1970);
        fclose(fp);
    
        return 1;
}

ssize_t create_hosted_file(string UID, string AID) {

    FILE *fp = NULL;

    string hosted_pathname = "USERS/" + UID + "/" + "HOSTED" + "/" + AID + ".txt";

    fp = fopen(hosted_pathname.c_str(), "w");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "%s\n", AID.c_str());
    fclose(fp);

    return 1;

}

ssize_t delete_login_file(string UID) {

    unlink(("USERS/" + UID + "/" + UID + "_login.txt").c_str());

    return 1;

}

ssize_t delete_pass_file(string UID) {

    unlink(("USERS/" + UID + "/" + UID + "_pass.txt").c_str());

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

void monitorAuctionEnd(string AID, int time_active, int start_time_1970) {

    string end_pathname = "AUCTIONS/" + AID + "/" + AID + "_end.txt";

    while (time_active > (time(NULL) - start_time_1970)) {
        cout << "time active: " << time_active << " time since start: " << time(NULL) - start_time_1970 << endl;
        this_thread::sleep_for(chrono::seconds(1));
        if (check_file_size(end_pathname.c_str()) != 0) {
            return;
        }
    }

    // Auction has ended, create the END file
    create_end_aid_file(AID, start_time_1970);
}
