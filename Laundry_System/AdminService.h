#pragma once
#include "db_conn.h"
#include <string>
#include <vector>
#include <memory>

class AdminService {
public:
    AdminService(std::shared_ptr<DBConnection> db);

    // --- 1. MANAGE CUSTOMERS (Implemented in AdminService_Records.cpp) ---
    void listCustomers();
    void addCustomer(std::string username, std::string email, std::string password);
    void deleteCustomer(int userId);

    // --- 2. MANAGE MACHINES (Implemented in AdminService_Records.cpp) ---
    void listMachines();
    void addMachine(std::string name, std::string type, int weight, double price);
    void deleteMachine(int machineId);
    bool updateMachineStatus(int machineId, int statusChoice, std::string note);

    // --- 3. MANAGE ORDERS (Implemented in AdminService_Records.cpp) ---
    void listAllOrders();
    void deleteOrder(int orderId);

    // --- 4. SEARCH (Implemented in AdminService_Reports.cpp) ---
    void searchOrders(std::string keyword);

    // --- 5. ADVANCED REPORTS (Implemented in AdminService_Reports.cpp) ---
    void report_MachineUsageTable();      // Table Style
    void report_FinancialSummary();       // Summary List
    void report_RevenueBarChart();        // Text Visual Chart
    void report_RevenueTrendAnalysis();   // Graph Summary

    // NEW: Save today's stats to the database permanently
    void generateDailyReport(); 

    void saveDailyReportToDB();
    void exportTransactionHistoryPDF(std::string customFilename);

    void viewSavedReports();

    void exportDailyReportPDF(std::string filename, std::string targetDate);

    // 2. Export Summary of ALL DATES (From 'reports' table)
    void exportAllHistoryPDF(std::string filename);

    void exportTransactionHistoryPDF(std::string customFilename);
    // NEW: View the history saved in the 'reports' table
    void exportTransactionHistoryPDF();

    // --- 6. BACKUP (Implemented in AdminService_Records.cpp) ---
    void saveData();
    void loadData();

private:
    std::shared_ptr<DBConnection> db_connection;

    void createRawPDF(std::string filename, std::string title, std::vector<std::string> lines);

    // Helper: Get Date string
    std::string getCurrentDate();
};