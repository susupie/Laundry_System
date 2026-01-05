#include "InputValidation.h"
#include <iostream>
#include <limits> 

using namespace std;

int getValidatedChoice(int min_choice, int max_choice) {
    int choice;
    while (true) {

        /*cout << " > ";*/
        cin >> choice;

        if (cin.fail()) {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        }
        else {
            if (choice >= min_choice && choice <= max_choice) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
                return choice;
            }
            else {
                cout << "Invalid choice. Please enter a number between "
                    << min_choice << " and " << max_choice << "." << endl;
                cout << "Choice > ";
            }
        }
    }
}