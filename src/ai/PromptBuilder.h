#pragma once
#include <string>
#include "../core/AppData.h"

// Builds the text sent to Gemini. All numbers are computed here, never by the AI.
std::string buildPrompt(const AppData& data);
