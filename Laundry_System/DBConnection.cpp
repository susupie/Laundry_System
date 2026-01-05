#include "db_conn.h"
#include <iostream> // For std::cout, std::cerr

DBConnection::DBConnection(const std::string& host, const std::string& user,
    const std::string& password, const std::string& schema,
    unsigned int port)
    : driver(nullptr), con(nullptr),
    db_host(host), db_user(user), db_password(password),
    db_schema(schema), db_port(port) {
}

DBConnection::~DBConnection() {
    disconnect();
}

bool DBConnection::connect() {
    if (con != nullptr && con->isValid()) {
        // std::cout << "Already connected to MySQL." << std::endl; 
        return true;
    }

    try {
        // std::cout << "Getting MySQL driver instance..." << std::endl;
        driver = sql::mysql::get_mysql_driver_instance();
        if (!driver) {
            throw std::runtime_error("Failed to get MySQL driver instance");
        }

        std::string connection_url = "tcp://" + db_host + ":" + std::to_string(db_port);
        // std::cout << "Connecting to: " << connection_url << " as user " << db_user << "..." << std::endl;

        con = driver->connect(connection_url, db_user, db_password);
        // std::cout << "Connection established." << std::endl;

        // std::cout << "Setting schema to: " << db_schema << "..." << std::endl;
        con->setSchema(db_schema);
        // std::cout << "Schema set." << std::endl;

        return true;
    }
    catch (sql::SQLException& e) {
        logError("MySQL SQLException during connect", &e);
        disconnect();
        return false;
    }
    catch (std::runtime_error& e) {
        logError("Runtime error during connect: " + std::string(e.what()));
        disconnect();
        return false;
    }
    catch (std::exception& e) {
        logError("Standard exception during connect: " + std::string(e.what()));
        disconnect();
        return false;
    }
}

void DBConnection::disconnect() {
    if (con) {
        // std::cout << "Disconnecting from MySQL..." << std::endl;
        delete con;
        con = nullptr;
    }
    driver = nullptr;
}

bool DBConnection::isConnected() const {
    return con != nullptr && con->isValid();
}

int DBConnection::executeUpdate(const std::string& sql) {
    if (!isConnected()) {
        logError("Not connected to database. Cannot executeUpdate.");
        throw std::runtime_error("Database not connected.");
    }
    sql::Statement* stmt = nullptr;
    int rows_affected = 0;
    try {
        stmt = con->createStatement();
        if (!stmt) {
            throw std::runtime_error("Failed to create statement for executeUpdate.");
        }
        // std::cout << "Executing update: " << sql << std::endl; // SILENCED
        rows_affected = stmt->executeUpdate(sql);
        delete stmt;
    }
    catch (sql::SQLException& e) {
        logError("MySQL SQLException during executeUpdate", &e);
        if (stmt) delete stmt;
        throw;
    }
    catch (std::runtime_error& e) {
        logError("Runtime error during executeUpdate: " + std::string(e.what()));
        if (stmt) delete stmt;
        throw;
    }
    return rows_affected;
}

sql::ResultSet* DBConnection::executeQuery(const std::string& sql) {
    if (!isConnected()) {
        logError("Not connected to database. Cannot executeQuery.");
        throw std::runtime_error("Database not connected.");
    }
    sql::Statement* stmt = nullptr;
    sql::ResultSet* res = nullptr;
    try {
        stmt = con->createStatement();
        if (!stmt) {
            throw std::runtime_error("Failed to create statement for executeQuery.");
        }
        // std::cout << "Executing query: " << sql << std::endl; // SILENCED
        res = stmt->executeQuery(sql);
        delete stmt;
        return res;
    }
    catch (sql::SQLException& e) {
        logError("MySQL SQLException during executeQuery", &e);
        if (stmt) delete stmt;
        if (res) delete res;
        throw;
    }
    catch (std::runtime_error& e) {
        logError("Runtime error during executeQuery: " + std::string(e.what()));
        if (stmt) delete stmt;
        if (res) delete res;
        throw;
    }
}

sql::PreparedStatement* DBConnection::prepareStatement(const std::string& sql) {
    if (!isConnected()) {
        logError("Not connected to database. Cannot prepareStatement.");
        throw std::runtime_error("Database not connected.");
    }
    sql::PreparedStatement* pstmt = nullptr;
    try {
        // std::cout << "Preparing statement: " << sql << std::endl; // SILENCED
        pstmt = con->prepareStatement(sql);
        return pstmt;
    }
    catch (sql::SQLException& e) {
        logError("MySQL SQLException during prepareStatement", &e);
        if (pstmt) delete pstmt;
        throw;
    }
    catch (std::runtime_error& e) {
        logError("Runtime error during prepareStatement: " + std::string(e.what()));
        if (pstmt) delete pstmt;
        throw;
    }
}

void DBConnection::logError(const std::string& message, const sql::SQLException* e) const {
    // We KEEP cerr because these are actual errors/crashes, not just info
    std::cerr << "DBConnection ERROR: " << message << std::endl;
    if (e) {
        std::cerr << "  SQLState: " << e->getSQLState() << std::endl;
        std::cerr << "  Error Code: " << e->getErrorCode() << std::endl;
        std::cerr << "  Details: " << e->what() << std::endl;
    }
}