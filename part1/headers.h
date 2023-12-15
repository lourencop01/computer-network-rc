#ifndef AUX_H
#define AUX_H

#include <iostream>
#include <iomanip>

using namespace std;

class User {
    public:
        
        string UID;
        string password;
        bool logged_in;
        bool unregistered;
        string uid_pathname;
        string pass_pathname;
        string login_pathname;
        string hosted_pathname;
        string bids_pathname;

        User(string UID, string password, string uid_pathname, string pass_pathname, string login_pathname, string hosted_pathname, string bids_pathname) {
            this->UID = UID;
            this->password = password;
            this->logged_in = true;
            this->unregistered = false;
            this->uid_pathname = uid_pathname;
            this->pass_pathname = pass_pathname;
            this->login_pathname = login_pathname;
            this->hosted_pathname = hosted_pathname;
            this->bids_pathname = bids_pathname;
        }

        string getUID() const {
        return this->UID;
        }
        string getPassword() const {
            return this->password;
        }
 
        bool is_logged_in() const {
            return logged_in;
        }
        bool is_logged_out() const {
            return !logged_in;
        }
        bool is_unregistered() const {
            return unregistered;
        }
        string get_uid_pathname() const {
            return this->uid_pathname;
        }
        string get_pass_pathname() const {
            return this->pass_pathname;
        }
        string get_login_pathname() const {
            return this->login_pathname;
        }
        string get_hosted_pathname() const {
            return this->hosted_pathname;
        }
        string get_bids_pathname() const {
            return this->bids_pathname;
        }
        void setUID(string UID) {
            this->UID = UID;
        }
        void setPassword(string password) {
            this->password = password;
        }
        void set_logged_in() {
            logged_in = true;
        }
        void set_logged_out() {
            this->logged_in = false;
        }
        void set_unregistered() {
            this->unregistered = true;
        }
        void set_registered() {
            this->unregistered = false;
        }
        void set_uid_pathname(string uid_pathname) {
            this->uid_pathname = uid_pathname;
        }
        void set_pass_pathname(string pass_pathname) {
            this->pass_pathname = pass_pathname;
        }
        void set_login_pathname(string login_pathname) {
            this->login_pathname = login_pathname;
        }
        void set_hosted_pathname(string hosted_pathname) {
            this->hosted_pathname = hosted_pathname;
        }
        void set_bids_pathname(string bids_pathname) {
            this->bids_pathname = bids_pathname;
        }
        void login(string UID, string password) {
            setUID(UID);
            setPassword(password);
            this->logged_in = true;
            this->unregistered = false;
        }
        void re_register(string password) {
            setPassword(password);
            this->unregistered = false;
            this->logged_in = true;
        }
        void unregister() {
            this->password = "";
            this->unregistered = true;
        }
};

/* aux.c file functions */
void check(bool condition, string message);
void safe_stop(int signal);
string int_to_three_digit_string(int number);
string int_to_six_digit_string(int number);
bool possible_UID(string UID);
bool possible_password(string password);
bool possible_AID(string AID);
bool possible_auction_name(string auction_name);
bool possible_start_value(string start_value);
bool possible_time_active(string time_active);
bool possible_fname(string fname);
string vector_to_string(vector<string> message);
vector<string> string_to_vector(string str);
vector<string> string_analysis(char* str);
int check_user_login_file(string UID);
string check_user_password_file(string UID);
string check_auction_start_file(string AID);
string check_auction_end_file(string AID);
string check_bid_file(string AID, string bid_file_name);
bool user_directory_exists(string UID);
bool user_hosted_directory_empty(string UID);
bool user_bidded_directory_empty(string UID);
bool auction_directory_empty();
bool auction_directory_exists(string AID);
string count_directories(string directoryPath);
string auction_start_line(string AID);
int maximum_bid(string AID);
bool auction_is_active(string AID);

ssize_t create_pass_file(string UID, string password);
ssize_t create_login_file(string UID);
long int create_start_aid_file(string AID, string name, string start_value, string time_active, string fname, string UID);
ssize_t create_hosted_file(string UID, string AID);
ssize_t create_bid_file_auction(string UID, string AID, string value);
ssize_t create_bid_file_user(string UID, string AID);
ssize_t create_end_aid_file(string AID, int time_since_1970);
ssize_t delete_login_file(string UID);
ssize_t delete_pass_file(string UID);
string intToThreeDigitString(int number);
int check_file_size(const char *fname);
void monitor_auction_end(string AID, int time_active, int start_time_1970);

/* user.cpp file functions */
int tcp_message(char *asip, char *port, string message);
int udp_message(char *asip, char *port, string message);
vector<string> string_analysis(char* str, User &user);

/* as.cpp file functions */
int tcp_server();
int udp_server();
string vector_analysis(vector<string> message);
string login(string UID, string password);
string logout(string UID, string password);
string unregister(string UID, string password);
string myauctions(string UID);
string mybids(string UID);
string list();
string open(string UID, string password, string name, string start_value, string time_active, string fname);
string close(string UID, string password, string AID);
string show_asset(string AID);
string bid(string UID, string password, string AID, string value);
string show_record(string AID);

#endif // AUX_H
