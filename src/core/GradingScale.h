#pragma once
#include <string>

enum class Direction { LowerIsBetter, HigherIsBetter };

struct GradingScale {
    std::string name = "STI College";
    double minGrade = 1.0; // smallest number on the scale (not necessarily the best grade)
    double maxGrade = 5.0; // largest number on the scale
    double passingGrade = 3.0;
    Direction direction = Direction::LowerIsBetter;
    double nearMarginPercent = 5.0; // "close to goal" window, as % of the scale range
};

// Preset indexes, in the order shown in the Settings combo box.
const int PRESET_STI = 0;
const int PRESET_UST = 1;
const int PRESET_UE = 2;
const int PRESET_NU = 3;
const int PRESET_FEU = 4;
const int PRESET_CUSTOM = 5;

GradingScale presetScale(int presetIndex);
bool sameScale(const GradingScale& a, const GradingScale& b); // compares the numbers, not the name

// The ONLY places that know which direction is "better". Never compare grades with < or > elsewhere.
bool isBetterOrEqual(double a, double b, const GradingScale& scale);
bool isPassing(double grade, const GradingScale& scale);
double goodness(double grade, const GradingScale& scale); // 0.0 = worst possible, 1.0 = best possible
double bestGrade(const GradingScale& scale);
double worstGrade(const GradingScale& scale);
