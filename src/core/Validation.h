#pragma once
#include <string>
#include "GradingScale.h"

// Each returns true when valid; otherwise fills `error` with a message for the user.
bool validateGrade(double grade, const GradingScale& scale, std::string& error);
bool validateUnits(int units, std::string& error);
bool validateName(const std::string& name, std::string& error);
bool validateScale(const GradingScale& scale, std::string& error);
bool validateTarget(double target, const GradingScale& scale, std::string& error);

std::string trim(const std::string& text);
std::string formatGrade(double value); // two decimals, e.g. 1.80
