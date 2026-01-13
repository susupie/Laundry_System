#include "db_conn.h"
#include "AuthService.h"
#include "InputValidation.h"
#include <iostream>
#include <memory>
#include <string>   
#include <cstdlib> 

using namespace std;

void runAuthMenu(AuthService& auth_service);

int main() {
    // 1. Silent Configuration
    const string host = "localhost";
    const string user = "root";
    const string password = "";
    const string schema = "laundry_db";
    const unsigned int port = 3306;

    shared_ptr<DBConnection> db_conn;

    try {
        // 2. Connect
        db_conn = make_shared<DBConnection>(host, user, password, schema, port);

        if (!db_conn->connect()) {
            cerr << "[FATAL] Database connection failed." << endl;
            return 1;
        }

        // 3. Init Service 
        AuthService auth_service(db_conn);

        // 4. Run App
        runAuthMenu(auth_service);

    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

void runAuthMenu(AuthService& auth_service) {
    string email, password, username, identifier;
    int choice;

    do {
        system("cls"); 

        cout << "================================================================\n";
        cout << "           WELCOME TO LAUNDRY SERVICE & CREDIT SYSTEM           \n";
        cout << "================================================================\n";
        cout << "                                                                \n";
        cout << "    1. Register New Account                                     \n";
        cout << "    2. Login to System                                          \n";
        cout << "    3. Exit Application                                         \n";
        cout << "                                                                \n";
        cout << "================================================================\n";

        cout << " Select Option > ";
        choice = getValidatedChoice(1, 3);

        switch (choice) {
        case 1:
            cout << "\n================================================================\n";
            cout << "                         REGISTRATION ";
            cout << "\n================================================================\n";
            cout << "\nEnter Email: ";
            getline(cin, email);
            cout << "Enter Username: ";
            getline(cin, username);
            password = getMaskedPassword("Enter Password: ");
            cout << "-----------------------------------------------------------------" << endl;
            auth_service.registerUser(username, email, password);

            cout << "\n(Press Enter to continue...)";
            cin.get();
            break;

        case 2:
            cout << "\n================================================================\n";
            cout << "                             LOGIN";
            cout << "\n================================================================\n";
            cout << "\nEnter Email OR Username: ";
            getline(cin, identifier);
            password = getMaskedPassword("Enter Password: ");
            cout << "-----------------------------------------------------------------" << endl;
            auth_service.loginUser(identifier, password);

            cout << "\n(Press Enter to continue...)";
            cin.get();
            break;

        case 3:
            cout << "\n================================================================" << endl;
            cout << "           THANK YOU AND HAVE A NICE DAY!                       " << endl;
            cout << "================================================================" << endl;
            break;
        }
    } while (choice != 3);
}