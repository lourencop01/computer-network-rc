#ifndef AUX_H
#define AUX_H

using namespace std;

class User {
    public:
        string UID;
        string password;
        User(string UID, string password) {
            this->UID = UID;
            this->password = password;
        }
        string getUID() {
            return this->UID;
        }
        string getPassword() {
            return this->password;
        }
        void setUID(string UID) {
            this->UID = UID;
        }
        void setPassword(string password) {
            this->password = password;
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
string vector_to_string(vector<string> message);
vector<string> string_analysis(char* str);

/* user.cpp file functions */
int tcp_message(char *asip, char *port, string message);
int udp_message(char *asip, char *port, string message);
vector<string> string_analysis(char* str);

#endif // AUX_H
