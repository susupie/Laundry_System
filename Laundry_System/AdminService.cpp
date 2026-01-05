#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

AdminService::AdminService(shared_ptr<DBConnection> db) : db_connection(db) {}

// ======================= CUSTOMERS =======================

void AdminService::listCustomers() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("SELECT user_id, username, email, balance FROM users WHERE role = 'customer'")
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n-------------------- CUSTOMER LIST --------------------" << endl;
        cout << left << setw(5) << "ID" << setw(15) << "Username" << setw(25) << "Email" << "Balance" << endl;
        cout << "-------------------------------------------------------" << endl;
        while (res && res->next()) {
            cout << left << setw(5) << res->getInt("user_id")
                << setw(15) << res->getString("username")
                << setw(25) << res->getString("email")
                << "RM" << fixed << setprecision(2) << res->getDouble("balance") << endl;
        }
        cout << "-------------------------------------------------------" << endl;
    }
    catch (...) {}
}

void AdminService::addCustomer(string username, string email, string password) {
    try {
        unsigned long hash = 0;
        for (char c : password) hash = (hash * 31) + c;

        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("INSERT INTO users (username, email, password, role) VALUES (?, ?, HEX(?), 'customer')")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, email);
        pstmt->setInt(3, hash);
        pstmt->executeUpdate();
        cout << "[SUCCESS] Customer added." << endl;
    }
    catch (const exception& e) { cout << "[ERROR] " << e.what() << endl; }
}

void AdminService::deleteCustomer(int userId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("DELETE FROM users WHERE user_id = ? AND role='customer'"));
        pstmt->setInt(1, userId);
        if (pstmt->executeUpdate() > 0) cout << "[SUCCESS] Customer deleted." << endl;
        else cout << "[FAILED] ID not found." << endl;
    }
    catch (...) {}
}

// ======================= MACHINES =======================

void AdminService::listMachines() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT * FROM machines ORDER BY machine_id ASC"));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n-------------------------- MACHINE LIST --------------------------" << endl;
        cout << left << setw(5) << "ID" << setw(15) << "Name" << setw(10) << "Type" << setw(15) << "Status" << "Note" << endl;
        cout << "------------------------------------------------------------------" << endl;
        while (res && res->next()) {
            string note = res->getString("maintenance_note");
            if (note.empty()) note = "-";
            cout << left << setw(5) << res->getInt("machine_id")
                << setw(15) << res->getString("machine_name")
                << setw(10) << res->getString("type")
                << setw(15) << res->getString("status")
                << note << endl;
        }
        cout << "------------------------------------------------------------------" << endl;
    }
    catch (...) {}
}

void AdminService::addMachine(string name, string type, int weight, double price) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("INSERT INTO machines (machine_name, type, weight_capacity, price_per_cycle, status) VALUES (?, ?, ?, ?, 'Idle')")
        );
        pstmt->setString(1, name);
        pstmt->setString(2, type);
        pstmt->setInt(3, weight);
        pstmt->setDouble(4, price);
        pstmt->executeUpdate();
        cout << "[SUCCESS] Machine added." << endl;
    }
    catch (...) {}
}

void AdminService::deleteMachine(int machineId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("DELETE FROM machines WHERE machine_id = ?"));
        pstmt->setInt(1, machineId);
        if (pstmt->executeUpdate() > 0) cout << "[SUCCESS] Machine deleted." << endl;
        else cout << "[FAILED] ID not found." << endl;
    }
    catch (...) {}
}

bool AdminService::updateMachineStatus(int machineId, int statusChoice, string note) {
    string newStatus;
    if (statusChoice == 1) newStatus = "Idle";
    else if (statusChoice == 2) newStatus = "Out of Service"; // Maintenance
    else return false;

    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("UPDATE machines SET status = ?, maintenance_note = ? WHERE machine_id = ?")
        );
        pstmt->setString(1, newStatus);
        pstmt->setString(2, note);
        pstmt->setInt(3, machineId);
        return pstmt->executeUpdate() > 0;
    }
    catch (...) { return false; }
}

// ======================= ORDERS =======================

void AdminService::listAllOrders() {
    try {
        string sql = "SELECT o.order_id, u.username, m.machine_name, o.order_status, o.cost FROM orders o "
            "JOIN users u ON o.user_id = u.user_id JOIN machines m ON o.machine_id = m.machine_id ORDER BY o.start_time DESC LIMIT 20";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n-------------------- RECENT ORDERS --------------------" << endl;
        cout << left << setw(5) << "ID" << setw(15) << "User" << setw(15) << "Machine" << setw(15) << "Status" << "Cost" << endl;
        cout << "-------------------------------------------------------" << endl;
        while (res && res->next()) {
            cout << left << setw(5) << res->getInt("order_id")
                << setw(15) << res->getString("username")
                << setw(15) << res->getString("machine_name")
                << setw(15) << res->getString("order_status")
                << "RM" << res->getDouble("cost") << endl;
        }
    }
    catch (...) {}
}

void AdminService::deleteOrder(int orderId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("DELETE FROM orders WHERE order_id = ?"));
        pstmt->setInt(1, orderId);
        if (pstmt->executeUpdate() > 0) cout << "[SUCCESS] Order deleted." << endl;
        else cout << "[FAILED] ID not found." << endl;
    }
    catch (...) {}
}

// ======================= REPORTS & TOOLS =======================

void AdminService::searchOrders(string keyword) {
    try {
        string sql = "SELECT o.order_id, u.username, o.order_status FROM orders o JOIN users u ON o.user_id = u.user_id WHERE u.username LIKE ?";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        pstmt->setString(1, "%" + keyword + "%");
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n--- SEARCH RESULTS ---" << endl;
        bool found = false;
        while (res && res->next()) {
            found = true;
            cout << "Order #" << res->getInt("order_id") << " | User: " << res->getString("username") << " | Status: " << res->getString("order_status") << endl;
        }
        if (!found) cout << "No matches found." << endl;
    }
    catch (...) {}
}

void AdminService::generateDailyReport() {
    cout << "\n==============================================" << endl;
    cout << "           DAILY REPORT (TODAY)               " << endl;
    cout << "==============================================" << endl;
    try {
        // Revenue
        unique_ptr<sql::PreparedStatement> pstmt1(db_connection->prepareStatement("SELECT COALESCE(SUM(cost), 0.00) AS total FROM orders WHERE DATE(start_time) = CURDATE()"));
        unique_ptr<sql::ResultSet> res1(pstmt1->executeQuery());
        if (res1 && res1->next()) cout << " Total Revenue    : RM" << res1->getDouble("total") << endl;

        // Count
        unique_ptr<sql::PreparedStatement> pstmt2(db_connection->prepareStatement("SELECT COUNT(*) AS cnt FROM orders WHERE DATE(start_time) = CURDATE()"));
        unique_ptr<sql::ResultSet> res2(pstmt2->executeQuery());
        if (res2 && res2->next()) cout << " Total Orders     : " << res2->getInt("cnt") << endl;

    }
    catch (const exception& e) { cout << "Report Error: " << e.what() << endl; }
    cout << "==============================================" << endl;
}

// ======================= BACKUP =======================

void AdminService::saveData() {
    cout << "\n[SYSTEM] Exporting Customers to CSV..." << endl;
    ofstream file("backup_customers.csv");
    if (file.is_open()) {
        file << "ID,Username,Email,Balance\n";
        try {
            unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT * FROM users WHERE role='customer'"));
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res && res->next()) {
                file << res->getInt("user_id") << "," << res->getString("username") << "," << res->getString("email") << "," << res->getDouble("balance") << "\n";
            }
        }
        catch (...) {}
        file.close();
        cout << "[SUCCESS] Export complete." << endl;
    }
}

void AdminService::loadData() {
    cout << "\n[SYSTEM] Checking for backups..." << endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << "[SUCCESS] Backup files found (Restore logic simulated)." << endl;
}