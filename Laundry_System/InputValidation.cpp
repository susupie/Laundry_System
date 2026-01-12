#include "InputValidation.h"
#include <iostream>
#include <limits> 
#include <string>
#include <conio.h> 


using namespace std;

int getValidatedChoice(int min_choice, int max_choice) {
    int choice;
    while (true) {
        if (cin >> choice) {
            if (choice >= min_choice && choice <= max_choice) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return choice;
            }
            cout << "Invalid choice. Enter (" << min_choice << "-" << max_choice << ") > ";
        }
        else {
            cout << "Invalid input. Numbers only > ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// --- NEW VALIDATION FOR PRICES/WEIGHTS ---
double getValidatedDouble(string prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= 0) { // Assume prices/weights can't be negative
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "[!] Value cannot be negative." << endl;
        }
        else {
            cout << "[!] Invalid input. Please enter a number (e.g. 10.50)." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// --- NEW VALIDATION FOR STRINGS ---
string getValidatedString(string prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (!input.empty()) {
            return input;
        }
        cout << "[!] Input cannot be empty." << endl;
    }
}

string getMaskedPassword(string prompt) {
    string password = "";
    char ch;

    cout << prompt;

    while (true) {
        ch = _getch(); // Get character without echoing to screen

        // IF ENTER KEY (13) IS PRESSED
        if (ch == 13) {
            cout << endl;
            break;
        }
        // IF BACKSPACE KEY (8) IS PRESSED
        else if (ch == 8) {
            if (!password.empty()) {
                password.pop_back();   // Remove last char from string
                cout << "\b \b";       // Erase star from screen
            }
        }
        // NORMAL CHARACTER
        else {
            password += ch;
            cout << "*";
        }
    }
    return password;
}