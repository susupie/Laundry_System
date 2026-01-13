#include "OrderService.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace std;

OrderService::OrderService(shared_ptr<DBConnection> db) : db_connection(db) {}

int OrderService::getUserIdByEmail(string email) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT user_id FROM users WHERE email = ?"));
        pstmt->setString(1, email);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res && res->next()) return res->getInt("user_id");
    }
    catch (...) {}
    return -1;
}

double OrderService::getUserBalance(int userId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT balance FROM users WHERE user_id = ?"));
        pstmt->setInt(1, userId);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res && res->next()) return res->getDouble("balance");
    }
    catch (...) {}
    return 0.0;
}

void OrderService::recordTransaction(int userId, string type, double amount) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("INSERT INTO credit_transactions (user_id, type, amount, date) VALUES (?, ?, ?, NOW())"));
        pstmt->setInt(1, userId);
        pstmt->setString(2, type);
        pstmt->setDouble(3, amount);
        pstmt->executeUpdate();
    }
    catch (...) {}
}

bool OrderService::topUpBalance(int userId, double amount) {
    if (amount <= 0) return false;
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("UPDATE users SET balance = balance + ? WHERE user_id = ?"));
        pstmt->setDouble(1, amount);
        pstmt->setInt(2, userId);
        if (pstmt->executeUpdate() > 0) {
            recordTransaction(userId, "TopUp", amount);
            return true;
        }
    }
    catch (...) {}
    return false;
}

int OrderService::startWash(int userId, int machineId, double price) {
    try {
        if (getUserBalance(userId) < price) return -1;

        unique_ptr<sql::PreparedStatement> pstmtUser(db_connection->prepareStatement("UPDATE users SET balance = balance - ? WHERE user_id = ?"));
        pstmtUser->setDouble(1, price);
        pstmtUser->setInt(2, userId);
        pstmtUser->executeUpdate();

        recordTransaction(userId, "Payment", price);

        unique_ptr<sql::PreparedStatement> pstmtMachine(db_connection->prepareStatement("UPDATE machines SET status = 'In Use' WHERE machine_id = ?"));
        pstmtMachine->setInt(1, machineId);
        pstmtMachine->executeUpdate();

        unique_ptr<sql::PreparedStatement> pstmtOrder(db_connection->prepareStatement("INSERT INTO orders (user_id, machine_id, cost, order_status, start_time) VALUES (?, ?, ?, 'In Progress', NOW())"));
        pstmtOrder->setInt(1, userId);
        pstmtOrder->setInt(2, machineId);
        pstmtOrder->setDouble(3, price);
        pstmtOrder->executeUpdate();

        unique_ptr<sql::PreparedStatement> pstmtGetId(db_connection->prepareStatement("SELECT LAST_INSERT_ID() AS last_id"));
        unique_ptr<sql::ResultSet> res(pstmtGetId->executeQuery());
        if (res && res->next()) return res->getInt("last_id");
    }
    catch (...) {}
    return -1;
}

void OrderService::completeWash(int machineId, int orderId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmtM(db_connection->prepareStatement("UPDATE machines SET status = 'Idle' WHERE machine_id = ?"));
        pstmtM->setInt(1, machineId);
        pstmtM->executeUpdate();

        unique_ptr<sql::PreparedStatement> pstmtO(db_connection->prepareStatement("UPDATE orders SET order_status = 'Completed', end_time = NOW() WHERE order_id = ?"));
        pstmtO->setInt(1, orderId);
        pstmtO->executeUpdate();
    }
    catch (...) {}
}

// --- UPDATED: Get Available Machines with Price/Weight ---
vector<Machine> OrderService::getAvailableMachines(string type) {
    vector<Machine> machines;
    try {
        // Now fetching weight_capacity and price_per_cycle
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("SELECT machine_id, machine_name, location, weight_capacity, price_per_cycle FROM machines WHERE type = ? AND status = 'Idle'")
        );
        pstmt->setString(1, type);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res && res->next()) {
            Machine m;
            m.id = res->getInt("machine_id");
            m.name = res->getString("machine_name");
            m.location = res->getString("location");
            // Store DB values in struct
            m.weight = res->getInt("weight_capacity");
            m.price = res->getDouble("price_per_cycle");
            machines.push_back(m);
        }
    }
    catch (const exception& e) {
        cerr << "Error fetching machines: " << e.what() << endl;
    }
    return machines;
}

void OrderService::showUserOrders(int userId) {
    try {
        string sql = "SELECT o.order_id, m.machine_name, o.order_status, o.start_time, o.end_time, o.cost "
            "FROM orders o JOIN machines m ON o.machine_id = m.machine_id "
            "WHERE o.user_id = ? ORDER BY o.start_time DESC LIMIT 10";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        pstmt->setInt(1, userId);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        cout << "\n========================================================================================\n";
        cout << "                             MY ORDERS " << endl;
        cout << "========================================================================================\n";
        cout << left << setw(10) << "ID" << setw(15) << "Machine" << setw(15) << "Status" << setw(10) << "Cost" << "Start Time" << setw(20) 
            << "             End Time" << endl;
        cout << "========================================================================================\n";
        bool found = false;
        while (res && res->next()) {
            found = true;
            cout << left << setw(10) << res->getInt("order_id")
                << setw(15) << res->getString("machine_name")
                << setw(15) << res->getString("order_status")
                << "RM" << setw(9) << res->getDouble("cost")
                << res->getString("start_time") 
                << "   " << res->getString("end_time") << endl;
        }
        if (!found) cout << "No orders found." << endl;
        cout << "----------------------------------------------------------------------------------------" << endl;
    }
    catch (...) {}
}

void OrderService::showTransactionHistory(int userId) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement("SELECT type, amount, date FROM credit_transactions WHERE user_id = ? ORDER BY date DESC"));
        pstmt->setInt(1, userId);
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        cout << "\n================================================================\n";
        cout << "                       TRANSACTION HISTORY " << endl;
        cout << "================================================================\n";
        cout << left << setw(20) << "Date" << setw(15) << "Type" << setw(10) << "Amount" << endl;
        cout << "-----------------------------------------------------------------" << endl;
        while (res && res->next()) {
            string type = res->getString("type");
            double amount = res->getDouble("amount");
            string date = res->getString("date");
            string displayAmount = (type == "TopUp" ? "+ RM" : "- RM") + to_string(amount).substr(0, to_string(amount).find(".") + 3);
            cout << left << setw(20) << date << setw(15) << type << setw(10) << displayAmount << endl;
        }
        cout << "-----------------------------------------------------------------" << endl;
    }
    catch (...) {}
}