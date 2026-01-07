#include "AdminService.h"
#include "InputValidation.h"
#include "db_conn.h"
#include <iostream>
#include <memory>
#include <string>  // required for getline
#include <iomanip> // for centering logic

using namespace std;

// --- VISUAL HELPERS ---

// 1. Just prints the title box (NO CLEAR SCREEN)
void printHeader(string title) {
    cout << "\n"; // Extra space to separate from previous action
    cout << "==============================================" << endl;
    cout << "           " << title << endl;
    cout << "==============================================" << endl;
}

// 2. Shows a nice box for sub-actions
void printSectionHeader(string title) {
    cout << "\n";
    cout << "**********************************************" << endl;

    // Center text logic
    int width = 46;
    int padding = (width - static_cast<int>(title.length())) / 2;
    if (padding < 0) padding = 0;

    cout << string(padding, ' ') << title << endl;
    cout << "**********************************************" << endl;
}

// --- SUB MENU: CUSTOMERS ---
void SubMenu_Customers(AdminService& service) {
    bool backToMain = false;
    while (!backToMain) {
        printHeader("MANAGE CUSTOMERS");
        cout << "1. View All Customers" << endl;
        cout << "2. Add New Customer" << endl;
        cout << "3. Delete Customer" << endl;
        cout << "4. Back" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "Select Option > ";

        int choice = getValidatedChoice(1, 4);

        if (choice == 1) {
            service.listCustomers();
        }
        else if (choice == 2) {
            printSectionHeader("ADD NEW CUSTOMER");

            // Replaced raw getline with safe wrapper
            string username = getValidatedString("Username : ");
            string email = getValidatedString("Email    : ");
            string password = getValidatedString("Password : ");

            service.addCustomer(username, email, password);

            cout << "\n(Press Enter to continue...)"; cin.get();
        }
        else if (choice == 3) {
            service.listCustomers(); // Show list first so they know ID
            printSectionHeader("DELETE CUSTOMER");
            cout << "Enter User ID to delete: ";
            int targetUserId = getValidatedChoice(1, 9999);

            service.deleteCustomer(targetUserId);
        }
        else if (choice == 4) {
            backToMain = true;
        }
    }
}

// --- SUB MENU: MACHINES ---
void SubMenu_Machines(AdminService& service) {
    bool backToMain = false;
    while (!backToMain) {
        printHeader("MANAGE MACHINES");
        cout << "1. View All Machines" << endl;
        cout << "2. Update Status (Maintenance)" << endl;
        cout << "3. Add New Machine" << endl;
        cout << "4. Delete Machine" << endl;
        cout << "5. Back" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "Select Option > ";

        int choice = getValidatedChoice(1, 5);

        if (choice == 1) {
            service.listMachines();
        }
        else if (choice == 2) {
            service.listMachines();
            printSectionHeader("UPDATE MACHINE STATUS");

            cout << "Enter Machine ID : ";
            int targetMachineId = getValidatedChoice(1, 9999);

            cout << "\nSelect New Status:" << endl;
            cout << "1. Idle (Ready/Fixed)" << endl;
            cout << "2. Out of Service (Broken)" << endl;
            cout << "Choice > ";
            int statusChoice = getValidatedChoice(1, 2);

            string maintenanceNote = "-";
            if (statusChoice == 2) {
                cout << "Enter Reason (e.g. Motor Issue): ";
                cin.ignore();
                getline(cin, maintenanceNote);
            }

            if (service.updateMachineStatus(targetMachineId, statusChoice, maintenanceNote))
                cout << "\n[SUCCESS] Status Updated Successfully." << endl;
            else
                cout << "\n[FAILED] ID not found." << endl;
        }
        else if (choice == 3) {
            printSectionHeader("ADD NEW MACHINE");

            // Safe String Input
            string name = getValidatedString("Name (e.g. Washer A) : ");
            string type = getValidatedString("Type (washer/dryer)  : ");

            // Safe Numeric Input (Prevents Crashing)
            int weight = (int)getValidatedDouble("Weight Capacity (kg) : ");
            double price = getValidatedDouble("Price per Cycle (RM) : ");

            service.addMachine(name, type, weight, price);

            cout << "\n(Press Enter to continue...)"; cin.get();
        }
        else if (choice == 4) {
            printSectionHeader("DELETE MACHINE");
            cout << "Enter Machine ID : ";
            int targetMachineId = getValidatedChoice(1, 9999);

            service.deleteMachine(targetMachineId);
        }
        else if (choice == 5) {
            backToMain = true;
        }
    }
}

// --- SUB MENU: ORDERS ---
void SubMenu_Orders(AdminService& service) {
    bool backToMain = false;
    while (!backToMain) {
        printHeader("MANAGE ORDERS");
        cout << "1. View Recent Orders" << endl;
        cout << "2. Delete Order Record" << endl;
        cout << "3. Back" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "Select Option > ";

        int choice = getValidatedChoice(1, 3);

        if (choice == 1) {
            service.listAllOrders();
        }
        else if (choice == 2) {
            printSectionHeader("DELETE ORDER RECORD");
            cout << "Enter Order ID : ";
            int targetOrderId = getValidatedChoice(1, 9999);

            service.deleteOrder(targetOrderId);
        }
        else if (choice == 3) {
            backToMain = true;
        }
    }
}

// --- SUB MENU: REPORTS ---
void SubMenu_Reports(AdminService& service) {
    bool backToMain = false;
    while (!backToMain) {
        printHeader("ADVANCED REPORTING");
        cout << "1. Machine Performance Table" << endl;
        cout << "2. Financial Summary List" << endl;
        cout << "3. Revenue Bar Chart (Visual)" << endl;
        cout << "4. Revenue Trend Analysis (%)" << endl;
        cout << "5. Search Orders" << endl;
        cout << "6. Export Daily Report to PDF" << endl;
        cout << "7. Export Transaction History to PDF" << endl;
        cout << "8. View Past Reports" << endl;
        cout << "9. Back" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "Select Report > ";

        int choice = getValidatedChoice(1, 9);

        // Options 1-5 remain the same (Console Views)
        if (choice == 1) service.report_MachineUsageTable();
        else if (choice == 2) service.report_FinancialSummary();
        else if (choice == 3) service.report_RevenueBarChart();
        else if (choice == 4) service.report_RevenueTrendAnalysis();
        else if (choice == 5) {
            printSectionHeader("SEARCH DATABASE");
            // No need for 'cin >> s', use the wrapper to prevent empty searches
            string s = getValidatedString("Enter Customer Username : ");
            service.searchOrders(s);
        }
        else if (choice == 6) { // Export Menu
            printSectionHeader("EXPORT REPORTS TO PDF");
            cout << "1. Export Today's Report" << endl;
            cout << "2. Export Specific Date" << endl;
            cout << "3. Export ALL History (Summary)" << endl;
            cout << "4. Cancel" << endl;
            cout << "Select Type > ";

            int pdfChoice = getValidatedChoice(1, 4);

            if (pdfChoice == 1) {
                string fname = getValidatedString("Filename: ");
                service.exportDailyReportPDF(fname, "CURDATE()");
            }
            else if (pdfChoice == 2) {
                // 1. Show the list of valid dates
                service.showAvailableReportDates();

                // 2. Now ask for input
                cout << "Enter Date from list (YYYY-MM-DD): ";
                string dateStr; cin >> dateStr;

                cout << "Filename to save as: ";
                string fname; cin >> fname;

                service.exportDailyReportPDF(fname, dateStr);
                // --- UPDATE ENDS HERE ---
            }
            else if (pdfChoice == 3) {
                cout << "Filename (e.g. full_history): ";
                string fname; cin >> fname;
                service.exportAllHistoryPDF(fname);
            }

            if (pdfChoice != 4) {
                cout << "\n(Press Enter to continue...)"; cin.get(); cin.get();
            }
        }
        else if (choice == 7) {
            printSectionHeader("EXPORT TRANS. HISTORY");
            cout << "Enter filename to save (e.g. history_log): ";
            string fname;
            cin >> fname;

            service.exportTransactionHistoryPDF(fname);

            cout << "\n(Press Enter to continue...)"; cin.get(); cin.get();
        }

        // --- VIEW SAVED REPORTS ---
        else if (choice == 8) {
            service.viewSavedReports();
            cout << "\n(Press Enter to continue...)"; cin.get(); cin.get();
        }

        else if (choice == 9) {
            backToMain = true;
        }

        if (!backToMain && choice <= 5) {
            cout << "\n(Press Enter to continue...)"; cin.get(); cin.get();
        }
    }
}

// --- MAIN ADMIN MENU ---
void MenuAdmin(shared_ptr<DBConnection> db_conn, string adminName) {
    AdminService adminService(db_conn);

    bool stayInAdminMenu = true;
    while (stayInAdminMenu) {
        printHeader("ADMIN DASHBOARD: " + adminName);
        cout << "1. Manage Customers" << endl;
        cout << "2. Manage Machines" << endl;
        cout << "3. Manage Orders" << endl;
        cout << "4. Reports & Analytics" << endl;
        cout << "5. Backup Data" << endl;
        cout << "6. Logout" << endl;
        cout << "----------------------------------------------" << endl;

        cout << "Select Option > ";
        int choice = getValidatedChoice(1, 6);

        switch (choice) {
        case 1:
            SubMenu_Customers(adminService);
            break;
        case 2:
            SubMenu_Machines(adminService);
            break;
        case 3:
            SubMenu_Orders(adminService);
            break;
        case 4:
            SubMenu_Reports(adminService);
            break;
        case 5:
            printSectionHeader("DATA BACKUP");
            adminService.saveData();
            break;
        case 6:
            stayInAdminMenu = false;
            break;
        }
    }
}