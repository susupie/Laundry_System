#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include "db_conn.h"
#include <string>
#include <vector>
#include <memory>

// Updated Struct to match Database columns
struct Machine {
    int id;
    std::string name;
    std::string location;
    int weight;      // From DB
    double price;    // From DB
};

class OrderService {
public:
    OrderService(std::shared_ptr<DBConnection> db);

    int getUserIdByEmail(std::string email);
    double getUserBalance(int userId);

    // Returns list of machines with their specific Price and Weight
    std::vector<Machine> getAvailableMachines(std::string type);

    int startWash(int userId, int machineId, double price);
    void completeWash(int machineId, int orderId);
    bool topUpBalance(int userId, double amount);
    void showUserOrders(int userId);
    void showTransactionHistory(int userId);

private:
    std::shared_ptr<DBConnection> db_connection;
    void recordTransaction(int userId, std::string type, double amount);
};

#endif