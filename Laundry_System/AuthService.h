#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <string>
#include <memory>
#include "db_conn.h"

class AuthService {
public:
    AuthService(std::shared_ptr<DBConnection> db_conn);

    // Removed initializeUserTable()

    bool registerUser(const std::string& username, const std::string& email, const std::string& password);
    bool loginUser(const std::string& identifier, const std::string& password);

private:
    std::shared_ptr<DBConnection> db_connection;
    std::string hashPassword(const std::string& password) const;
    bool emailExists(const std::string& email) const;
    bool usernameExists(const std::string& username) const;
    bool isValidEmail(const std::string& email) const;
};

#endif