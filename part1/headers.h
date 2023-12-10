#ifndef AUX_H
#define AUX_H

#include <iostream>
#include <iomanip>

using namespace std;

class Bid {
    public:
        int bid;
        string bidder_UID;
        int bid_date_time;

        Bid(int bid, string bidder_UID) {
            this->bid = bid;
            this->bidder_UID = bidder_UID;
            this->bid_date_time = 0;
            // TODO: GET DATE AND TIME

        }

        int get_bid() const {
            return this->bid;
        }
        string get_bidder_UID() const {
            return this->bidder_UID;
        }
};

class Auction {
    public:
        
        string AID;
        string name;
        string asset_fname;
        int status;
        int start_value;
        int time_active;
        string hosted_by;
        vector<Bid> bids;
        string auction_pathname;
        string bids_pathname;
        string start_aid_pathname;
        string end_aid_pathname;
        string image_pathname;
        long int start_time_1970;

        Auction(string AID, string name, string asset_fname, int start_value, int time_active, string hosted_by, string auction_pathname, string bids_pathname, string start_aid_pathname, string end_aid_pathname, string image_pathname) {
            this->AID = AID;
            this->name = name;
            this->status = 1;
            this->asset_fname = asset_fname;
            this->start_value = start_value;
            this->time_active = time_active;
            this->hosted_by = hosted_by;
            this->auction_pathname = auction_pathname;
            this->bids_pathname = bids_pathname;
            this->start_aid_pathname = start_aid_pathname;
            this->end_aid_pathname = end_aid_pathname;
            this->image_pathname = image_pathname;
            this->start_time_1970 = 0;
            this->bids = vector<Bid>();
        }

        void set_start_time_1970(long int start_time_1970) {
            this->start_time_1970 = start_time_1970;
        }

        void set_status(int status) {
            this->status = status;
        }

        string get_AID() const {
            return this->AID;
        }

        string get_name() const {
            return this->name;
        }

        int get_status() const {
            return this->status;
        }

        string get_asset_fname() const {
            return this->asset_fname;
        }

        int get_start_value() const {
            return this->start_value;
        }

        int get_time_active() const {
            return this->time_active;
        }

        string get_hosted_by() const {
            return this->hosted_by;
        }

        vector<Bid>* get_bids() {
            return &(this->bids);
        }

        string get_auction_pathname() const {
            return this->auction_pathname;
        }
        string get_bids_pathname() const {
            return this->bids_pathname;
        }
        string get_start_aid_pathname() const {
            return this->start_aid_pathname;
        }
        string get_end_aid_pathname() const {
            return this->end_aid_pathname;
        }
        string get_image_pathname() const {
            return this->image_pathname;
        }
        long int get_start_time_1970() const {
            return this->start_time_1970;
        }
};

class Auctions {
    public:
        vector<Auction> auctions;
        int* AID_counter;

        Auctions(int* sharedCounter) {
            this->auctions = vector<Auction>();
            this->AID_counter = sharedCounter;
        }

        Auction* get_auction(string AID) {
            for(size_t i = 0; i < this->auctions.size(); i++) {
                if(this->auctions[i].get_AID() == AID) {
                    return &this->auctions[i];
                }
            }
            return nullptr;
        }

        vector<Auction>* get_all_auctions() {
            return &(this->auctions);
        }

        ssize_t add_auction(string name, string asset_fname, int start_value, int time_active, string hosted_by) {
            if (*AID_counter > 999) {
                return -1;
            }
            string AID_string = intToThreeDigitString(*AID_counter);
            string auction_pathname = "AUCTIONS/" + AID_string;
            string bids_pathname = auction_pathname + "/" + "BIDS";
            string start_aid_pathname = auction_pathname + "/" + "START_" + AID_string + ".txt";
            string end_aid_pathname = auction_pathname + "/" + "END_" + AID_string + ".txt";
            string image_pathname = auction_pathname + "/" + asset_fname;

            Auction auction(AID_string, name, asset_fname, start_value, time_active, hosted_by, auction_pathname, bids_pathname, start_aid_pathname, end_aid_pathname, image_pathname);
            this->auctions.push_back(auction);
            *AID_counter += 1;
            return (*AID_counter-1);
        }
        
        void remove_auction(Auction auction) {
            for(size_t i = 0; i < this->auctions.size(); i++) {
                if(this->auctions[i].get_AID() == auction.get_AID()) {
                    this->auctions.erase(this->auctions.begin() + i);
                    break;
                }
            }
        }

        void remove_auction(string AID) {
            for(size_t i = 0; i < this->auctions.size(); i++) {
                if(this->auctions[i].get_AID() == AID) {
                    this->auctions.erase(this->auctions.begin() + i);
                    break;
                }
            }
        }

        string intToThreeDigitString(int number) {
            if (number > 999) {
                return "LIMIT EXCEEDED";
            }
            ostringstream oss;
            oss << setw(3) << setfill('0') << number;
            return oss.str();
        }
};

class User {
    public:
        
        string UID;
        string password;
        bool logged_in;
        bool unregistered;
        vector<Auction> hosted;
        vector<Auction> bidded;
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
            this->hosted = vector<Auction>();
            this->bidded = vector<Auction>();
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
        vector<Auction>* get_hosted() {
            return &(this->hosted);
        }
        vector<Auction>* get_bidded() {
            return &(this->bidded);
        }

        Auction* get_hosted(string AID) {
            for(size_t i = 0; i < this->hosted.size(); i++) {
                if(this->hosted[i].get_AID() == AID) {
                    return &this->hosted[i];
                }
            }
            return nullptr;
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
        void add_hosted(string AID, string name, string asset_fname, int start_value, int time_active, string hosted_by, string auction_pathname, string bids_pathname, string start_aid_pathname, string end_aid_pathname, string image_pathname) {
            this->hosted.push_back(Auction(AID, name, asset_fname, start_value, time_active, hosted_by, auction_pathname, bids_pathname, start_aid_pathname, end_aid_pathname, image_pathname));
        }
        void add_hosted(Auction &auction) { // TODO check if this can be &auction
            this->hosted.push_back(auction);
        }
        void add_bidded(string AID, string name, string asset_fname, int start_value, int time_active, string hosted_by, string auction_pathname, string bids_pathname, string start_aid_pathname, string end_aid_pathname, string image_pathname) {
            this->bidded.push_back(Auction(AID, name, asset_fname, start_value, time_active, hosted_by, auction_pathname, bids_pathname, start_aid_pathname, end_aid_pathname, image_pathname));
        }
        void add_bidded(Auction auction) {
            this->bidded.push_back(auction);
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
            for (auto& user : this->users) {
                if (user.getUID() == UID) {
                    return &user;
                }
            }
            return nullptr;
        }

        void print_all_users() {
        for (const User &user : users) {
            std::cout << "User Information:" << std::endl;
            std::cout << "UID: " << user.getUID() << std::endl;
            std::cout << "Password: " << user.getPassword() << std::endl;
            std::cout << "Logged In: " << (user.is_logged_in() ? "true" : "false") << std::endl;
            std::cout << "Unregistered: " << (user.is_unregistered() ? "true" : "false") << std::endl;
            std::cout << "UID Pathname: " << user.get_uid_pathname() << std::endl;
            std::cout << "Pass Pathname: " << user.get_pass_pathname() << std::endl;
            std::cout << "Login Pathname: " << user.get_login_pathname() << std::endl;
            std::cout << "Hosted Pathname: " << user.get_hosted_pathname() << std::endl;
            std::cout << "Bids Pathname: " << user.get_bids_pathname() << std::endl;

            std::cout << "Hosted Items: ";
            for (Auction hosted : user.hosted) {
                std::cout << hosted.get_AID() << " " << hosted.get_name() << " " <<hosted.get_status() << " " << hosted.get_asset_fname() << " " << hosted.get_start_value() << " " << hosted.get_time_active();
            }
            std::cout << std::endl;

            std::cout << "Bidded Items: ";
            for (Auction bidded : user.bidded) {
                std::cout << bidded.get_AID() << " " << bidded.get_name() <<" " << bidded.get_status() << " " << bidded.get_asset_fname() << " " << bidded.get_start_value() << " " << bidded.get_time_active();
            }
            std::cout << "-------------------------------------" << std::endl;
        }
    }

};

/* aux.c file functions */
void check(bool condition, string message);
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
ssize_t create_start_aid_file(Auction &auction);
ssize_t create_hosted_file(User &user, Auction &auction);
ssize_t create_end_aid_file(Auction &auction);
ssize_t delete_login_file(User &user);
ssize_t delete_pass_file(User &user);
void load_users(Users &users);
string intToThreeDigitString(int number);
int check_file_size(const char *fname);
void monitorAuctionEnd(Auction& auction);

/* user.cpp file functions */
int tcp_message(char *asip, char *port, string message);
int udp_message(char *asip, char *port, string message);
vector<string> string_analysis(char* str, User &user);

/* as.cpp file functions */
int tcp_server(Users &users, Auctions &auctions);
int udp_server(Users &users, Auctions &auctions);
string vector_analysis(vector<string> message, Users &users, Auctions &auctions);
string login(string UID, string password, Users &users);
string logout(string UID, string password, Users &users);
string unregister(string UID, string password, Users &users);
string myauctions(string UID, Users &users);
string mybids(string UID, Users &users);
string list(Auctions &auctions);
string open(string UID, string password, string name, string start_value, string time_active, string fname, Auctions &auctions, Users &users);
string close(string UID, string password, string AID, Users &users, Auctions &auctions);

#endif // AUX_H
