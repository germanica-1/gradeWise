// Plain assert-based checks for the core logic. Run: ctest, or build and run this file directly.
#undef NDEBUG
#include <cassert>
#include <cmath>
#include <iostream>
#include "../src/core/Calculator.h"
#include "../src/core/SubjectManager.h"
#include "../src/core/Validation.h"
#include "../src/ai/PromptBuilder.h"

static bool near(double a, double b) { return std::fabs(a - b) < 1e-6; }

static GradingScale percentScale() { // a custom higher-is-better scale
    GradingScale s = presetScale(PRESET_CUSTOM);
    s.minGrade = 0;
    s.maxGrade = 100;
    s.passingGrade = 75;
    s.direction = Direction::HigherIsBetter;
    return s;
}

static void testUniversityPresets() {
    GradingScale feu = presetScale(PRESET_FEU); // 0-4.0, higher is better, 1.0 passes
    assert(bestGrade(feu) == 4.0 && worstGrade(feu) == 0.0);
    assert(isPassing(1.0, feu) && !isPassing(0.0, feu));
    assert(evaluateGoal(3.40, 10, 3.50, feu) == GoalStatus::NotYet); // gap 0.10 <= 0.20
    assert(evaluateGoal(3.20, 10, 3.50, feu) == GoalStatus::AtRisk); // gap 0.30 > 0.20
    assert(evaluateGoal(3.60, 10, 3.50, feu) == GoalStatus::Achieved);

    GradingScale ust = presetScale(PRESET_UST); // 1.00-5.00, lower is better, 3.00 passes
    assert(bestGrade(ust) == 1.0 && worstGrade(ust) == 5.0);
    assert(sameScale(ust, presetScale(PRESET_STI)) && !sameScale(ust, feu));
}

static std::vector<Subject> exampleA() { // PH scale, from the plan
    return {{"Math", 1.50, 3}, {"Programming", 1.25, 3}, {"PE", 2.00, 2}, {"English", 2.50, 3}};
}

static std::vector<Subject> exampleB() { // percentage scale
    return {{"Math", 92, 3}, {"Science", 85, 3}, {"PE", 80, 2}};
}

static void testTotalsAndGwa() {
    std::vector<Subject> a = exampleA();
    assert(getTotalUnits(a) == 11);
    assert(near(getTotalWeighted(a), 19.75));
    assert(near(calculateGWA(a), 19.75 / 11));
    assert(formatGrade(calculateGWA(a)) == "1.80");

    std::vector<Subject> b = exampleB();
    assert(near(calculateGWA(b), 691.0 / 8));

    std::vector<Subject> empty;
    assert(getTotalUnits(empty) == 0);
    assert(calculateGWA(empty) == 0.0);
}

static void testRecursiveMatchesIterative() {
    std::vector<Subject> a = exampleA();
    for (std::size_t n = 0; n <= a.size(); n++) {
        std::vector<Subject> part(a.begin(), a.begin() + n);
        assert(near(sumWeightedRecursive(part, 0), sumWeightedIterative(part)));
        assert(sumUnitsRecursive(part, 0) == sumUnitsIterative(part));
    }
}

static void testGoalLowerIsBetter() {
    GradingScale ph = presetScale(PRESET_STI);
    int units = 11;
    assert(evaluateGoal(1.80, units, 1.75, ph) == GoalStatus::NotYet);   // gap 0.05 <= 0.20
    assert(evaluateGoal(1.80, units, 1.90, ph) == GoalStatus::Achieved);
    assert(evaluateGoal(1.75, units, 1.75, ph) == GoalStatus::Achieved); // exactly on target
    assert(evaluateGoal(2.50, units, 1.75, ph) == GoalStatus::AtRisk);   // gap 0.75 > 0.20
    assert(evaluateGoal(3.10, units, 3.00, ph) == GoalStatus::AtRisk);   // close, but failing
    assert(evaluateGoal(0.0, 0, 1.75, ph) == GoalStatus::NoData);
}

static void testGoalHigherIsBetter() {
    GradingScale pct = percentScale();
    int units = 8;
    assert(evaluateGoal(86.375, units, 88, pct) == GoalStatus::NotYet);  // gap 1.625 <= 5
    assert(evaluateGoal(90, units, 88, pct) == GoalStatus::Achieved);
    assert(evaluateGoal(80, units, 88, pct) == GoalStatus::AtRisk);      // gap 8 > 5
}

static void testCategoriesAndPassing() {
    GradingScale ph = presetScale(PRESET_STI);
    assert(isPassing(3.00, ph) && !isPassing(5.00, ph));
    assert(performanceCategory(1.00, ph) == "Excellent");
    assert(performanceCategory(5.00, ph) == "Failed");
    GradingScale pct = percentScale();
    assert(isPassing(75, pct) && !isPassing(74, pct));
    assert(performanceCategory(95, pct) == "Excellent");
}

static void testRankByImpact() {
    std::vector<int> order = rankByImpact(exampleA(), presetScale(PRESET_STI));
    assert(order.size() == 4);
    assert(order[0] == 3); // English (2.50 x 3 units) hurts most
    assert(order[3] == 1); // Programming (1.25) hurts least
}

static void testSubjectManager() {
    GradingScale ph = presetScale(PRESET_STI);
    std::vector<Subject> list;
    std::string error;

    assert(addSubject(list, {"  Math  ", 1.5, 3}, ph, error));
    assert(list[0].name == "Math");
    assert(!addSubject(list, {"math", 2.0, 3}, ph, error));    // duplicate, case-insensitive
    assert(!addSubject(list, {"", 2.0, 3}, ph, error));        // empty name
    assert(!addSubject(list, {"Art", 0.5, 3}, ph, error));     // below scale
    assert(!addSubject(list, {"Art", 2.0, 0}, ph, error));     // zero units
    assert(list.size() == 1);

    assert(editSubject(list, 0, {"Math", 1.25, 3}, ph, error)); // same name allowed on edit
    assert(near(list[0].grade, 1.25));
    assert(!editSubject(list, 5, {"X", 1.0, 3}, ph, error));

    assert(nextDefaultName(list) == "Subject 2");
    assert(deleteSubject(list, 0));
    assert(!deleteSubject(list, 0));
    assert(list.empty());
}

static void testScaleValidation() {
    std::string error;
    GradingScale s = presetScale(PRESET_STI);
    assert(validateScale(s, error));
    s.minGrade = 5;
    s.maxGrade = 1;
    assert(!validateScale(s, error));
    assert(!validateTarget(6.0, presetScale(PRESET_STI), error));
}

static void testPrompt() {
    AppData data;
    data.subjects = exampleA();
    data.target = 1.75;
    std::string p = buildPrompt(data);
    for (const Subject& s : data.subjects) {
        assert(p.find(s.name) != std::string::npos); // every subject is sent
    }
    assert(p.find("Current GWA: 1.80") != std::string::npos);
    assert(p.find("LOWER numbers are BETTER") != std::string::npos);
    assert(p.find("GOAL NOT YET ACHIEVED") != std::string::npos);
    assert(p.find("1. English") != std::string::npos); // biggest impact listed first
}

int main() {
    testPrompt();
    testUniversityPresets();
    testTotalsAndGwa();
    testRecursiveMatchesIterative();
    testGoalLowerIsBetter();
    testGoalHigherIsBetter();
    testCategoriesAndPassing();
    testRankByImpact();
    testSubjectManager();
    testScaleValidation();
    std::cout << "All core tests passed.\n";
    return 0;
}
