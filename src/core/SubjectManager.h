#pragma once
#include <string>
#include <vector>
#include "GradingScale.h"
#include "Subject.h"

// Each returns true on success; on failure `error` explains why and the list is unchanged.
bool addSubject(std::vector<Subject>& subjects, const Subject& subject, const GradingScale& scale, std::string& error);
bool editSubject(std::vector<Subject>& subjects, int index, const Subject& subject, const GradingScale& scale, std::string& error);
bool deleteSubject(std::vector<Subject>& subjects, int index);

int findSubject(const std::vector<Subject>& subjects, const std::string& name); // -1 if not found
std::string nextDefaultName(const std::vector<Subject>& subjects);             // "Subject 1", "Subject 2", ...
