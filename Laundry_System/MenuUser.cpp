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

        cout << "\n==============================================" << endl;
        cout << "          WELCOME: " << userEmail << endl;
        cout << "          BALANCE: RM " << fixed << setprecision(2) << currentBalance << endl;
        cout << "==============================================" << endl;
        cout << "1. Top Up Balance" << endl;
        cout << "2. Start Machine" << endl;
        cout << "3. View Orders" << endl;
        cout << "4. View Transactions" << endl;
        cout << "5. Logout" << endl;
        cout << "----------------------------------------------" << endl;

        cout << "Select Option > ";
        menuUser = getValidatedChoice(1, 5);

        if (menuUser == 1) {
            // --- TOP UP SELECTION ---
            cout << "\n==============================================" << endl;
            cout << "            TOP UP SELECTION                  " << endl;
            cout << "==============================================" << endl;
            cout << "1. RM 10.00" << endl;
            cout << "2. RM 20.00" << endl;
            cout << "3. RM 50.00" << endl;
            cout << "4. RM 100.00" << endl;
            cout << "----------------------------------------------" << endl;

            cout << "Select Amount > ";
            int amtChoice = getValidatedChoice(1, 4);

            double targetAmount = 0.0;
            switch (amtChoice) {
            case 1: targetAmount = 10.00; break;
            case 2: targetAmount = 20.00; break;
            case 3: targetAmount = 50.00; break;
            case 4: targetAmount = 100.00; break;
            }

            // --- CHOOSE METHOD ---
            cout << "\nSelect Payment Method:" << endl;
            cout << "1. Cash" << endl;
            cout << "2. Credit / Debit Card" << endl;
            cout << "3. Cancel" << endl;
            cout << "Choice > ";
            int payMethod = getValidatedChoice(1, 3);

            if (payMethod == 3) {
                cout << "Transaction Cancelled." << endl;
                continue;
            }

            // --- START PAYMENT GATEWAY UI ---
            cout << "\n";
            cout << "**********************************************" << endl;
            cout << "              PAYMENT GATEWAY                 " << endl;
            cout << "**********************************************" << endl;
            cout << "  TRANSACTION TYPE  : " << (payMethod == 1 ? "CASH DEPOSIT" : "CARD PAYMENT") << endl;
            cout << "  TOTAL AMOUNT DUE  : RM " << fixed << setprecision(2) << targetAmount << endl;
            cout << "----------------------------------------------" << endl;

            if (payMethod == 1) {
                // --- CASH LOGIC ---
                double amountTendered = 0.0;

                // Input Loop
                while (true) {
                    cout << "  ENTER CASH AMOUNT : RM ";
                    if (cin >> amountTendered) {
                        if (amountTendered > 0) break;
                    }
                    else {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

                cout << "----------------------------------------------" << endl;

                if (amountTendered >= targetAmount) {
                    double change = amountTendered - targetAmount;
                    cout << "  CHANGE DUE        : RM " << change << endl;
                    cout << "**********************************************" << endl;

                    if (orderService.topUpBalance(userId, targetAmount)) {
                        cout << "\n[SUCCESS] Cash Accepted. Wallet Updated." << endl;
                    }
                    else {
                        cout << "\n[ERROR] System Error. Refund Issued." << endl;
                    }
                }
                else {
                    double shortAmt = targetAmount - amountTendered;
                    cout << "  [!] SHORTAGE      : RM " << shortAmt << endl;
                    cout << "**********************************************" << endl;
                    cout << "\n[FAILED] Insufficient funds. Cash Returned." << endl;
                }
            }
            else if (payMethod == 2) {
                // --- CARD LOGIC ---
                cout << "  STATUS            : INSERT CARD..." << endl;
                this_thread::sleep_for(chrono::seconds(1)); // Simulate action
                cout << "  CARD READ         : ************4242" << endl;
                cout << "----------------------------------------------" << endl;

                cout << "  ENTER 6-DIGIT PIN : ";
                string pin;
                getline(cin, pin); // User types PIN

                cout << "  PROCESSING        : CONTACTING BANK";
                // Animation dots
                for (int i = 0; i < 3; i++) {
                    cout << "." << flush;
                    this_thread::sleep_for(chrono::milliseconds(600));
                }
                cout << endl;

                cout << "**********************************************" << endl;

                // Validate PIN Length (Simple check)
                if (pin.length() == 6) {
                    if (orderService.topUpBalance(userId, targetAmount)) {
                        cout << "\n[APPROVED] Transaction Successful." << endl;
                        cout << "RM " << targetAmount << " credited to account." << endl;
                    }
                    else {
                        cout << "\n[DECLINED] Bank Error." << endl;
                    }
                }
                else {
                    cout << "\n[DECLINED] Invalid PIN." << endl;
                }
            }
            cout << "==============================================" << endl;
        }
        else if (menuUser == 2) {
            // --- START MACHINE ---

            // 1. Choose Type
            cout << "\n==============================================" << endl;
            cout << "          SELECT MACHINE TYPE " << endl;
            cout << "1. Washer\n2. Dryer" << endl;
            cout << "Choice > ";
            int typeChoice = getValidatedChoice(1, 2);
            string machineType = (typeChoice == 1) ? "washer" : "dryer";

            // 2. FETCH FROM DB (Now includes price & weight!)
            cout << "\n--- SEARCHING FOR IDLE " << machineType << "S ---" << endl;
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
            cout << "------------------------------------------------------------" << endl;

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
            cout << "\n==============================================" << endl;
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