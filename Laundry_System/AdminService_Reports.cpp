#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

//// ... (Keep searchOrders / saveData / loadData as they were) ...
//void AdminService::searchOrders(string keyword) { /* ... */ }
//void AdminService::saveData() { /* ... */ }
//void AdminService::loadData() { /* ... */ }


// =================================================================================
//  1. MACHINE USAGE TABLE (Table Format)
// =================================================================================
void AdminService::report_MachineUsageTable() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement(
                "SELECT m.machine_name, COUNT(o.order_id) as Cycles, SUM(o.cost) as Revenue "
                "FROM orders o "
                "JOIN machines m ON o.machine_id = m.machine_id "
                "GROUP BY m.machine_name"
            )
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n1. Machine Performance Table:\n" << endl;

        // ASCII Border
        cout << "+----------------------+----------+---------------+" << endl;
        cout << "| Machine Name         | Cycles   | Total Revenue |" << endl;
        cout << "+----------------------+----------+---------------+" << endl;

        while (res && res->next()) {
            string name = res->getString("machine_name");
            int cycles = res->getInt("Cycles");
            double rev = res->getDouble("Revenue");

            cout << "| " << left << setw(20) << name
                << " | " << left << setw(8) << cycles
                << " | RM" << left << setw(11) << fixed << setprecision(2) << rev << " |" << endl;
        }

        cout << "+----------------------+----------+---------------+" << endl;

    }
    catch (const exception& e) {
        cerr << "Error generating table: " << e.what() << endl;
    }
}

// =================================================================================
//  2. FINANCIAL SUMMARY (Key Metrics List)
// =================================================================================
void AdminService::report_FinancialSummary() {
    try {
        double totalRev = 0, totalTopup = 0;

        // Get Revenue
        unique_ptr<sql::PreparedStatement> pst1(db_connection->prepareStatement("SELECT SUM(cost) FROM orders"));
        unique_ptr<sql::ResultSet> res1(pst1->executeQuery());
        if (res1->next()) totalRev = res1->getDouble(1);

        // Get TopUps
        unique_ptr<sql::PreparedStatement> pst2(db_connection->prepareStatement("SELECT SUM(amount) FROM credit_transactions WHERE type='TopUp'"));
        unique_ptr<sql::ResultSet> res2(pst2->executeQuery());
        if (res2->next()) totalTopup = res2->getDouble(1);

        cout << "\n2. Financial Summary List:\n" << endl;

        cout << "  1. Total Laundry Revenue : $" << fixed << setprecision(2) << totalRev << endl;
        cout << "  2. Total Wallet Top-Ups  : $" << totalTopup << endl;

        // Calculate "Float" (Money held in user wallets but not spent)
        double netHoldings = totalTopup - totalRev;
        cout << "  3. Net Float (Unspent)   : $" << netHoldings << endl;

    }
    catch (const exception& e) {
        cerr << "Error generating summary: " << e.what() << endl;
    }
}

// =================================================================================
//  3. REVENUE BAR CHART (Text-Based)
// =================================================================================
void AdminService::report_RevenueBarChart() {
    try {
        // Group by Date (Daily Revenue)
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement(
                "SELECT DATE(start_time) as dt, SUM(cost) as daily_sales "
                "FROM orders GROUP BY DATE(start_time) ORDER BY dt DESC LIMIT 5"
            )
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n3. Daily Revenue Bar Chart:\n" << endl;

        while (res && res->next()) {
            string date = res->getString("dt");
            double sales = res->getDouble("daily_sales");

            cout << "  " << left << setw(12) << date << " : ";

            // Logic: 1 Star (*) per $10 earned
            int barLength = static_cast<int>(sales / 10.0);
            if (barLength == 0 && sales > 0) barLength = 1; // At least one star if any sale

            for (int i = 0; i < barLength; i++) cout << "*"; // The "Bar"

            cout << " ($" << fixed << setprecision(2) << sales << ")" << endl;
        }

    }
    catch (const exception& e) {
        cerr << "Error generating chart: " << e.what() << endl;
    }
}

// =================================================================================
//  4. REVENUE TREND ANALYSIS (Graph Summary / % Change)
// =================================================================================
void AdminService::report_RevenueTrendAnalysis() {
    try {
        // Fetch daily sales, ordered by date ascending for comparison
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement(
                "SELECT DATE(start_time) as dt, SUM(cost) as daily_sales "
                "FROM orders GROUP BY DATE(start_time) ORDER BY dt ASC"
            )
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n4. Revenue Trend Summary (% Change):\n" << endl;

        double prevSales = 0.0;
        bool firstRow = true;

        while (res && res->next()) {
            string date = res->getString("dt");
            double currentSales = res->getDouble("daily_sales");

            cout << "  Date " << left << setw(12) << date << " Sales: $" << fixed << setprecision(2) << currentSales;

            if (!firstRow && prevSales > 0) {
                // Calculate Percentage Change
                double change = ((currentSales - prevSales) / prevSales) * 100.0;

                cout << " (";
                if (change > 0) cout << "+" << (int)change << "% increase";
                else if (change < 0) cout << (int)change << "% decrease";
                else cout << "0% change";
                cout << ")";
            }
            else if (!firstRow && prevSales == 0) {
                cout << " (+100% new revenue)";
            }

            cout << endl;
            prevSales = currentSales;
            firstRow = false;
        }

    }
    catch (const exception& e) {
        cerr << "Error generating trend: " << e.what() << endl;
    }
}

// --- NEW: SAVE TODAY'S REPORT TO DB ---
void AdminService::saveDailyReportToDB() {
    cout << "\n[SYSTEM] Calculating totals for today..." << endl;

    double totalTopup = 0.0;
    double totalRevenue = 0.0;
    int totalOrders = 0;

    try {
        // 1. Calculate Top-Ups
        unique_ptr<sql::PreparedStatement> pst1(
            db_connection->prepareStatement("SELECT COALESCE(SUM(amount), 0.00) FROM credit_transactions WHERE type = 'TopUp' AND DATE(date) = CURDATE()")
        );
        unique_ptr<sql::ResultSet> res1(pst1->executeQuery());
        if (res1->next()) totalTopup = res1->getDouble(1);

        // 2. Calculate Revenue
        unique_ptr<sql::PreparedStatement> pst2(
            db_connection->prepareStatement("SELECT COALESCE(SUM(cost), 0.00) FROM orders WHERE DATE(start_time) = CURDATE()")
        );
        unique_ptr<sql::ResultSet> res2(pst2->executeQuery());
        if (res2->next()) totalRevenue = res2->getDouble(1);

        // 3. Count Orders
        unique_ptr<sql::PreparedStatement> pst3(
            db_connection->prepareStatement("SELECT COUNT(*) FROM orders WHERE DATE(start_time) = CURDATE()")
        );
        unique_ptr<sql::ResultSet> res3(pst3->executeQuery());
        if (res3->next()) totalOrders = res3->getInt(1);

        // 4. INSERT INTO REPORTS TABLE (The "Save" Action)
        // ON DUPLICATE KEY UPDATE means: If a report for Today already exists, update it.
        unique_ptr<sql::PreparedStatement> pstInsert(
            db_connection->prepareStatement(
                "INSERT INTO reports (report_date, total_topups, total_revenue, total_orders) "
                "VALUES (CURDATE(), ?, ?, ?) "
                "ON DUPLICATE KEY UPDATE "
                "total_topups = VALUES(total_topups), "
                "total_revenue = VALUES(total_revenue), "
                "total_orders = VALUES(total_orders)"
            )
        );
        pstInsert->setDouble(1, totalTopup);
        pstInsert->setDouble(2, totalRevenue);
        pstInsert->setInt(3, totalOrders);
        pstInsert->executeUpdate();

        cout << "[SUCCESS] Report for TODAY saved successfully." << endl;
        cout << "Stats archived: Revenue RM " << totalRevenue << " | TopUps RM " << totalTopup << endl;

    }
    catch (const exception& e) {
        cerr << "[ERROR] Could not save report: " << e.what() << endl;
    }
}

// --- NEW: VIEW PAST REPORTS ---
void AdminService::viewSavedReports() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            db_connection->prepareStatement("SELECT * FROM reports ORDER BY report_date DESC LIMIT 15")
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n================= ARCHIVED DAILY REPORTS =================" << endl;
        cout << left << setw(15) << "Date"
            << setw(15) << "Top-Ups"
            << setw(15) << "Revenue"
            << "Orders" << endl;
        cout << "----------------------------------------------------------" << endl;

        bool found = false;
        while (res && res->next()) {
            found = true;
            cout << left << setw(15) << res->getString("report_date")
                << "RM " << setw(12) << fixed << setprecision(2) << res->getDouble("total_topups")
                << "RM " << setw(12) << res->getDouble("total_revenue")
                << res->getInt("total_orders") << endl;
        }
        if (!found) cout << "No archived reports found." << endl;
        cout << "==========================================================" << endl;
    }
    catch (...) {}
}