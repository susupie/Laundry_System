#include "AuthService.h"
#include "MenuUser.h" 
#include "MenuAdmin.h"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <regex>

using namespace std;

AuthService::AuthService(shared_ptr<DBConnection> db_conn)
    : db_connection(db_conn) {
    if (!db_connection || !db_connection->isConnected()) {
        throw runtime_error("Connection Error.");
    }
}

// --- HELPERS ---
string AuthService::hashPassword(const string& password) const {
    unsigned long hash = 0;
    for (char c : password) {
        hash = (hash * 31) + c;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

bool AuthService::isValidEmail(const string& email) const {
    const regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return regex_match(email, pattern);
}

bool AuthService::emailExists(const string& email) const {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("SELECT COUNT(*) FROM users WHERE email = ?")
        );
        pstmt->setString(1, email);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res && res->next()) return res->getInt(1) > 0;
        return false;
    }
    catch (...) { return false; }
}

bool AuthService::usernameExists(const string& username) const {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("SELECT COUNT(*) FROM users WHERE username = ?")
        );
        pstmt->setString(1, username);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res && res->next()) return res->getInt(1) > 0;
        return false;
    }
    catch (...) { return false; }
}

// --- REGISTER ---
bool AuthService::registerUser(const string& username, const string& email, const string& password) {
    if (username.empty() || email.empty() || password.empty()) {
        cerr << "[ERROR] All fields are required." << endl;
        return false;
    }
    if (!isValidEmail(email)) {
        cerr << "[ERROR] Invalid email format." << endl;
        return false;
    }
    if (emailExists(email)) {
        cout << "[ERROR] Email already taken." << endl;
        return false;
    }
    if (usernameExists(username)) {
        cout << "[ERROR] Username already taken." << endl;
        return false;
    }

    string password_hash = hashPassword(password);

    unique_ptr<sql::PreparedStatement> pstmt(
        db_connection->prepareStatement("INSERT INTO users (username, email, password, role, balance) VALUES (?, ?, ?, 'customer', 0.00)")
    );

    try {
        pstmt->setString(1, username);
        pstmt->setString(2, email);
        pstmt->setString(3, password_hash);

        int rows = pstmt->executeUpdate();
        if (rows == 1) {
            cout << "[SUCCESS] Registration Successful!" << endl;
            return true;
        }
    }
    catch (const exception& e) {
        cerr << "[DB ERROR] " << e.what() << endl;
    }
    return false;
}

// --- LOGIN ---
bool AuthService::loginUser(const string& identifier, const string& password) {
    if (identifier.empty() || password.empty()) return false;

    string password_hash = hashPassword(password);

    unique_ptr<sql::PreparedStatement> pstmt(
        db_connection->prepareStatement("SELECT user_id, username, email, role FROM users WHERE (email = ? OR username = ?) AND password = ?")
    );

    try {
        pstmt->setString(1, identifier);
        pstmt->setString(2, identifier);
        pstmt->setString(3, password_hash);

        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res && res->next()) {
            string dbUsername = res->getString("username");
            string dbEmail = res->getString("email");
            string role = res->getString("role");

            cout << "[SUCCESS] Login successful! Welcome " << dbUsername << endl;

            if (role == "admin") {
                MenuAdmin(db_connection, dbUsername);
            }
            else {
                MenuUser(db_connection, dbEmail);
            }
            return true;
        }
        else {
            cout << "[FAILED] Invalid credentials." << endl;
            return false;
        }
    }
    catch (const exception& e) {
        cerr << "[ERROR] " << e.what() << endl;
        return false;
    }
}