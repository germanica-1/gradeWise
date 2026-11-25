#include "Validation.h"
#include <cctype>
#include <cstdio>

std::string formatGrade(double value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return buffer;
}

std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) {
        start++;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        end--;
    }
    return text.substr(start, end - start);
}

bool validateGrade(double grade, const GradingScale& scale, std::string& error) {
    if (grade < scale.minGrade || grade > scale.maxGrade) {
        if (grade < scale.minGrade) {
            error = "Grade is below the lowest allowed value (" + formatGrade(scale.minGrade) + ").";
        } else {
            error = "Grade is above the highest allowed value (" + formatGrade(scale.maxGrade) + ").";
        }
        return false;
    }
    return true;
}

bool validateUnits(int units, std::string& error) {
    if (units < 1) {
        error = "Units must be at least 1.";
        return false;
    } else if (units > 10) {
        error = "Units cannot be more than 10.";
        return false;
    }
    return true;
}

bool validateName(const std::string& name, std::string& error) {
    std::string cleaned = trim(name);
    if (cleaned.empty()) {
        error = "Subject name cannot be empty.";
        return false;
    }
    if (cleaned.size() > 60) {
        error = "Subject name is too long (max 60 characters).";
        return false;
    }
    return true;
}

bool validateScale(const GradingScale& scale, std::string& error) {
    if (scale.minGrade >= scale.maxGrade) {
        error = "Minimum grade must be smaller than maximum grade.";
        return false;
    } else if (scale.passingGrade < scale.minGrade || scale.passingGrade > scale.maxGrade) {
        error = "Passing grade must be inside the scale range.";
        return false;
    } else if (scale.nearMarginPercent < 0 || scale.nearMarginPercent > 50) {
        error = "Near-target margin must be between 0% and 50%.";
        return false;
    }
    return true;
}

bool validateTarget(double target, const GradingScale& scale, std::string& error) {
    if (target < scale.minGrade || target > scale.maxGrade) {
        error = "Target GWA must be inside the grading scale range.";
        return false;
    }
    return true;
}
