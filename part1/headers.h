#ifndef AUX_H
#define AUX_H

using namespace std;

class User {
    public:
        
        string UID;
        string password;
        bool logged_in;
        bool unregistered;
        vector<int> hosted;
        vector<int> bidded;
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
            this->hosted = vector<int>();
            this->bidded = vector<int>();
            this->uid_pathname = uid_pathname;
            this->pass_pathname = pass_pathname;
            this->login_pathname = login_pathname;
            this->hosted_pathname = hosted_pathname;
            this->bids_pathname = bids_pathname;
        }

        string getUID() {
            return this->UID;
        }
        string getPassword() {
            return this->password;
        }
        bool is_logged_in() {
            return logged_in;
        }
        bool is_logged_out() {
            return !logged_in;
        }
        bool is_unregistered() {
            return unregistered;
        }
        string get_uid_pathname() {
            return this->uid_pathname;
        }
        string get_pass_pathname() {
            return this->pass_pathname;
        }
        string get_login_pathname() {
            return this->login_pathname;
        }
        string get_hosted_pathname() {
            return this->hosted_pathname;
        }
        string get_bids_pathname() {
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
        void add_hosted(int AID) {
            this->hosted.push_back(AID);
        }
        void add_bidded(int AID) {
            this->bidded.push_back(AID);
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
        void erase() {
            this->UID = "";
            this->password = "";
            this->logged_in = false;
            this->unregistered = false;
        }
};

class Users {

    public:

        vector<User> users;

        Users() {
            this->users = vector<User>();
        }

        void add_user(User user) {
            this->users.push_back(user);
        }

        void remove_user(User user) {
            for(size_t i = 0; i < this->users.size(); i++) {
                if(this->users[i].getUID() == user.getUID()) {
                    this->users.erase(this->users.begin() + i);
                    break;
                }
            }
        }

        bool user_exists(string UID) {
            for(size_t i = 0; i < this->users.size(); i++) {
                if(this->users[i].getUID() == UID) {
                    return true;
                }
            }
            return false;
        }

        bool user_exists(string UID, string password) {
            for(size_t i = 0; i < this->users.size(); i++) {
                if(this->users[i].getUID() == UID && this->users[i].getPassword() == password) {
                    return true;
                }
            }
            return false;
        }

        User* get_user(string UID) {
            for(size_t i = 0; i < this->users.size(); i++) {
                if(this->users[i].getUID() == UID) {
                    return &this->users[i];
                }
            }
            return nullptr;
        }

};

/* aux.c file functions */
void check(bool condition);
void safe_stop(int signal);
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
ssize_t create_pass_file(User &user);
ssize_t create_login_file(User &user);

/* user.cpp file functions */
int tcp_message(char *asip, char *port, string message);
int udp_message(char *asip, char *port, string message);
vector<string> string_analysis(char* str, User &user);

/* as.cpp file functions */
int tcp_server(Users &users);
int udp_server(Users &users);
string vector_analysis(vector<string> message, Users &users);
string login(string UID, string password, Users &users);
string logout(string UID, string password, Users &users);

#endif // AUX_H
