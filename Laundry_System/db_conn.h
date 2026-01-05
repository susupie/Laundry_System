#pragma once

#include <string>
#include <stdexcept> // For std::runtime_error

// Include the necessary headers for the traditional C++ Connector API
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h> // Include if using prepared statements

// Forward declarations to avoid including iostream in the header unless strictly necessary
// (Though for logging/debugging, it often ends up there anyway)
// class std::string; // Already included via <string>
// class std::runtime_error; // Already included via <stdexcept>

// It's generally good practice to avoid 'using namespace std;' in header files.
// Be explicit with std::, sql::mysql::, etc.

class DBConnection {
public:
    // Constructor: Takes connection parameters
    DBConnection(const std::string& host, const std::string& user,
        const std::string& password, const std::string& schema,
        unsigned int port = 3306);

    // Destructor: Cleans up database resources
    ~DBConnection();

    // Establishes the connection to the database
    bool connect();

    // Disconnects from the database
    void disconnect();

    // Executes a non-query SQL statement (INSERT, UPDATE, DELETE, DDL)
    // Returns the number of rows affected
    int executeUpdate(const std::string& sql);

    // Executes a query SQL statement (SELECT)
    // Returns a pointer to a sql::ResultSet (caller is responsible for deleting)
    sql::ResultSet* executeQuery(const std::string& sql);

    // Prepares a statement for execution (e.g., for INSERT/UPDATE with parameters)
    // Returns a pointer to a sql::PreparedStatement (caller is responsible for deleting)
    sql::PreparedStatement* prepareStatement(const std::string& sql);

    // Check if the connection is active
    bool isConnected() const;

    // Optional: Get the underlying connection object (use with caution)
    sql::Connection* getConnection() const { return con; }

private:
    sql::mysql::MySQL_Driver* driver;
    sql::Connection* con;

    // Connection parameters
    const std::string db_host;
    const std::string db_user;
    const std::string db_password;
    const std::string db_schema;
    const unsigned int db_port;

    // Private helper for error logging (optional, can just use cerr directly)
    void logError(const std::string& message, const sql::SQLException* e = nullptr) const;

    // Prevent copy construction and assignment (connections are typically non-copyable)
    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;
};