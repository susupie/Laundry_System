#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <ctime>
#include <sstream>

using namespace std;

string AdminService::getCurrentDate() {
    time_t now = time(0); struct tm tstruct; char buf[80];
    localtime_s(&tstruct, &now); strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
}

// --- PDF ENGINE ---
void AdminService::createRawPDF(string filename, string title, vector<string> lines) {
    ofstream file(filename);
    if (!file.is_open()) return;
    // Increased Page Height to 5000 to prevent cutoff
    file << "%PDF-1.4\n1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    file << "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n";
    file << "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 5000] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n";
    file << "5 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>\nendobj\n";
    file << "4 0 obj\n<< /Length " << (2000 + lines.size() * 100) << " >>\nstream\n";
    file << "BT\n/F1 20 Tf\n50 4900 Td\n(" << title << ") Tj\n/F1 9 Tf\n0 -30 Td\n(Generated: " << getCurrentDate() << ") Tj\n0 -30 Td\n";
    for (const string& line : lines) { file << "(" << line << ") Tj\n0 -12 Td\n"; }
    file << "ET\nendstream\nendobj\nxref\n0 6\n0000000000 65535 f \n0000000010 00000 n \n0000000060 00000 n \n0000000110 00000 n \n0000000300 00000 n \n0000000200 00000 n \ntrailer\n<< /Size 6 /Root 1 0 R >>\nstartxref\n400\n%%EOF\n";
    file.close();
    cout << "[SUCCESS] Saved '" << filename << "'" << endl;
    string cmd = "start \"\" \"" + filename + "\""; system(cmd.c_str());
}

// --- 1. MACHINE USAGE TABLE  ---
void AdminService::report_MachineUsageTable() {
    try {
        // LEFT JOIN machines to orders ensures machines with 0 orders still show up
        string sql = "SELECT m.machine_name, COUNT(o.order_id) as c, COALESCE(SUM(o.cost), 0.00) as r "
            "FROM machines m "
            "LEFT JOIN orders o ON m.machine_id = o.machine_id "
            "GROUP BY m.machine_name ORDER BY m.type DESC, m.machine_name ASC";
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        cout << "\n================================================================\n";
        cout << "                     MACHINE PERFORMANCE TABLE " << endl;
        cout << "================================================================\n\n";
        cout << "+----------------------+------------+-----------------+" << endl;
        cout << "| " << left << setw(20) << "Machine Name" << " | " << setw(10) << "Cycles" << " | " << setw(15) << "Total Revenue" << " |" << endl;
        cout << "+----------------------+------------+-----------------+" << endl;
        while (r->next()) {
            cout << "| " << left << setw(20) << r->getString(1)
                << " | " << setw(10) << r->getInt(2)
                << " | RM " << setw(12) << fixed << setprecision(2) << r->getDouble(3) << " |" << endl;
        }
        cout << "+----------------------+------------+-----------------+" << endl;
    }
    catch (...) {}
}

// --- 4. SALES SUMMARY (New: Profit Margin Style) ---
// 1. CONSOLE: SALES SUMMARY
void AdminService::report_SalesSummary() {
    try {
        double totalRev = 0, totalTopup = 0;
        int totalCycles = 0;

        // Get Revenue
        unique_ptr<sql::PreparedStatement> p1(db_connection->prepareStatement("SELECT COALESCE(SUM(cost),0) FROM orders"));
        unique_ptr<sql::ResultSet> r1(p1->executeQuery()); if (r1->next()) totalRev = r1->getDouble(1);

        // Get TopUp
        unique_ptr<sql::PreparedStatement> p2(db_connection->prepareStatement("SELECT COALESCE(SUM(amount),0) FROM credit_transactions WHERE type='TopUp'"));
        unique_ptr<sql::ResultSet> r2(p2->executeQuery()); if (r2->next()) totalTopup = r2->getDouble(1);

        // Get Total Cycles (To calculate expense)
        unique_ptr<sql::PreparedStatement> p3(db_connection->prepareStatement("SELECT COUNT(*) FROM orders"));
        unique_ptr<sql::ResultSet> r3(p3->executeQuery()); if (r3->next()) totalCycles = r3->getInt(1);

        // --- CALCULATION LOGIC ---
        // Assumption: Each wash/dry costs RM 3.50 in utilities (Water/Electricity)
        double costPerCycle = 3.50;
        double totalExpenses = totalCycles * costPerCycle;

        double netProfit = totalRev - totalExpenses;
        double profitMargin = (totalRev > 0) ? (netProfit / totalRev) * 100.0 : 0.0;

        cout << "\n================================================================\n";
        cout << "                            SALES SUMMARY                     " << endl;
        cout << "================================================================\n";
        cout << "\n1. Total Laundry Sales   : RM " << fixed << setprecision(2) << totalRev << endl;
        cout << "2. Total Wallet Deposits : RM " << totalTopup << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "3. Est. Utility Cost     : RM " << totalExpenses << " (RM 3.50/cycle)" << endl;
        cout << "4. Net Profit            : RM " << netProfit << endl;
        cout << "5. Profit Margin         : " << (int)profitMargin << "%" << endl;
        cout << "-----------------------------------------------------------------" << endl;
    }
    catch (...) {}
}


// --- HELPER: GET DATES FOR MENU ---
vector<string> AdminService::getAvailableReportDates() {
    vector<string> dates;
    try {
        string sql = "SELECT DISTINCT DATE(dt) as d FROM (SELECT start_time as dt FROM orders UNION SELECT date as dt FROM credit_transactions) as c ORDER BY d DESC LIMIT 10";
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        while (r->next()) dates.push_back(r->getString(1));
    }
    catch (...) {}
    return dates;
}

// 2. PDF: DAILY REPORT
void AdminService::exportDailyReportPDF(string filename, string targetDate) {
    if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".pdf") filename += ".pdf";
    string dateClause = (targetDate == "CURDATE()") ? "CURDATE()" : "'" + targetDate + "'";

    if (targetDate == "CURDATE()") saveDailyReportToDB();

    vector<string> lines;
    lines.push_back("DATE: " + (targetDate == "CURDATE()" ? getCurrentDate() : targetDate));
    lines.push_back("-----------------------------------------------------------------");

    try {
        // Fetch Revenue & Topups
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT COALESCE(SUM(cost),0), (SELECT COALESCE(SUM(amount),0) FROM credit_transactions WHERE type='TopUp' AND DATE(date)=" + dateClause + ") FROM orders WHERE DATE(start_time)=" + dateClause));
        unique_ptr<sql::ResultSet> r(p->executeQuery()); r->next();
        double rev = r->getDouble(1);
        double top = r->getDouble(2);

        // Fetch Cycle Count
        unique_ptr<sql::PreparedStatement> pCount(db_connection->prepareStatement("SELECT COUNT(*) FROM orders WHERE DATE(start_time)=" + dateClause));
        unique_ptr<sql::ResultSet> rCount(pCount->executeQuery()); rCount->next();
        int cycles = rCount->getInt(1);

        // --- CALCULATE PROFIT ---
        double expensePerCycle = 3.50; // Cost of water/electricity per machine use
        double totalExpense = cycles * expensePerCycle;
        double netProfit = rev - totalExpense;
        double margin = (rev > 0) ? (netProfit / rev) * 100.0 : 0.0;

        stringstream ss;
        ss << "REVENUE TODAY       : RM " << fixed << setprecision(2) << rev; lines.push_back(ss.str()); ss.str("");
        ss << "TOP-UPS COLLECTED   : RM " << top; lines.push_back(ss.str()); ss.str("");
        ss << "TOTAL CYCLES        : " << cycles; lines.push_back(ss.str()); ss.str("");

        lines.push_back("-----------------------------------------------------------------");

        ss << "EST. UTILITY COST   : RM " << totalExpense << " (Based on RM 3.50/cycle)"; lines.push_back(ss.str()); ss.str("");
        ss << "NET PROFIT          : RM " << netProfit; lines.push_back(ss.str()); ss.str("");
        ss << "PROFIT MARGIN       : " << (int)margin << "%"; lines.push_back(ss.str());

        lines.push_back(""); lines.push_back("MACHINE BREAKDOWN:");
        lines.push_back("-----------------------------------------------------------------");

        string sqlM = "SELECT m.machine_name, COUNT(o.order_id), COALESCE(SUM(o.cost), 0) "
            "FROM machines m LEFT JOIN orders o ON m.machine_id = o.machine_id "
            "AND DATE(o.start_time)=" + dateClause + " GROUP BY m.machine_name ORDER BY m.type DESC";

        unique_ptr<sql::PreparedStatement> pm(db_connection->prepareStatement(sqlM));
        unique_ptr<sql::ResultSet> rm(pm->executeQuery());
        while (rm->next()) {
            stringstream sm;
            sm << left << setw(20) << rm->getString(1) << " : " << rm->getInt(2) << " Cycles (RM " << rm->getDouble(3) << ")";
            lines.push_back(sm.str());
        }

    }
    catch (...) {}
    createRawPDF(filename, "DAILY SUMMARY REPORT", lines);
}

// --- PDF: MASTER REPORT (All History + Fixed Join) ---
void AdminService::exportMasterReportPDF(string filename) {
    if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".pdf") filename += ".pdf";
    cout << "\n[SYSTEM] Compiling Master Report..." << endl;
    vector<string> lines;

    // SECTION 1: MACHINE BREAKDOWN (FIXED LEFT JOIN)
    lines.push_back("SECTION 1: MACHINE BREAKDOWN (All Time)");
    lines.push_back("-----------------------------------------------------------------");
    lines.push_back("MACHINE NAME          TYPE        CYCLES      TOTAL EARNED");
    lines.push_back("-----------------------------------------------------------------");
    try {
        string sql = "SELECT m.machine_name, m.type, COUNT(o.order_id), COALESCE(SUM(o.cost),0) "
            "FROM machines m LEFT JOIN orders o ON m.machine_id=o.machine_id "
            "GROUP BY m.machine_name ORDER BY m.type DESC";
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        while (r && r->next()) {
            stringstream ss;
            ss << left << setw(22) << r->getString(1) << setw(12) << r->getString(2) << setw(12) << r->getInt(3) << "RM " << r->getDouble(4);
            lines.push_back(ss.str());
        }
    }
    catch (...) {}
    lines.push_back("");

    // SECTION 2: HISTORY
    lines.push_back("SECTION 2: FULL BUSINESS HISTORY");
    lines.push_back("-----------------------------------------------------------------");
    lines.push_back("DATE            REVENUE       TOP-UPS       ORDERS");
    lines.push_back("-----------------------------------------------------------------");
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT * FROM reports ORDER BY report_date DESC LIMIT 100"));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        while (r && r->next()) {
            stringstream ss;
            ss << left << setw(16) << r->getString("report_date") << "RM " << setw(10) << fixed << setprecision(2) << r->getDouble("total_revenue") << "RM " << setw(10) << r->getDouble("total_topups") << r->getInt("total_orders");
            lines.push_back(ss.str());
        }
    }
    catch (...) {}
    lines.push_back("");

    // SECTION 3: TRANSACTIONS
    lines.push_back("SECTION 3: ALL TRANSACTIONS");
    lines.push_back("-----------------------------------------------------------------");
    lines.push_back("DATE                 USER            TYPE            AMOUNT");
    lines.push_back("-----------------------------------------------------------------");
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT t.date, u.username, t.type, t.amount FROM credit_transactions t JOIN users u ON t.user_id=u.user_id ORDER BY t.date DESC LIMIT 100"));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        while (r && r->next()) {
            stringstream ss;
            string d = r->getString("date");
            ss << left << setw(21) << d.substr(0, 19) << setw(16) << r->getString("username") << setw(16) << r->getString("type") << "RM " << r->getDouble("amount");
            lines.push_back(ss.str());
        }
    }
    catch (...) {}

    createRawPDF(filename, "MASTER BUSINESS REPORT", lines);
}

void AdminService::report_RevenueBarChart() {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT DATE_FORMAT(start_time, '%Y-%m') as mth, SUM(cost) as sales FROM orders GROUP BY mth ORDER BY mth DESC LIMIT 6"));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        cout << "\n================================================================\n";
        cout << "                      MONTHLY REVENUE CHART" << endl;
        cout << "================================================================\n\n";
        while (r && r->next()) {
            string d = r->getString("mth"); double s = r->getDouble("sales");
            cout << left << setw(10) << d << " : ";
            int len = (int)(s / 10.0); for (int i = 0; i < len; i++) cout << "*";
            cout << " (RM " << fixed << setprecision(2) << s << ")" << endl;
        }
    }
    catch (...) {}
}
void AdminService::report_RevenueTrendAnalysis() {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT DATE_FORMAT(start_time, '%Y-%m') as mth, SUM(cost) as sales FROM orders GROUP BY mth ORDER BY mth ASC"));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        cout << "\n================================================================\n";
        cout << "                       MONTHLY TREND (% Change)" << endl;
        cout << "================================================================\n\n";
        double prev = 0.0;
        while (r && r->next()) {
            double curr = r->getDouble("sales");
            cout << r->getString("mth") << ": RM " << curr;
            if (prev > 0) {
                double chg = ((curr - prev) / prev) * 100.0;
                cout << " (" << (chg > 0 ? "+" : "") << (int)chg << "%)";
            }
            cout << endl; prev = curr;
        }
    }
    catch (...) {}
}
void AdminService::saveDailyReportToDB() {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("INSERT INTO reports (report_date, total_topups, total_revenue, total_orders) SELECT CURDATE(), (SELECT COALESCE(SUM(amount),0) FROM credit_transactions WHERE type='TopUp' AND DATE(date)=CURDATE()), (SELECT COALESCE(SUM(cost),0) FROM orders WHERE DATE(start_time)=CURDATE()), (SELECT COUNT(*) FROM orders WHERE DATE(start_time)=CURDATE()) ON DUPLICATE KEY UPDATE total_topups=VALUES(total_topups), total_revenue=VALUES(total_revenue), total_orders=VALUES(total_orders)"));
        p->executeUpdate();
    }
    catch (...) {}
}
void AdminService::viewSavedReports() {
    try {
        unique_ptr<sql::PreparedStatement> p(db_connection->prepareStatement("SELECT * FROM reports ORDER BY report_date DESC LIMIT 15"));
        unique_ptr<sql::ResultSet> r(p->executeQuery());
        cout << "\n================================================================\n";
        cout << "DATE            TOP-UP        REVENUE       ORDERS" << endl;
        cout << "================================================================\n";
        while (r->next()) {
            cout << left << setw(15) << r->getString("report_date") << "RM " << setw(10) << r->getDouble("total_topups") << "RM " << setw(10) << r->getDouble("total_revenue") << r->getInt("total_orders") << endl;
        }
        cout << "-----------------------------------------------------------------" << endl;
    }
    catch (...) {}
}
