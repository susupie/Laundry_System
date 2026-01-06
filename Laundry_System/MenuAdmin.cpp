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
            string newUsername, newEmail, newPassword;

            // Clear buffer
            cin.ignore();

            cout << "Username : "; getline(cin, newUsername);
            cout << "Email    : "; getline(cin, newEmail);
            cout << "Password : "; getline(cin, newPassword);

            service.addCustomer(newUsername, newEmail, newPassword);
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
            string machineName, machineType;
            int weightCapacity;
            double cyclePrice;

            printSectionHeader("ADD NEW MACHINE");
            cin.ignore();

            cout << "Name (e.g. Washer A) : "; getline(cin, machineName);
            cout << "Type (washer/dryer)  : "; cin >> machineType;
            cout << "Weight Capacity (kg) : "; cin >> weightCapacity;
            cout << "Price per Cycle (RM) : "; cin >> cyclePrice;

            service.addMachine(machineName, machineType, weightCapacity, cyclePrice);
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
        cout << "1. View Daily Report (Console)" << endl;
        cout << "2. View Transaction History (Console)" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "3. EXPORT Daily Report (.pdf)" << endl;
        cout << "4. EXPORT Transaction History (.pdf)" << endl;
        cout << "----------------------------------------------" << endl;
        cout << "5. Save Today's Stats to Database" << endl;
        cout << "6. View Past Database Reports" << endl;
        cout << "7. Back" << endl;
        cout << "Select Option > ";

        int choice = getValidatedChoice(1, 7);

        if (choice == 1) {
            service.generateDailyReport();
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 2) {
            service.listAllOrders(); // Reusing the list function for view
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 3) {
            service.exportDailyReportPDF(); // CALL PDF GENERATOR
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 4) {
            service.exportTransactionHistoryPDF(); // CALL PDF GENERATOR
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 5) {
            service.saveDailyReportToDB();
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 6) {
            service.viewSavedReports();
            cout << "\n(Press Enter...)"; cin.get(); cin.get();
        }
        else if (choice == 7) {
            backToMain = true;
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