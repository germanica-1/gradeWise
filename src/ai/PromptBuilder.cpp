#include "PromptBuilder.h"
#include "../core/Calculator.h"
#include "../core/Validation.h"

static std::string describeScale(const GradingScale& scale) {
    std::string text = "Grading scale: " + scale.name + ", from " + formatGrade(scale.minGrade) +
                       " to " + formatGrade(scale.maxGrade) + ", passing grade " + formatGrade(scale.passingGrade) + ". ";
    if (scale.direction == Direction::LowerIsBetter) {
        text += "IMPORTANT: on this scale LOWER numbers are BETTER (" + formatGrade(scale.minGrade) + " is the best grade).";
    } else {
        text += "On this scale HIGHER numbers are BETTER (" + formatGrade(scale.maxGrade) + " is the best grade).";
    }
    return text;
}

std::string buildPrompt(const AppData& data) {
    const std::vector<Subject>& subjects = data.subjects;
    double gwa = calculateGWA(subjects);
    int units = getTotalUnits(subjects);
    GoalStatus status = evaluateGoal(gwa, units, data.target, data.scale);
    std::vector<int> ranking = rankByImpact(subjects, data.scale);

    std::string p;
    p += "You are a supportive academic advisor for a student.\n";
    p += "Use ONLY the data below. Do not invent subjects, grades, units, or any other numbers. "
         "Do not recalculate the GWA; it is already correct.\n\n";

    p += describeScale(data.scale) + "\n";
    p += "Target GWA: " + formatGrade(data.target) + "\n";
    p += "Current GWA: " + formatGrade(gwa) + "\n";
    p += "Total units: " + std::to_string(units) + "\n";
    p += "Goal status: " + goalStatusText(status) + "\n\n";

    p += "Subjects, ordered from the one lowering the GWA the most to the least "
         "(impact = distance from the best grade x units):\n";
    for (std::size_t r = 0; r < ranking.size(); r++) {
        const Subject& s = subjects[ranking[r]];
        p += std::to_string(r + 1) + ". " + s.name + " | grade " + formatGrade(s.grade) + " | " +
             std::to_string(s.units) + " units | weighted " + formatGrade(weightedGrade(s)) + " | " +
             performanceCategory(s.grade, data.scale) + "\n";
    }
    p += "\n";

    if (status == GoalStatus::Achieved) {
        p += "The student REACHED the target. Give specific, encouraging feedback: explain the performance, "
             "name the subjects contributing most positively, and suggest how to maintain or improve.\n";
    } else {
        p += "The student has NOT reached the target. Identify the subjects pulling the GWA down (use the order above), "
             "explain why each should be prioritized (mention its grade and units), give realistic weekly study "
             "recommendations, and keep the tone constructive and motivating.\n";
    }

    p += "\nReply with JSON only, in exactly this shape:\n"
         "{\"summary\": \"2-4 sentences\", "
         "\"priority_subjects\": [{\"name\": \"exact subject name from the list\", \"reason\": \"one sentence\"}], "
         "\"strengths\": [\"one sentence each\"], "
         "\"study_tips\": [\"one sentence each\"]}\n"
         "Keep the whole answer under 250 words. priority_subjects may be empty if every subject is already strong.\n";
    return p;
}
