#include "SubjectManager.h"
#include <cctype>
#include "Validation.h"

static std::string toLower(const std::string& text) {
    std::string result = text;
    for (std::size_t i = 0; i < result.size(); i++) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

// Shared checks for add and edit. `skipIndex` lets an edited subject keep its own name.
static bool checkSubject(const std::vector<Subject>& subjects, const Subject& subject, int skipIndex,
                         const GradingScale& scale, std::string& error) {
    if (!validateName(subject.name, error)) return false;
    if (!validateGrade(subject.grade, scale, error)) return false;
    if (!validateUnits(subject.units, error)) return false;

    int existing = findSubject(subjects, subject.name);
    if (existing != -1 && existing != skipIndex) {
        error = "A subject with that name already exists.";
        return false;
    }
    return true;
}

bool addSubject(std::vector<Subject>& subjects, const Subject& subject, const GradingScale& scale, std::string& error) {
    if (!checkSubject(subjects, subject, -1, scale, error)) {
        return false;
    }
    Subject cleaned = subject;
    cleaned.name = trim(subject.name);
    subjects.push_back(cleaned);
    return true;
}

bool editSubject(std::vector<Subject>& subjects, int index, const Subject& subject, const GradingScale& scale, std::string& error) {
    if (index < 0 || index >= static_cast<int>(subjects.size())) {
        error = "No subject selected.";
        return false;
    }
    if (!checkSubject(subjects, subject, index, scale, error)) {
        return false;
    }
    subjects[index] = subject;
    subjects[index].name = trim(subject.name);
    return true;
}

bool deleteSubject(std::vector<Subject>& subjects, int index) {
    if (index < 0 || index >= static_cast<int>(subjects.size())) {
        return false;
    }
    subjects.erase(subjects.begin() + index);
    return true;
}

int findSubject(const std::vector<Subject>& subjects, const std::string& name) {
    std::string wanted = toLower(trim(name));
    int i = 0;
    while (i < static_cast<int>(subjects.size())) {
        if (toLower(subjects[i].name) == wanted) {
            return i;
        }
        i++;
    }
    return -1;
}

std::string nextDefaultName(const std::vector<Subject>& subjects) {
    int number = static_cast<int>(subjects.size());
    std::string candidate;
    do {
        number++;
        candidate = "Subject " + std::to_string(number);
    } while (findSubject(subjects, candidate) != -1);
    return candidate;
}
