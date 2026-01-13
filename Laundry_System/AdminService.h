#pragma once
#include "db_conn.h"
#include <string>
#include <vector>
#include <memory>

class AdminService {
public:
    AdminService(std::shared_ptr<DBConnection> db);

    // RECORDS
    void listCustomers();
    void addCustomer(std::string username, std::string email, std::string password);
    void deleteCustomer(int userId);
    void listMachines();
    void addMachine(std::string name, std::string type, int weight, double price, std::string location);
    void deleteMachine(int machineId);
    bool updateMachineStatus(int machineId, int statusChoice, std::string note);
    void listAllOrders();
    void deleteOrder(int orderId);

    // REPORTS
    void report_MachineUsageTable();
    void report_RevenueBarChart();
    void report_RevenueTrendAnalysis();

    // NEW: Console Sales Summary (Like the Fruit Example)
    void report_SalesSummary();

    // PDF EXPORTS
    void exportMasterReportPDF(std::string filename);
    void exportDailyReportPDF(std::string filename, std::string targetDate);

    // HELPERS
    void saveDailyReportToDB();
    void viewSavedReports();
    //void searchOrders(std::string keyword);

    std::vector<std::string> getAvailableReportDates();

private:
    std::shared_ptr<DBConnection> db_connection;
    void createRawPDF(std::string filename, std::string title, std::vector<std::string> lines);
    std::string getCurrentDate();
    bool isValidEmail(const std::string& email);
};