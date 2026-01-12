#ifndef INPUTVALIDATION_H
#define INPUTVALIDATION_H

#include <string>

int getValidatedChoice(int min_choice, int max_choice);

// NEW: For Prices (Double) and Weights
double getValidatedDouble(std::string prompt);

// NEW: For Non-Empty Strings (Names, Emails)
std::string getValidatedString(std::string prompt);
std::string getMaskedPassword(std::string prompt);


#endif