#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "GradingScale.h"
#include "Subject.h"

// Recursive versions (used by the app) and iterative versions (kept for comparison).
double sumWeightedRecursive(const std::vector<Subject>& subjects, std::size_t index);
int sumUnitsRecursive(const std::vector<Subject>& subjects, std::size_t index);
double sumWeightedIterative(const std::vector<Subject>& subjects);
int sumUnitsIterative(const std::vector<Subject>& subjects);

int getTotalUnits(const std::vector<Subject>& subjects);
double getTotalWeighted(const std::vector<Subject>& subjects);
double calculateGWA(const std::vector<Subject>& subjects); // returns 0 when there are no units

enum class GoalStatus { NoData, Achieved, NotYet, AtRisk };

GoalStatus evaluateGoal(double gwa, int totalUnits, double target, const GradingScale& scale);
std::string goalStatusText(GoalStatus status);
std::string performanceCategory(double grade, const GradingScale& scale);

// Subject indexes ordered from the one hurting the GWA most to the one hurting it least.
std::vector<int> rankByImpact(const std::vector<Subject>& subjects, const GradingScale& scale);
