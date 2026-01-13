#include "MenuUser.h"
#include "InputValidation.h"
#include "OrderService.h"
#include <iostream>
#include <thread> 
#include <chrono> 
#include <vector>
#include <iomanip> 

using namespace std;

// machine simulation
void backgroundMachineTask(shared_ptr<DBConnection> db, int machineId, int orderId) {
    int duration = 45; 
    this_thread::sleep_for(chrono::seconds(duration));
    try {
        OrderService threadService(db);
        threadService.completeWash(machineId, orderId);
    }
    catch (...) {}
}

void MenuUser(shared_ptr<DBConnection> db_conn, string userEmail) {
    OrderService orderService(db_conn);

    int userId = orderService.getUserIdByEmail(userEmail);
    if (userId == -1) {
        cout << "Error: User account not found." << endl;
        return;
    }

    int menuUser;
    bool stayInMenu = true;

    while (stayInMenu) {
        double currentBalance = orderService.getUserBalance(userId);

        cout << "\n================================================================\n";
        cout << "                 WELCOME: " << userEmail << endl;
        cout << "                 BALANCE: RM " << fixed << setprecision(2) << currentBalance << endl;
        cout << "================================================================\n";
        cout << "1. Top Up Balance" << endl;
        cout << "2. Start Machine" << endl;
        cout << "3. View Orders" << endl;
        cout << "4. View Transactions" << endl;
        cout << "5. Logout" << endl;
        cout << "-----------------------------------------------------------------" << endl;

        cout << "Select Option > ";
        menuUser = getValidatedChoice(1, 5);

        if (menuUser == 1) {
            // --- TOP UP SELECTION ---
            cout << "\n================================================================\n";
            cout << "                          TOP UP WALLET                     " << endl;
            cout << "================================================================\n";
            cout << "1. RM 10.00\n2. RM 20.00\n3. RM 50.00\n4. RM 100.00" << endl;
            cout << "-----------------------------------------------------------------" << endl;
            cout << "Select Amount > ";

            int amtChoice = getValidatedChoice(1, 4);

            double targetAmount = 0.0;
            switch (amtChoice) {
            case 1: targetAmount = 10.00; break;
            case 2: targetAmount = 20.00; break;
            case 3: targetAmount = 50.00; break;
            case 4: targetAmount = 100.00; break;
            }

            cout << "\n";
            cout << "**********************************************" << endl;
            cout << "              PAYMENT GATEWAY                 " << endl;
            cout << "**********************************************" << endl;
            cout << "  TRANSACTION TYPE  : ONLINE BANKING" << endl;
            cout << "  TOTAL AMOUNT DUE  : RM " << fixed << setprecision(2) << targetAmount << endl;
            cout << "----------------------------------------------" << endl;

            cout << "  STATUS            : CONNECTING TO BANK..." << endl;
            this_thread::sleep_for(chrono::seconds(1));
            cout << "  CONNECTION        : SECURE" << endl;
            cout << "----------------------------------------------" << endl;

            string pin = getMaskedPassword("  ENTER 6-DIGIT PIN : ");

            cout << "  PROCESSING        : VERIFYING";
            for (int i = 0; i < 3; i++) {
                cout << "." << flush;
                this_thread::sleep_for(chrono::milliseconds(600));
            }
            cout << endl;

            cout << "**********************************************" << endl;

            // Validate PIN Length
            if (pin.length() == 6) {
                if (orderService.topUpBalance(userId, targetAmount)) {
                    cout << "\n[APPROVED] Transaction Successful." << endl;
                    cout << "RM " << targetAmount << " credited to your account." << endl;
                }
                else {
                    cout << "\n[DECLINED] Bank Error." << endl;
                }
            }
            else {
                cout << "\n[DECLINED] Invalid PIN (Must be 6 digits)." << endl;
            }
            cout << "\n================================================================\n";
        }
        else if (menuUser == 2) {
            // --- START MACHINE ---

            // 1. Choose Type
            cout << "\n================================================================\n";
            cout << "                      SELECT MACHINE TYPE " << endl;
            cout << "================================================================\n";
            cout << "1. Washer\n2. Dryer" << endl;
            cout << "\nChoice > ";
            int typeChoice = getValidatedChoice(1, 2);
            string machineType = (typeChoice == 1) ? "washer" : "dryer";

            // 2. FETCH FROM DB (Now includes price & weight!)

            cout << "\nSEARCHING FOR IDLE " << machineType << "S..." << endl;
            cout << "-----------------------------------------------------------------" << endl;
            vector<Machine> availableMachines = orderService.getAvailableMachines(machineType);

            if (availableMachines.empty()) {
                cout << "[!] No machines available." << endl;
                continue;
            }

            // 3. DISPLAY MACHINES (With details from DB)
            cout << left << setw(5) << "No."
                << setw(20) << "Name"
                << setw(10) << "Weight"
                << setw(10) << "Price"
                << "Location" << endl;
            cout << "-----------------------------------------------------------------" << endl;

            for (size_t i = 0; i < availableMachines.size(); i++) {
                cout << left << setw(5) << (i + 1)
                    << setw(20) << availableMachines[i].name
                    << availableMachines[i].weight << "kg   "
                    << "RM " << fixed << setprecision(2) << availableMachines[i].price << "   "
                    << availableMachines[i].location << endl;
            }

            // 4. CHOOSE MACHINE
            cout << "\nSelect Machine > ";
            int machineIndex = getValidatedChoice(1, static_cast<int>(availableMachines.size()));

            // Extract details from the chosen machine
            Machine selected = availableMachines[machineIndex - 1];

            // 5. CONFIRMATION (Price is already known)
            cout << "\nConfirm: " << selected.name
                << " (" << selected.weight << "kg)"
                << " for RM " << selected.price << "?" << endl;
            cout << "1. Yes (Pay & Start)\n2. No (Cancel)" << endl;
            cout << "Choice > ";
            int confirm = getValidatedChoice(1, 2);

            if (confirm == 1) {
                // Pass the specific DB price to startWash
                int orderId = orderService.startWash(userId, selected.id, selected.price);

                if (orderId != -1) {
                    cout << "\n[SUCCESS] Machine started! Check status in 'View Orders'." << endl;
                    thread worker(backgroundMachineTask, db_conn, selected.id, orderId);
                    worker.detach();
                }
                else {
                    cout << "\n[FAILED] Insufficient balance." << endl;
                }
            }
            else {
                cout << "Cancelled." << endl;
            }
            cout << "-----------------------------------------------------------------" << endl;
        }
        else if (menuUser == 3) {
            orderService.showUserOrders(userId);
        }
        else if (menuUser == 4) {
            orderService.showTransactionHistory(userId);
        }
        else if (menuUser == 5) {
            stayInMenu = false;
        }
    }
}