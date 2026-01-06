#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

#include "AdminService.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <ctime>
#include <sstream>

using namespace std;

// --- HELPER: Get Date ---
string AdminService::getCurrentDate() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
}

// =================================================================================
//  THE "PURE C++" PDF GENERATOR ENGINE
// =================================================================================
void AdminService::createRawPDF(string filename, string title, vector<string> lines) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] Could not create PDF file." << endl;
        return;
    }

    // 1. PDF Header
    file << "%PDF-1.4\n";

    // 2. Object 1: Catalog
    file << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

    // 3. Object 2: Pages (Root)
    file << "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n";

    // 4. Object 3: Page Definition
    file << "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n";

    // 5. Object 5: Font Definition (Helvetica)
    file << "5 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>\nendobj\n";

    // 6. Object 4: The Content Stream (Where text goes)
    file << "4 0 obj\n<< /Length " << (1000 + lines.size() * 100) << " >>\nstream\n";

    // Start Text Block
    file << "BT\n";
    file << "/F1 24 Tf\n";      // Font Size 24 (Title)
    file << "50 750 Td\n";      // Position Top-Left (x=50, y=750)
    file << "(" << title << ") Tj\n"; // Write Title

    file << "/F1 12 Tf\n";      // Font Size 12 (Body)
    file << "0 -30 Td\n";       // Move down 30 units
    file << "(Date: " << getCurrentDate() << ") Tj\n";
    file << "0 -30 Td\n";       // Move down another 30 units for content

    // Loop through the data lines and write them
    for (const string& line : lines) {
        // Escape parentheses ( ) because PDF uses them for strings
        string safeLine = line;
        // (In a full app, you'd replace '(' with '\(' etc., but basic text is fine)

        file << "(" << safeLine << ") Tj\n"; // Write line
        file << "0 -15 Td\n";                // Move down 15 units (Next line)
    }

    file << "ET\n"; // End Text Block
    file << "endstream\nendobj\n";

    // 7. Cross-Reference Table (Simplified)
    file << "xref\n";
    file << "0 6\n";
    file << "0000000000 65535 f \n";
    file << "0000000010 00000 n \n"; // Obj 1
    file << "0000000060 00000 n \n"; // Obj 2
    file << "0000000110 00000 n \n"; // Obj 3
    file << "0000000300 00000 n \n"; // Obj 4 (Content)
    file << "0000000200 00000 n \n"; // Obj 5 (Font)

    // 8. Trailer
    file << "trailer\n<< /Size 6 /Root 1 0 R >>\n";
    file << "startxref\n400\n%%EOF\n";

    file.close();
    cout << "[SUCCESS] Report exported to '" << filename << "'" << endl;
}

// =================================================================================
//  EXPORT FUNCTIONS (Prepare Data -> Call PDF Engine)
// =================================================================================
void AdminService::exportDailyReportPDF(string customFilename) {
    // 1. AUTOMATICALLY SAVE TO DATABASE FIRST
    // This ensures that whenever a report is printed, the DB is updated.
    saveDailyReportToDB();

    cout << "\n[SYSTEM] Generating PDF Document..." << endl;

    // 2. Handle Filename Extension
    if (customFilename.length() < 4 || customFilename.substr(customFilename.length() - 4) != ".pdf") {
        customFilename += ".pdf";
    }

    vector<string> pdfLines;

    try {
        double totalRev = 0, totalTopup = 0;
        int totalOrders = 0;

        // Fetch Data
        unique_ptr<sql::PreparedStatement> pst1(db_connection->prepareStatement("SELECT COALESCE(SUM(amount),0) FROM credit_transactions WHERE type='TopUp' AND DATE(date)=CURDATE()"));
        unique_ptr<sql::ResultSet> rs1(pst1->executeQuery());
        if (rs1->next()) totalTopup = rs1->getDouble(1);

        unique_ptr<sql::PreparedStatement> pst2(db_connection->prepareStatement("SELECT COALESCE(SUM(cost),0) FROM orders WHERE DATE(start_time)=CURDATE()"));
        unique_ptr<sql::ResultSet> rs2(pst2->executeQuery());
        if (rs2->next()) totalRev = rs2->getDouble(1);

        unique_ptr<sql::PreparedStatement> pst3(db_connection->prepareStatement("SELECT COUNT(*) FROM orders WHERE DATE(start_time)=CURDATE()"));
        unique_ptr<sql::ResultSet> rs3(pst3->executeQuery());
        if (rs3->next()) totalOrders = rs3->getInt(1);

        // Format for PDF
        pdfLines.push_back("------------------------------------------------------------");
        pdfLines.push_back("METRIC                          VALUE");
        pdfLines.push_back("------------------------------------------------------------");

        stringstream ss;
        ss << left << setw(32) << "Total Revenue Today" << "RM " << fixed << setprecision(2) << totalRev;
        pdfLines.push_back(ss.str()); ss.str("");

        ss << left << setw(32) << "Total Top-Ups Collected" << "RM " << fixed << setprecision(2) << totalTopup;
        pdfLines.push_back(ss.str()); ss.str("");

        ss << left << setw(32) << "Total Orders Processed" << totalOrders;
        pdfLines.push_back(ss.str()); ss.str("");

        pdfLines.push_back("------------------------------------------------------------");
        pdfLines.push_back("");
        pdfLines.push_back("Generated by Smart Laundry Admin System.");

        // 3. Create PDF with Custom Name
        createRawPDF(customFilename, "DAILY FINANCIAL REPORT", pdfLines);

    }
    catch (...) { cout << "Error fetching data." << endl; }
}

void AdminService::exportTransactionHistoryPDF(string customFilename) {
    cout << "\n[SYSTEM] Generating PDF Document..." << endl;

    // Handle Extension
    if (customFilename.length() < 4 || customFilename.substr(customFilename.length() - 4) != ".pdf") {
        customFilename += ".pdf";
    }

    vector<string> pdfLines;
    pdfLines.push_back("----------------------------------------------------------------");
    pdfLines.push_back("DATE                 USER            TYPE            AMOUNT");
    pdfLines.push_back("----------------------------------------------------------------");

    try {
        string sql = "SELECT t.date, u.username, t.type, t.amount FROM credit_transactions t JOIN users u ON t.user_id = u.user_id ORDER BY t.date DESC LIMIT 40";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res && res->next()) {
            stringstream ss;
            string date = res->getString("date");
            ss << left << setw(21) << date.substr(0, 19)
                << setw(16) << res->getString("username")
                << setw(16) << res->getString("type")
                << "RM " << fixed << setprecision(2) << res->getDouble("amount");
            pdfLines.push_back(ss.str());
        }
    }
    catch (...) {}

    createRawPDF(customFilename, "TRANSACTION HISTORY LOG", pdfLines);
}

void AdminService::exportTransactionHistoryPDF() {
    cout << "\n[SYSTEM] Fetching Transaction Logs..." << endl;
    vector<string> pdfLines;

    pdfLines.push_back("----------------------------------------------------------------");
    pdfLines.push_back("DATE                 USER            TYPE            AMOUNT");
    pdfLines.push_back("----------------------------------------------------------------");

    try {
        string sql = "SELECT t.date, u.username, t.type, t.amount FROM credit_transactions t JOIN users u ON t.user_id = u.user_id ORDER BY t.date DESC LIMIT 30";
        unique_ptr<sql::PreparedStatement> pstmt(db_connection->prepareStatement(sql));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res && res->next()) {
            stringstream ss;
            string date = res->getString("date");
            // Truncate date to fit
            ss << left << setw(21) << date.substr(0, 19)
                << setw(16) << res->getString("username")
                << setw(16) << res->getString("type")
                << "RM " << fixed << setprecision(2) << res->getDouble("amount");
            pdfLines.push_back(ss.str());
        }
    }
    catch (...) {}

    createRawPDF("Report_History.pdf", "TRANSACTION HISTORY LOG", pdfLines);
}

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

        cout << "  1. Total Laundry Revenue : RM" << fixed << setprecision(2) << totalRev << endl;
        cout << "  2. Total Wallet Top-Ups  : RM" << totalTopup << endl;

        // Calculate "Float" (Money held in user wallets but not spent)
        double netHoldings = totalTopup - totalRev;
        cout << "  3. Net Float (Unspent)   : RM" << netHoldings << endl;

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

            // Logic: 1 Star (*) per RM10 earned
            int barLength = static_cast<int>(sales / 10.0);
            if (barLength == 0 && sales > 0) barLength = 1; // At least one star if any sale

            for (int i = 0; i < barLength; i++) cout << "*"; // The "Bar"

            cout << " (RM" << fixed << setprecision(2) << sales << ")" << endl;
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

            cout << "  Date " << left << setw(12) << date << " Sales: RM" << fixed << setprecision(2) << currentSales;

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