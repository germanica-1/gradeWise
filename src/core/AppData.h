#pragma once
#include <string>
#include <vector>
#include "GradingScale.h"
#include "Subject.h"

// Everything the app remembers between runs. MainWindow owns the single copy.
struct AppData {
    std::vector<Subject> subjects;
    GradingScale scale;
    int presetIndex = PRESET_STI;
    double target = 1.75;

    std::string model = "gemini-3.5-flash-lite";
    std::string apiKey; // optional; the GEMINI_API_KEY environment variable wins

    std::string lastFeedback;     // raw JSON text returned by Gemini
    std::string lastFeedbackDate; // e.g. "2026-11-25 14:03"
    bool feedbackStale = false;   // true when grades changed after the feedback was made
};
