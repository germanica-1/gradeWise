#include "GradingScale.h"

// Sources: STI, UST (Student Handbook 2018), UE (Student Manual 2023): 1.00 best, 3.00 lowest pass, 5.00 fail.
// FEU (Student Handbook 2024): 4.0 best, 1.0 (D) lowest pass, 0 (F) fail.
// NU: 4.0 best, 1.0 lowest pass, 0.0 fail (third-party guides; confirm with the registrar).
GradingScale presetScale(int presetIndex) {
    GradingScale s; // defaults: 1.00-5.00, lower is better, passing 3.00
    switch (presetIndex) {
    case PRESET_STI:
        s.name = "STI College";
        break;
    case PRESET_UST:
        s.name = "University of Santo Tomas (UST)";
        break;
    case PRESET_UE:
        s.name = "University of the East (UE)";
        break;
    case PRESET_NU:
    case PRESET_FEU:
        s.name = (presetIndex == PRESET_NU) ? "National University (NU)" : "FEU / FEU Tech";
        s.minGrade = 0.0;
        s.maxGrade = 4.0;
        s.passingGrade = 1.0;
        s.direction = Direction::HigherIsBetter;
        break;
    default:
        s.name = "Custom";
        break;
    }
    return s;
}

bool sameScale(const GradingScale& a, const GradingScale& b) {
    return a.minGrade == b.minGrade && a.maxGrade == b.maxGrade && a.passingGrade == b.passingGrade &&
           a.direction == b.direction && a.nearMarginPercent == b.nearMarginPercent;
}

bool isBetterOrEqual(double a, double b, const GradingScale& scale) {
    if (scale.direction == Direction::LowerIsBetter) {
        return a <= b;
    } else {
        return a >= b;
    }
}

bool isPassing(double grade, const GradingScale& scale) {
    return isBetterOrEqual(grade, scale.passingGrade, scale);
}

double goodness(double grade, const GradingScale& scale) {
    double range = scale.maxGrade - scale.minGrade;
    if (range <= 0) {
        return 0.0;
    }
    if (scale.direction == Direction::LowerIsBetter) {
        return (scale.maxGrade - grade) / range;
    } else {
        return (grade - scale.minGrade) / range;
    }
}

double bestGrade(const GradingScale& scale) {
    return scale.direction == Direction::LowerIsBetter ? scale.minGrade : scale.maxGrade;
}

double worstGrade(const GradingScale& scale) {
    return scale.direction == Direction::LowerIsBetter ? scale.maxGrade : scale.minGrade;
}
