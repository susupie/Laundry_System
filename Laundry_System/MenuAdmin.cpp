#include "AdminService.h"
#include "InputValidation.h"
#include "db_conn.h"
#include <iostream>
#include <memory>
#include <string>  // required for getline
#include <iomanip> // for centering logic

using namespace std;

// 1. Just prints the title box (NO CLEAR SCREEN)
void printHeader(string title) {
    cout << "\n"; 
    cout << "================================================================\n";
    cout << "                 " << title << endl;
    cout << "================================================================\n";
}

// 2. Shows a nice box for sub-actions
void printSectionHeader(string title) {
    cout << "\n";
    cout << "***************************************************************" << endl;
    // Center text logic
    int width = 46;
    int padding = (width - static_cast<int>(title.length())) / 2;
    if (padding < 0) padding = 0;

    cout << string(padding, ' ') << title << endl;
    cout << "***************************************************************" << endl;
}

// --- SUB MENU: CUSTOMERS ---
void SubMenu_Customers(AdminService& service) {
    bool backToMain = false;
    while (!backToMain) {
        printHeader("    MANAGE CUSTOMERS");
        cout << "1. View All Customers" << endl;
        cout << "2. Add New Customer" << endl;
        cout << "3. Delete Customer" << endl;
        cout << "4. Back" << endl;
        cout << "-----------------------------------------------------------------" << endl;
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
        printHeader("     MANAGE MACHINES");
        cout << "1. View All Machines" << endl;
        cout << "2. Update Status (Maintenance)" << endl;
        cout << "3. Add New Machine" << endl;
        cout << "4. Delete Machine" << endl;
        cout << "5. Back" << endl;
        cout << "-----------------------------------------------------------------" << endl;
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
                //cin.ignore();
                getline(cin, maintenanceNote);
            }

            if (service.updateMachineStatus(targetMachineId, statusChoice, maintenanceNote))
                cout << "\n[SUCCESS] Status Updated Successfully." << endl;
            else
                cout << "\n[FAILED] ID not found." << endl;
        }
        else if (choice == 3) {
            printSectionHeader("ADD MACHINE");
            string name = getValidatedString("Name: ");

            // 1/2 Selection for Type
            cout << "Type (1. Washer, 2. Dryer) > ";
            int t = getValidatedChoice(1, 2);
            string type = (t == 1) ? "washer" : "dryer";

            string location = getValidatedString("Location: ");
            int weight = (int)getValidatedDouble("Weight (kg): ");
            double price = getValidatedDouble("Price (RM): ");

            service.addMachine(name, type, weight, price, location);
            cout << "\n(Press enter to continue...)"; cin.get();
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
        printHeader("     MANAGE ORDERS");
        cout << "1. View Recent Orders" << endl;
        cout << "2. Delete Order Record" << endl;
        cout << "3. Back" << endl;
        cout << "-----------------------------------------------------------------" << endl;
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
void SubMenu_ExportPDF(AdminService& service) {
    bool back = false;
    while (!back) {
        printHeader("     EXPORT TO PDF");
        cout << "1. Generate Master Report (All History)" << endl;
        cout << "2. Daily Summary (Today's Report)" << endl;
        cout << "3. Specific Date Report" << endl;
        cout << "4. Back" << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "Select > ";

        int choice = getValidatedChoice(1, 4);

        if (choice == 1) {
            string f = getValidatedString("Enter Filename (e.g. master_rep): ");
            service.exportMasterReportPDF(f);
            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 2) {
            string f = getValidatedString("Enter Filename: ");
            service.exportDailyReportPDF(f, "CURDATE()"); // Today
            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 3) {
            // New "1, 2, 3" List Selection Logic
            printSectionHeader("SELECT DATE");
            vector<string> dates = service.getAvailableReportDates();

            for (size_t i = 0; i < dates.size(); i++) {
                cout << (i + 1) << ". " << dates[i] << endl;
            }

            cout << "Select No. > ";
            int dateIdx = getValidatedChoice(1, static_cast<int>(dates.size()));
            string selectedDate = dates[dateIdx - 1];

            string f = getValidatedString("Enter Filename: ");
            service.exportDailyReportPDF(f, selectedDate);

            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 4) back = true;
    }
}

// --- RESTRUCTURED REPORTS MENU ---
void SubMenu_Reports(AdminService& service) {
    bool back = false;
    while (!back) {
        printHeader("     REPORTS & ANALYTICS");
        cout << "1. Machine Performance Table" << endl;
        cout << "2. Revenue Bar Chart (Monthly)" << endl;
        cout << "3. Revenue Trend Analysis (Monthly)" << endl;
        cout << "4. Generate Sales Summary" << endl; // New Console View
        cout << "5. Export to PDF" << endl;          // New Sub-Menu
        cout << "6. View Saved Database Reports" << endl;
        cout << "7. Back" << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "Select > ";

        int choice = getValidatedChoice(1, 7);

        if (choice == 1) { 
            service.report_MachineUsageTable(); 
            cout << "\n(Press enter to continue...)"; 
            cin.get(); 
        }
        else if (choice == 2) { 
            service.report_RevenueBarChart(); 
            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 3) { 
            service.report_RevenueTrendAnalysis(); 
            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 4) { 
            service.report_SalesSummary(); 
            cout << "\n(Press enter to continue...)";
            cin.get();
        } 
        else if (choice == 5) { 
            SubMenu_ExportPDF(service); 
        } 
        else if (choice == 6) { 
            service.viewSavedReports(); 
            cout << "\n(Press enter to continue...)";
            cin.get();
        }
        else if (choice == 7) { 
            back = true; 
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
        cout << "5. Logout" << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "Select Option > ";
        int choice = getValidatedChoice(1, 5);

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
            stayInAdminMenu = false;
            break;
        }
    }
}