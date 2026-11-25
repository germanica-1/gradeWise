#include "Calculator.h"

// sum(i) = weighted(subjects[i]) + sum(i + 1); base case: past the last subject -> 0.
double sumWeightedRecursive(const std::vector<Subject>& subjects, std::size_t index) {
    if (index >= subjects.size()) {
        return 0.0;
    }
    return weightedGrade(subjects[index]) + sumWeightedRecursive(subjects, index + 1);
}

int sumUnitsRecursive(const std::vector<Subject>& subjects, std::size_t index) {
    if (index >= subjects.size()) {
        return 0;
    }
    return subjects[index].units + sumUnitsRecursive(subjects, index + 1);
}

double sumWeightedIterative(const std::vector<Subject>& subjects) {
    double total = 0.0;
    for (std::size_t i = 0; i < subjects.size(); i++) {
        total += weightedGrade(subjects[i]);
    }
    return total;
}

int sumUnitsIterative(const std::vector<Subject>& subjects) {
    int total = 0;
    for (std::size_t i = 0; i < subjects.size(); i++) {
        total += subjects[i].units;
    }
    return total;
}

int getTotalUnits(const std::vector<Subject>& subjects) {
    return sumUnitsRecursive(subjects, 0);
}

double getTotalWeighted(const std::vector<Subject>& subjects) {
    return sumWeightedRecursive(subjects, 0);
}

double calculateGWA(const std::vector<Subject>& subjects) {
    int units = getTotalUnits(subjects);
    if (units == 0) {
        return 0.0; // avoid division by zero; the GUI shows "—"
    }
    return getTotalWeighted(subjects) / units;
}

GoalStatus evaluateGoal(double gwa, int totalUnits, double target, const GradingScale& scale) {
    if (totalUnits == 0) {
        return GoalStatus::NoData;
    }

    // gap > 0 means the student is still short of the target, whatever the scale direction.
    double gap;
    if (scale.direction == Direction::LowerIsBetter) {
        gap = gwa - target;
    } else {
        gap = target - gwa;
    }
    double margin = scale.nearMarginPercent / 100.0 * (scale.maxGrade - scale.minGrade);

    if (gap <= 1e-9) {
        return GoalStatus::Achieved;
    } else {
        if (!isPassing(gwa, scale)) {
            return GoalStatus::AtRisk; // failing average is always at risk, even if near the target
        } else if (gap <= margin) {
            return GoalStatus::NotYet;
        } else {
            return GoalStatus::AtRisk;
        }
    }
}

std::string goalStatusText(GoalStatus status) {
    switch (status) {
    case GoalStatus::Achieved: return "GOAL ACHIEVED";
    case GoalStatus::NotYet:   return "GOAL NOT YET ACHIEVED";
    case GoalStatus::AtRisk:   return "GOAL AT RISK";
    case GoalStatus::NoData:   return "NO DATA YET";
    }
    return "";
}

std::string performanceCategory(double grade, const GradingScale& scale) {
    if (!isPassing(grade, scale)) {
        return "Failed";
    }
    double g = goodness(grade, scale);
    if (g >= 0.90) {
        return "Excellent";
    } else if (g >= 0.75) {
        return "Very Good";
    } else if (g >= 0.60) {
        return "Good";
    } else {
        return "Fair";
    }
}

// Impact = how far a grade is from the best grade, times its units.
// A 2.50 in a 3-unit subject hurts more than a 2.00 in a 2-unit subject.
static double impact(const Subject& s, const GradingScale& scale) {
    return (1.0 - goodness(s.grade, scale)) * s.units;
}

std::vector<int> rankByImpact(const std::vector<Subject>& subjects, const GradingScale& scale) {
    std::vector<int> order;
    for (std::size_t i = 0; i < subjects.size(); i++) {
        order.push_back(static_cast<int>(i));
    }
    // Insertion sort, biggest impact first. Small lists, so simple is fine.
    for (std::size_t i = 1; i < order.size(); i++) {
        int current = order[i];
        std::size_t j = i;
        while (j > 0 && impact(subjects[order[j - 1]], scale) < impact(subjects[current], scale)) {
            order[j] = order[j - 1];
            j--;
        }
        order[j] = current;
    }
    return order;
}
