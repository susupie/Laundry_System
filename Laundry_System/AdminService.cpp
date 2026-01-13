#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <regex> // Added for Email Validation

using namespace std;

AdminService::AdminService(shared_ptr<DBConnection> db) : db_connection(db) {}

// Helper
bool AdminService::isValidEmail(const string& email) {
    const regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return regex_match(email, pattern);
}

// --- CUSTOMERS (With Validation) ---
void AdminService::addCustomer(string u, string e, string p) {
    if (u.empty() || e.empty() || p.empty()) {
        cout << "[ERROR] Fields cannot be empty." << endl; return;
    }
    if (!isValidEmail(e)) {
        cout << "[ERROR] Invalid Email Format." << endl; return;
    }

    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("INSERT INTO users (username, email, password, role) VALUES (?, ?, HEX(?), 'customer')"));
        pstmt->setString(1, u); pstmt->setString(2, e); pstmt->setString(3, p);
        pstmt->executeUpdate(); cout << "[SUCCESS] Customer added." << endl;
    }
    catch (const exception& ex) { cout << "[ERROR] Username/Email already exists." << endl; }
}

void AdminService::listCustomers() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT user_id, username, email, balance FROM users WHERE role = 'customer' ORDER BY user_id ASC"));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        cout << "\n================================================================\n";
        cout << "                          CUSTOMER LIST " << endl;
        cout << "================================================================\n";
        cout << left << setw(5) << "ID" << setw(15) << "Username" << setw(25) << "Email" << "Balance" << endl;
        cout << "-----------------------------------------------------------------" << endl;
        while (res && res->next()) {
            cout << left << setw(5) << res->getInt("user_id") << setw(15) << res->getString("username") << setw(25) << res->getString("email") << "RM " << fixed << setprecision(2) << res->getDouble("balance") << endl;
        }
    }
    catch (...) {}
}

void AdminService::deleteCustomer(int id) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("DELETE FROM users WHERE user_id=? AND role='customer'"));
        pstmt->setInt(1, id); pstmt->executeUpdate(); cout << "[SUCCESS] Deleted." << endl;
    }
    catch (...) {}
}

// --- MACHINES (Spacing Fixed) ---
void AdminService::listMachines() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT * FROM machines ORDER BY type DESC, machine_name ASC"));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        cout << "\n============================================================================================================" << endl;
        cout << "                                      MACHINE LIST" << endl;
        cout << "============================================================================================================" << endl;
        // Adjusted widths to prevent overlapping
        cout << left << setw(5) << "ID" << setw(18) << "Name" << setw(10) << "Price(RM)" << setw(10)  << "Weight(Kg)" << setw(10) << "Type" << setw(25) << "Location" << setw(18) << "Status" << "Note" << endl;
        cout << "------------------------------------------------------------------------------------------------------------" << endl;
        while (res && res->next()) {
            string note = res->getString("maintenance_note");
            if (note.empty()) note = "-";
            cout << left << setw(5) << res->getInt("machine_id")
                << setw(18) << res->getString("machine_name")
                << setw(10) << res->getString("type")
                << setw(10) << res->getString("price_per_cycle")
                << setw(10) << res->getString("weight_capacity")
                << setw(25) << res->getString("location") 
                << setw(18) << res->getString("status")
                << note << endl;
        }
        cout << "------------------------------------------------------------------------------------------------------------" << endl;
    }
    catch (...) {}
}

void AdminService::addMachine(string name, string type, int weight, double price, string location) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("INSERT INTO machines (machine_name, type, weight_capacity, price_per_cycle, status, location) VALUES (?, ?, ?, ?, 'Idle', ?)"));
        pstmt->setString(1, name); pstmt->setString(2, type); pstmt->setInt(3, weight); pstmt->setDouble(4, price); pstmt->setString(5, location);
        pstmt->executeUpdate(); cout << "[SUCCESS] Machine added." << endl;
    }
    catch (...) {}
}

bool AdminService::updateMachineStatus(int id, int s, string n) {
    string stat = (s == 1) ? "Idle" : "Out of Service";
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("UPDATE machines SET status=?, maintenance_note=? WHERE machine_id=?"));
        p->setString(1, stat); p->setString(2, n); p->setInt(3, id);
        return p->executeUpdate() > 0;
    }
    catch (...) { return false; }
}

void AdminService::deleteMachine(int id) {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("DELETE FROM machines WHERE machine_id=?"));
        p->setInt(1, id); p->executeUpdate(); cout << "[SUCCESS] Deleted." << endl;
    }
    catch (...) {}
}

// --- ORDERS ---
void AdminService::listAllOrders() {
    try {
        string sql = "SELECT o.order_id, u.username, m.machine_name, o.order_status, o.cost, o.start_time FROM orders o JOIN users u ON o.user_id = u.user_id JOIN machines m ON o.machine_id = m.machine_id ORDER BY o.start_time DESC LIMIT 20";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        cout << "\n====================================================================================" << endl;
        cout << "\                           RECENT ORDERS" << endl;
        cout << "====================================================================================" << endl;
        cout << left << setw(5) << "ID" << setw(22) << "Date" << setw(15) << "User" << setw(15) << "Machine" << setw(15) << "Status" << "Cost" << endl;
        cout << "------------------------------------------------------------------------------------" << endl;
        while (res && res->next()) {
            cout << left << setw(5) << res->getInt("order_id")
                << setw(22) << res->getString("start_time")
                << setw(15) << res->getString("username")
                << setw(15) << res->getString("machine_name")
                << setw(15) << res->getString("order_status")
                << "RM " << res->getDouble("cost") << endl;
        }
    }
    catch (...) {}
}

void AdminService::deleteOrder(int id) {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("DELETE FROM orders WHERE order_id=?"));
        p->setInt(1, id); p->executeUpdate(); cout << "[SUCCESS] Deleted." << endl;
    }
    catch (...) {}
}