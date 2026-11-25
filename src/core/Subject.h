#pragma once
#include <string>

// One subject record. The weighted grade is not stored; it is always grade * units.
struct Subject {
    std::string name;
    double grade = 0.0;
    int units = 0;
};

inline double weightedGrade(const Subject& s) { return s.grade * s.units; }
